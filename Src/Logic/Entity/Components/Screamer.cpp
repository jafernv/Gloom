/**
@file Screamer.cpp

Contiene la implementaci�n de la clase que
implementa las habilidades del personaje
"Screamer".
 
@see Logic::CScreamer
@see Logic::IComponent

@author Francisco Aisa Garc�a.
@author Antonio Jesus Narvaez Corrales
@date Marzo, 2013
*/


#include "Logic/Maps/WorldState.h"

// Componentes
#include "Screamer.h"
#include "AvatarController.h"
#include "ScreamerShieldDamageNotifier.h"
#include "PhysicDynamicEntity.h"
#include "Graphics.h"
#include "Life.h"

// Logica
#include "Logic/Maps/EntityFactory.h"
#include "Logic/Maps/Map.h"
#include "Logic/Entity/Entity.h"
#include "Logic/Server.h"
#include "Map/MapEntity.h"

// Mensajes
#include "Logic/Messages/MessageDamaged.h"
#include "Logic/Messages/MessageSetOwner.h"
#include "Logic/Messages/MessageChangeMaterial.h"
#include "Logic/Messages/MessageCreateParticle.h"
#include "Logic/Messages/MessageAddForcePlayer.h"
#include "Logic/Messages/MessageSetAnimation.h"
#include "Logic/Messages/MessageHud.h"
#include "Logic/Messages/MessageBlockShoot.h"
#include "Logic/Messages/MessageAudio.h"

// F�sica
#include "Physics/Server.h"
#include "Physics/GeometryFactory.h"
#include "Physics/SweepHit.h"
#include "Physics/RaycastHit.h"

// Gr�ficos
#include "Graphics/Server.h"
#include "Graphics/Particle.h"
#include "Graphics/Scene.h"
#include "Graphics/Camera.h"


using namespace std;

namespace Logic {

	IMP_FACTORY(CScreamer);

	//__________________________________________________________________

	CScreamer::CScreamer() : CPlayerClass("screamer"),
							 _secondarySkillIsActive(false),
							 _screamerShieldDamageTimer(0),
							 _screamerShieldRecoveryTimer(0),
							 _screamerShield(NULL) {


	}

	//__________________________________________________________________

	CScreamer::~CScreamer() {
		// Nada que hacer
	}
	
	//__________________________________________________________________

	bool CScreamer::spawn(CEntity* entity, CMap* map, const Map::CEntity* entityInfo) {
		if( !CPlayerClass::spawn(entity,map,entityInfo) ) return false;

		if(entityInfo->hasAttribute("materialName"))
			_materialName = entityInfo->getStringAttribute("materialName");

		// Nos aseguramos de que todos los atributos necesarios para jugar esten
		// correctamente asignados
		assert( entityInfo->hasAttribute("screamerShieldThreshold") );
		assert( entityInfo->hasAttribute("screamerShieldDamageTimeStep") );
		assert( entityInfo->hasAttribute("screamerShieldDamageOverTime") );
		assert( entityInfo->hasAttribute("screamerShieldRecoveryTimeStep") );
		assert( entityInfo->hasAttribute("screamerShieldRecoveryOverTime") );
		assert( entityInfo->hasAttribute("screamerExplotionDamage") );
		assert( entityInfo->hasAttribute("screamerExplotionRadius") );
		assert( entityInfo->hasAttribute("screamerScreamForce") );

		// Asignamos los atributos correspondientes.
		_currentScreamerShield = _screamerShieldThreshold = entityInfo->getFloatAttribute("screamerShieldThreshold");
		_screamerShieldDamageTimeStep = entityInfo->getIntAttribute("screamerShieldDamageTimeStep");
		_screamerShieldDamageOverTime = entityInfo->getFloatAttribute("screamerShieldDamageOverTime");
		_screamerShieldRecoveryTimeStep = entityInfo->getIntAttribute("screamerShieldRecoveryTimeStep");
		_screamerShieldRecoveryOverTime = entityInfo->getFloatAttribute("screamerShieldRecoveryOverTime");
		_screamerExplotionDamage = entityInfo->getFloatAttribute("screamerExplotionDamage");
		_screamerExplotionRadius = entityInfo->getFloatAttribute("screamerExplotionRadius");

		_screamerScreamForce = entityInfo->getFloatAttribute("screamerScreamForce");
		_screamerReboundForce= entityInfo->getFloatAttribute("screamerReboundForce");
		_screamerScreamMaxDistance= entityInfo->getFloatAttribute("screamerScreamMaxDistance");
		_screamerReboundDistance = entityInfo->getFloatAttribute("screamerReboundDistance");
		_screamerMaxRebounds = entityInfo->getIntAttribute("screamerMaxRebounds");

		_rebound = 0;

		return true;
	} // spawn

