/**
@file ExplotionController.cpp

@see Logic::IComponent

@author Francisco Aisa Garc�a
@date Febrero, 2013
*/

#include "FireBallController.h"
#include "IronHellGoat.h"
#include "ScreamerShieldDamageNotifier.h"
#include "PhysicDynamicEntity.h"
#include "ParticleSystem.h"

#include "Audio/Server.h"

#include "Logic/Maps/EntityFactory.h"
#include "Logic/GameNetPlayersManager.h"
#include "Physics/Server.h"
#include "Physics/GeometryFactory.h"

#include "Logic/Server.h"
#include "Logic/Maps/Map.h"

#include "Logic/Messages/MessageContactEnter.h"
#include "Logic/Messages/MessageDamaged.h"
#include "Logic/Messages/MessageCreateParticle.h"
#include "Logic/Messages/MessageAddForcePlayer.h"

#include "BaseSubsystems/Euler.h"

#include "Map/MapEntity.h"

using namespace std;

namespace Logic {
	
	IMP_FACTORY(CFireBallController);

	//________________________________________________________________________

	CFireBallController::CFireBallController() { 
		// Nada que hacer
	}

	//________________________________________________________________________

	CFireBallController::~CFireBallController() {
		// Le indicamos al componente due�o que nos vamos a destruir
		// y que por lo tanto ya no vamos a poder recibir mas instrucciones
		// de move
		if(_owner != NULL) {
			_owner->removeFireBall(this);
			_owner = NULL;
		}
	}

	//________________________________________________________________________

	void CFireBallController::onStart() {
		_physicComponent = _entity->getComponent<CPhysicDynamicEntity>("CPhysicDynamicEntity");
		assert(_physicComponent != NULL && "Error: La bola de fuego no tiene collider");
	}

	//________________________________________________________________________

	bool CFireBallController::spawn(CEntity* entity, CMap *map, const Map::CEntity *entityInfo) {
		if( !IComponent::spawn(entity,map,entityInfo) ) return false;

		assert( entityInfo->hasAttribute("speed") );
		assert( entityInfo->hasAttribute("damage") );
		assert( entityInfo->hasAttribute("explotionRadius") );
		assert( entityInfo->hasAttribute("direction") );
		assert( entityInfo->hasAttribute("strength") );
		assert( entityInfo->hasAttribute("shootHeight") );
		assert( entityInfo->hasAttribute("remoteForce") );

		_speed = entityInfo->getFloatAttribute("speed");
		_damage = entityInfo->getFloatAttribute("damage");
		_explotionRadius = entityInfo->getFloatAttribute("explotionRadius");
		Vector3 direction = entityInfo->getVector3Attribute("direction");
		_strength = entityInfo->getFloatAttribute("strength");
		_shootHeight = entityInfo->getFloatAttribute("shootHeight");
		_remoteForce = entityInfo->getFloatAttribute("remoteForce");

		_speedVector = _speed * direction;
		
		_world = Logic::CServer::getSingletonPtr()->getMap()->getEntityByName("World");
		return true;
	}

	//________________________________________________________________________

	bool CFireBallController::accept(const shared_ptr<CMessage>& message) {
		return message->getMessageType() == Message::CONTACT_ENTER;
	}

	//________________________________________________________________________