	//__________________________________________________________________

	void CScreamer::checkSecondarySkill(unsigned int msecs) {
		// Si la habilidad secundaria est� siendo usada
		if(_secondarySkillIsActive) {
			// Comprobamos si tenemos que explotar, ya que podr�an habernos
			// disparado al escudo
			if(_currentScreamerShield < 0) {
				// El screamer explota
				blowUp();
			}
			else {
				// Reducir la energia si es posible
				if(_currentScreamerShield > 1) {
					// Decrementar iterativamente hasta llegar a 0
					// o no poder decrementar mas
					_screamerShieldDamageTimer += msecs;
					unsigned int steps = _screamerShieldDamageTimer / _screamerShieldDamageTimeStep;
					_screamerShieldDamageTimer = _screamerShieldDamageTimer % _screamerShieldDamageTimeStep;
					
					for(int i = 0; i < steps; ++i) {
						_currentScreamerShield -= _screamerShieldDamageTimeStep;
						if(_currentScreamerShield <= 1) {
							_currentScreamerShield = 1;
							break;
						}
					}
				}
			}
		}
		else if(_currentScreamerShield < _screamerShieldThreshold) {
			// Incremento el timer de recuperaci�n hasta llegar al m�ximo
			_screamerShieldRecoveryTimer += msecs;
			unsigned int steps = _screamerShieldRecoveryTimer / _screamerShieldRecoveryTimeStep;
			_screamerShieldRecoveryTimer = _screamerShieldRecoveryTimer % _screamerShieldRecoveryTimeStep;

			for(int i = 0; i < steps; ++i) {
				_currentScreamerShield += _screamerShieldDamageTimeStep;
				if(_currentScreamerShield >= _screamerShieldThreshold) {
					_currentScreamerShield = _screamerShieldThreshold;
					break;
				}
			}	
			
		}
	}

	//________________________________________________________________________

	void CScreamer::blowUp() {
		// Elimino la entidad escudo
		if(_screamerShield != NULL) {
			// Destruimos la entidad e indicamos a los clientes que tambien la destruyan
			CEntityFactory::getSingletonPtr()->deferredDeleteEntity(_screamerShield, true);
			_screamerShield = NULL;
		}

		// Creo la explosi�n. La propia explosi�n me mata ya que estoy en el centro
		// y se emite un mensaje de instakill.
		createExplotion();
	}

	//________________________________________________________________________

	void CScreamer::onActivate() {
		CPlayerClass::onActivate();

		// @deprectaed De momento se hace esto en el activate para darle
		// una textura distinta a cada clase.
		/*std::shared_ptr<CMessageChangeMaterial> materialMsg = std::make_shared<CMessageChangeMaterial>();
		materialMsg->setMaterialName(_materialName);
		_entity->emitMessage(materialMsg);*/

		_secondarySkillIsActive = false;
		_currentScreamerShield = _screamerShieldThreshold;
		_screamerShieldDamageTimer = _screamerShieldRecoveryTimer = 0;
	}

	//________________________________________________________________________

	void CScreamer::onDeactivate() {
		if(_screamerShield != NULL) {
			// Destruimos la entidad e indicamos a los clientes que tambien la destruyan
			CEntityFactory::getSingletonPtr()->deferredDeleteEntity(_screamerShield, true);
			_screamerShield = NULL;
		}
	}

	//__________________________________________________________________

	void CScreamer::primarySkill() {

		// Sonido de grito
		std::shared_ptr<CMessageAudio> audioMsg = std::make_shared<CMessageAudio>();
		
		audioMsg->setAudioName("character/scream.ogg");
		audioMsg->isLoopable(false);
		audioMsg->is3dSound(true);
		audioMsg->streamSound(false);
		audioMsg->stopSound(false);
			
		_entity->emitMessage(audioMsg);

		// Si no he hecho ningun rebote, he de coger la del player, si estoy en algun rebote, el rebote se encarga de decirme en q posicion.
		if(_rebound == 0){
			_directionShoot = _entity->getOrientation()*Vector3::NEGATIVE_UNIT_Z;
			_distanceShoot = _screamerScreamMaxDistance;
			_positionShoot = _entity->getPosition() + Vector3(0,_heightShoot,0);
		}
	
		bool goToReflect = (_rebound < _screamerMaxRebounds);
		// Si he llegao al maximo de rebotes permitidos, actualizo 
		if(_rebound == _screamerMaxRebounds) _rebound = 0;
		float distance = _distanceShoot;

		Physics::CRaycastHit hitWorld;
		
		if(goToReflect){
			Ray ray( _positionShoot , _directionShoot);
			if(Physics::CServer::getSingletonPtr()->raycastSingle(ray,_screamerScreamMaxDistance,hitWorld,Physics::CollisionGroup::eWORLD))
				// A veces da como que si impacta pero en realidad no lo hace. Para esos casos, la distancia es -1
				if(hitWorld.distance > 0)
					distance =std::min(hitWorld.distance, _distanceShoot);		
		}
		

		// 3.5 mirao a ojo a traves del visual debbuger 
		Physics::SphereGeometry sphere  = Physics::CGeometryFactory::getSingletonPtr()->createSphere(3.5);
		std::vector<Physics::CSweepHit> hitSpots;
		Physics::CServer::getSingletonPtr()->sweepMultiple(sphere, _positionShoot,_directionShoot,distance,hitSpots, false, Physics::CollisionGroup::ePLAYER );
		
		sweepHitConsequences(hitSpots);
		// Ocurre igual q arriba, si la distancia es menor q 0 esq no ha habido colision con el mundo
		if(goToReflect && hitWorld.distance > 0)
			raycastHitConsequences(hitWorld);
		
	} // primarySkill
	//__________________________________________________________________

	void CScreamer::raycastHitConsequences(const Physics::CRaycastHit &hitWorld){
		if(hitWorld.distance < _screamerReboundDistance){
			auto m = std::make_shared<CMessageAddForcePlayer>();
			m->setForce(-_directionShoot * (_screamerReboundForce*( 1.0f - hitWorld.distance/_screamerScreamMaxDistance)));
			_entity->emitMessage(m);
			_rebound = 0;
		}else{
			if(_rebound < _screamerMaxRebounds){
				++_rebound;
				_positionShoot = hitWorld.impact;
				// los dos reflejos que quedan bien son
				// _directionShoot.reflect(hitWorld.normal)
				// (_directionShoot + hitWorld.normal).normalisedCopy()
				_directionShoot = _directionShoot.reflect(hitWorld.normal);
				_distanceShoot -= hitWorld.distance;
				primarySkill();
			}else{
				_rebound = 0;
			}
		}
	} // raycastHitConsequences
	//__________________________________________________________________
	
	void CScreamer::sweepHitConsequences(const std::vector<Physics::CSweepHit> &hits){
		for(auto it = hits.begin(); it < hits.end(); ++it){
			if((*it).entity->getName() != _entity->getName()){
				Vector3 direct = -(_directionShoot.reflect(-(*it).normal));
				auto m = std::make_shared<CMessageAddForcePlayer>();
				Vector3 temp = (_directionShoot + (Vector3(0.0f,1.0f,0.0f))).normalisedCopy();
				m->setForce(temp * (_screamerScreamForce*(1.0f- (*it).distance/_screamerScreamMaxDistance)));
				(*it).entity->emitMessage(m);
			}
		}				
	} // sweepHitConsequences
	//__________________________________________________________________

	void CScreamer::secondarySkill() {
		_secondarySkillIsActive = true;

		Map::CEntity* screamerShieldInfo = CEntityFactory::getSingletonPtr()->getInfo("ScreamerShield");
		float screamerShieldRadius = screamerShieldInfo->getFloatAttribute("physic_radius");

		// Creamos una entidad ScreamerShield
		// Obtenemos la informacion asociada al arquetipo del escudo del screamer
		Vector3 shootPosition = _entity->getPosition() + ( (_entity->getOrientation() * Vector3::NEGATIVE_UNIT_Z ) * (_capsuleRadius + screamerShieldRadius) );
		shootPosition.y += _heightShoot;

		// Creamos la entidad y la activamos
		_screamerShield = CEntityFactory::getSingletonPtr()->createEntity( 
			screamerShieldInfo,
			Logic::CServer::getSingletonPtr()->getMap(),
			shootPosition,
			_entity->getOrientation()
		);
		
		// Activamos la entidad creada
		_screamerShield->activate();
		_screamerShield->start();

		// Enviamos el mensaje SET_OWNER para que el escudo se mueva
		// acorde a los movimientos del player
		shared_ptr<CMessageSetOwner> setOwnerMsg = make_shared<CMessageSetOwner>();
		setOwnerMsg->setOwner(_entity);
		_screamerShield->emitMessage(setOwnerMsg);

		// Sonido del escudo
		std::shared_ptr<CMessageAudio> audioMsg = std::make_shared<CMessageAudio>();
		
		audioMsg->setAudioName("character/screamerShield.wav");
		audioMsg->isLoopable(true);
		audioMsg->is3dSound(true);
			
		_screamerShield->emitMessage(audioMsg);

		// Enviamos un mensaje para indicar que se bloquee el disparo
		shared_ptr<CMessageBlockShoot> canShootMsg = make_shared<CMessageBlockShoot>();
		canShootMsg->canShoot(false);
		_entity->emitMessage(canShootMsg);

		// Mandamos al jugador que tiene la habilidad disponible
		std::shared_ptr<CMessageHud> hudMsg = std::make_shared<CMessageHud>();
		hudMsg->setType(CMessageHud::HudType::SECONDARY_ACTIVE);
		_entity->emitMessage(hudMsg);
	} // secondarySkill
	//__________________________________________________________________