	void CFireBallController::process(const shared_ptr<CMessage>& message) {
		switch( message->getMessageType() ) {
			case Message::CONTACT_ENTER: {
				shared_ptr<CMessageContactEnter> contactMsg = static_pointer_cast<CMessageContactEnter>(message);
				
				CEntity* entityContacted = contactMsg->getEntity();
				CEntity* fireBallOwner = _owner->getEntity();
				Physics::CContactPoint contactPoint = contactMsg->getContactPoint();
				if( entityContacted != fireBallOwner ) {
					// Destruir en diferido esta entidad
					CEntityFactory::getSingletonPtr()->deferredDeleteEntity(_entity, true);

					if( entityContacted->getType() == "ScreamerShield" ) {
						CEntity* screamerShieldOwner = entityContacted->getComponent<CScreamerShieldDamageNotifier>("CScreamerShieldDamageNotifier")->getOwner();

						// Comprobamos que el screamer shield que hemos alcanzado
						// no es el nuestro
						if( screamerShieldOwner != fireBallOwner) {
							CGameNetPlayersManager* playersMgr = CGameNetPlayersManager::getSingletonPtr();
							TEntityID enemyId = entityContacted->getEntityID();
							
							if( playersMgr->existsByLogicId(enemyId) ) {
								TeamFaction::Enum enemyTeam = playersMgr->getTeamUsingEntityId(enemyId);
								TeamFaction::Enum myTeam = playersMgr->getTeamUsingEntityId(_owner->getEntity()->getEntityID());

								if(enemyTeam == TeamFaction::eNONE || myTeam == TeamFaction::eNONE || enemyTeam != myTeam) {
									std::shared_ptr<CMessageDamaged> damageDone = std::make_shared<CMessageDamaged>();
									damageDone->setDamage(_damage);
									damageDone->setEnemy( _owner->getEntity() );
									entityContacted->emitMessage(damageDone);
								}
							}
							else {
								shared_ptr<CMessageDamaged> dmgMsg = make_shared<CMessageDamaged>();
								dmgMsg->setDamage(_damage);
								dmgMsg->setEnemy( _owner->getEntity() );
								entityContacted->emitMessage(dmgMsg);
							}

							// Creamos las particulas de impacto sobre el escudo
							Map::CEntity* entityInfo = CEntityFactory::getSingletonPtr()->getInfo("ScreamerShieldHit");
							CEntity* shieldHit = CEntityFactory::getSingletonPtr()->createEntity(entityInfo, _entity->getMap(), contactPoint.position, Quaternion::IDENTITY );
							shieldHit->activate();
							shieldHit->start();
						}
					}
					else {
						createExplotion(contactPoint);
					}
				}
			}
		}
	}

	//________________________________________________________________________

	void CFireBallController::alterDirection() {
		CEntity* playerEntity = _owner->getEntity();
		Vector3 shootPosition = playerEntity->getPosition() + Vector3(0.0f, _shootHeight, 0.0f);
		Vector3 playerDirection = -playerEntity->getOrientation().zAxis();
		playerDirection.normalise();
		Vector3 vectorToFireball = _entity->getPosition() - shootPosition;

		// Dado el vector director del player, lo que queremos es el vector
		// perpendicular a dicho vector
		// Primero calculamos la proyeccion del vector player-bola / orientacion
		Vector3 proj = vectorToFireball.dotProduct(playerDirection) * playerDirection;

		// Ahora calculamos el vector perpendicular usando la proyeccion
		Vector3 perp = proj - vectorToFireball;
		perp.normalise();

		// Sumamos el desplazamiento como si de una fuerza se tratase a la direccion
		// actual siempre teniendo en cuenta que no superemos la velocidad maxima.
		_speedVector += perp * _remoteForce;
		// Normalizamos la velocidad para que en ningun caso la bola pueda llegar
		// a alcanzar una velocidad superior a la preestablecida. No divido por
		// la velocidad porque es practicamente 0 y se lia el taco
		if(_speedVector.x > _speed) _speedVector.x = _speed;
		if(_speedVector.y > _speed) _speedVector.y = _speed;
		if(_speedVector.z > _speed) _speedVector.z = _speed;
	}

	//________________________________________________________________________

	void CFireBallController::onFixedTick(unsigned int msecs) {
		// Muevo a la bola de fuego
		_physicComponent->move( _speedVector * msecs );
	}
	
	//________________________________________________________________________

	void CFireBallController::setOwner(CIronHellGoat* owner) {
		this->_owner = owner;
	}

	//________________________________________________________________________

	void CFireBallController::createExplotion(const Physics::CContactPoint& contactPoint) {
		// Generar un overlap y mandar los mensajes de da�o a las entidades
		// con las que se impacte
		vector<CEntity*> entitiesHit;
		Physics::SphereGeometry explotionGeom = Physics::CGeometryFactory::getSingletonPtr()->createSphere(_explotionRadius);
		Vector3 explotionPos = _entity->getPosition();
		Physics::CServer::getSingletonPtr()->overlapMultiple(explotionGeom, explotionPos, entitiesHit, Physics::CollisionGroup::ePLAYER);

		for(int i = 0; i < entitiesHit.size(); ++i) {
			// Mandamos el mensaje de da�o
			estimateDamage(entitiesHit[i], explotionPos);
		}

		// Creamos el sonido de explosion
		Audio::CServer::getSingletonPtr()->playSound3D("weapons/hit/fireball_hit.wav", contactPoint.position, Vector3::ZERO, false, false);

		// Creamos las particulas de la explosion
		Map::CEntity* entityInfo = CEntityFactory::getSingletonPtr()->getInfo("Explotion");
		CEntity* explotion = CEntityFactory::getSingletonPtr()->createEntity(entityInfo, _entity->getMap(), contactPoint.position, Quaternion::IDENTITY );
		explotion->activate();
		explotion->start();

		// Creamos las particulas de humo que rodean a la bola de fuego
		Euler euler(Quaternion::IDENTITY);
		euler.setDirection(contactPoint.normal);
		euler.pitch( Ogre::Radian(Math::HALF_PI) );

		entityInfo = CEntityFactory::getSingletonPtr()->getInfo("SmokeBash");
		CEntity* smokeBash = CEntityFactory::getSingletonPtr()->createEntity(entityInfo, _entity->getMap(), contactPoint.position, euler.toQuaternion() );
		smokeBash->activate();
		smokeBash->start();
	}

	//________________________________________________________________________

	void CFireBallController::estimateDamage(CEntity* entityHit, const Vector3& explotionPos) {
		// Comprobamos el punto de contacto
		Vector3 rayDirection = entityHit->getPosition();

		// Tiramos el rayo al centro de la capsula del enemigo
		// No podemos tirar el rayo directamente porque se lo tira a los pies y a veces no hay contacto
		// Como solucion rapida (aehem.. �apa) voy a sacar la info de la altura del screamer
		// que teoricamente es la misma que la de todos los players
		Map::CEntity * info = Logic::CEntityFactory::getSingletonPtr()->getInfo("Screamer");
		if(info != NULL) rayDirection.y += info->getIntAttribute("physic_height");
		rayDirection = rayDirection - explotionPos;
				
		// Hacemos una query de raycast desde el punto de explosion hasta la entidad analizada
		// de la query de overlap (para saber el punto de colision).
		// No es ni lo mas exacto ni lo mas eficiente, pero soluciona la papeleta.
		Ogre::Ray ray( explotionPos, rayDirection.normalisedCopy() );

		std::vector<Physics::CRaycastHit> hitBuffer;
		Physics::CServer::getSingletonPtr()->raycastMultiple(ray, _explotionRadius, hitBuffer);
		int bufferSize = hitBuffer.size();

		// Calculamos el da�o en base a la distancia del golpe
		float dmg = 0;
		for(int k = 0; k < bufferSize; ++k) {
			if(hitBuffer[k].entity == entityHit) {
				dmg = _damage * ( 1 - (hitBuffer[k].distance/_explotionRadius) );
			}
		}

		//send damage message
		CGameNetPlayersManager* playersMgr = CGameNetPlayersManager::getSingletonPtr();
		TEntityID enemyId = entityHit->getEntityID();
		TEntityID playerId = _owner->getEntity()->getEntityID();
		if( playersMgr->existsByLogicId(enemyId) ) {
			TeamFaction::Enum enemyTeam = playersMgr->getTeamUsingEntityId(enemyId);
			TeamFaction::Enum myTeam = playersMgr->getTeamUsingEntityId(playerId);

			if( !playersMgr->friendlyFireIsActive() && enemyId != playerId) {
				if(enemyTeam == TeamFaction::eNONE || myTeam == TeamFaction::eNONE || enemyTeam != myTeam) {
					std::shared_ptr<CMessageDamaged> damageDone = std::make_shared<CMessageDamaged>();
					damageDone->setDamage(dmg);
					damageDone->setEnemy( _owner->getEntity() );
					entityHit->emitMessage(damageDone);
				}
			}
			else {
				std::shared_ptr<CMessageDamaged> damageDone = std::make_shared<CMessageDamaged>();
				damageDone->setDamage(dmg);
				damageDone->setEnemy( _owner->getEntity() );
				entityHit->emitMessage(damageDone);
			}
		}
		else {
			std::shared_ptr<CMessageDamaged> damageDone = std::make_shared<CMessageDamaged>();
			damageDone->setDamage(dmg);
			damageDone->setEnemy( _owner->getEntity() );
			entityHit->emitMessage(damageDone);
		}

		//send force message
		std::shared_ptr<CMessageAddForcePlayer> force = std::make_shared<CMessageAddForcePlayer>();
		force->setForce( (entityHit->getPosition() - _entity->getPosition()).normalisedCopy() * _strength);
		entityHit->emitMessage(force);
	}

} // namespace Logic