	void CScreamer::stopSecondarySkill() {
		_secondarySkillIsActive = false;

		// Enviamos un mensaje para indicar que el disparo ya no esta bloqueado
		shared_ptr<CMessageBlockShoot> canShootMsg = make_shared<CMessageBlockShoot>();
		canShootMsg->canShoot(true);
		_entity->emitMessage(canShootMsg);

		if(_screamerShield != NULL) {
			// Destruimos la entidad e indicamos a los clientes que tambien la destruyan
			CEntityFactory::getSingletonPtr()->deferredDeleteEntity(_screamerShield, true);
			_screamerShield = NULL;
		}

		if(_secondarySkillCooldown > 0) {
			std::shared_ptr<CMessageHud> hudMsg = std::make_shared<CMessageHud>();
			hudMsg->setType(CMessageHud::HudType::SECONDARY_AVAIABLE);
			_entity->emitMessage(hudMsg);
		}
		else {
			std::shared_ptr<CMessageHud> hudMsg = std::make_shared<CMessageHud>();
			hudMsg->setType(CMessageHud::HudType::SECONDARY_SKILL);
			_entity->emitMessage(hudMsg);
		}

	} // stopSecondarySkill
	//__________________________________________________________________

	void CScreamer::absorbDamage(float damage) {
		_currentScreamerShield -= damage;
	} // absorbDamage
	//__________________________________________________________________

	void CScreamer::createExplotion() {
		Map::CEntity* screamerInfo = CEntityFactory::getSingletonPtr()->getInfo("Screamer");
		float height = screamerInfo->getFloatAttribute("heightShoot");
		
		// EntitiesHit sera el buffer que contendra la lista de entidades que ha colisionado
		// con el overlap
		vector<CEntity*> entitiesHit;

		// Hacemos una query de overlap con la geometria de una esfera en la posicion 
		// en la que se encuentra la granada con el radio que se indique de explosion
		Physics::SphereGeometry explotionGeom = Physics::CGeometryFactory::getSingletonPtr()->createSphere(_screamerExplotionRadius);
		Vector3 explotionPos = _entity->getPosition();
		explotionPos.y += height;
		Physics::CServer::getSingletonPtr()->overlapMultiple(explotionGeom, explotionPos, entitiesHit);

		int nbHits = entitiesHit.size();
		// Mandamos el mensaje de da�o a cada una de las entidades que hayamos golpeado
		// Adem�s aplicamos un desplazamiento al jugador 
		for(int i = 0; i < nbHits; ++i) {
			// Si la entidad golpeada es valida y es un player
			if( entitiesHit[i] != NULL && entitiesHit[i]->isPlayer() ) {
				// Emitimos el mensaje de instakill
				// @todo mandar un mensaje de instakill en vez de un mensaje de da�o
				shared_ptr<CMessageDamaged> dmgMsg = make_shared<CMessageDamaged>();
				dmgMsg->setDamage(_screamerExplotionDamage);
				dmgMsg->setEnemy(_entity);
				entitiesHit[i]->emitMessage(dmgMsg);
			}
		}

		// Creamos las particulas de la explosion
		Map::CEntity* entityInfo = CEntityFactory::getSingletonPtr()->getInfo("ScreamerExplotion");
		CEntity* explotion = CEntityFactory::getSingletonPtr()->createEntity(entityInfo, _entity->getMap(), explotionPos, Quaternion::IDENTITY );
		explotion->activate();
		explotion->start();
	} // createExplotion
	//________________________________________________________________________

	void CScreamer::onTick(unsigned int msecs) {
		CPlayerClass::onTick(msecs);

		checkSecondarySkill(msecs);
	} // onTick

} // namespace Logic

