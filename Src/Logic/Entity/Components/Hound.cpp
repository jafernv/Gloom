/**
@file Hound.cpp

Contiene la implementaci�n de la clase que
implementa las habilidades del personaje
"Hound".
 
@see Logic::CHound
@see Logic::IComponent

@author Antonio Jesus Narvaez Corrales.
@author Francisco Aisa Garc�a.
@author Rub�n Mulero Guerrero.
@date Marzo, 2013
*/

#include "Hound.h"
#include "Bite.h"
#include "Map/MapEntity.h"
#include "Logic/Entity/Entity.h"
#include "Logic/Maps/EntityFactory.h"
#include "Logic/Maps/WorldState.h"
#include "Physics/Server.h"
#include "Logic/Server.h"
#include "Logic/Messages/MessageDamageAmplifier.h"
#include "Logic/Messages/MessageReducedCooldown.h"
#include "Logic/Messages/MessageChangeMaterial.h"
#include "Logic/Messages/MessageParticleVisibility.h"
#include "Logic/Messages/MessageTouched.h"
#include "Logic/Messages/MessageDamaged.h"
#include "Logic/Messages/MessageHoundCharge.h"
#include "PhysicController.h"
#include "AvatarController.h"
#include "PhysicDynamicEntity.h"
#include "Net/Manager.h"

#include "Graphics/poolParticle.h"

namespace Logic {

	IMP_FACTORY(CHound);

	//__________________________________________________________________

	CHound::CHound() : CPlayerClass("hound"), _biteEntity(0) {
		// Nada que hacer
	}

	//__________________________________________________________________

	CHound::~CHound() {
		// Nada que hacer
	}
	
	//__________________________________________________________________

	bool CHound::spawn(CEntity* entity, CMap* map, const Map::CEntity* entityInfo) {
		if( !CPlayerClass::spawn(entity,map,entityInfo) ) return false;

		if(entityInfo->hasAttribute("materialName"))
			_materialName = entityInfo->getStringAttribute("materialName");

		// Leer el tiempo que dura el Berserker
		assert( entityInfo->hasAttribute("berserkerDuration") );
		// Pasamos el tiempo a msecs
		_berserkerDuration = entityInfo->getFloatAttribute("berserkerDuration") * 1000;

		assert( entityInfo->hasAttribute("biteSpeed") && "Error: No se ha definido el atributo biteSpeed en el mapa" );
		_biteSpeed = entityInfo->getFloatAttribute("biteSpeed");

		assert( entityInfo->hasAttribute("biteDuration") && "Error: No se ha definido el atributo biteDuration en el mapa" );
		_biteDuration = entityInfo->getFloatAttribute("biteDuration") * 1000;

		return true;
	} // spawn

	//__________________________________________________________________

	void CHound::onStart(){
		_physicController = _entity->getComponent<CPhysicController>("CPhysicController");
	}

	//________________________________________________________________________

	void CHound::onTick(unsigned int msecs) {
		CPlayerClass::onTick(msecs);

		if(_doingPrimarySkill) {
			_biteTimer -= msecs;
			if(_biteTimer < 0) {
				_biteTimer = 0;
				_doingPrimarySkill = false;

				// Mandamos un mensaje al avatar controller para que
				// vuelva a usar el filtro que corresponde
				std::shared_ptr<CMessageHoundCharge> houndChargeMsg = std::make_shared<CMessageHoundCharge>();
				houndChargeMsg->isActive(false);
				_entity->emitMessage(houndChargeMsg);
					
				// Destruimos la entidad del mordisco
				CEntityFactory::getSingletonPtr()->deferredDeleteEntity(_biteEntity, false);
				_biteEntity = NULL;

				// �apa
				if( Net::CManager::getSingletonPtr()->imServer() )
					_physicController->activateSimulation();
			}
		}
		
		if(_doingSecondarySkill) {
			_berserkerTimer -= msecs;
			if(_berserkerTimer < 0) {
				_berserkerTimer = 0;
				_doingSecondarySkill = false;

				auto materialMsg = std::make_shared<CMessageParticleVisibility>();
				materialMsg->setNameParticle("Odor");
				materialMsg->setVisibility(false);
				_entity->emitMessage(materialMsg);

				emitSound("character/houndSmell.wav", false, false, false, true, false);
			}
		}
	}

	//________________________________________________________________________

	void CHound::onActivate() {
		CPlayerClass::onActivate();

		_biteTimer = _berserkerTimer = 0;
		_doingPrimarySkill = _doingSecondarySkill = false;
	}

	//________________________________________________________________________

	void CHound::onDeactivate() {
		if(_biteEntity != NULL)
			CEntityFactory::getSingletonPtr()->deferredDeleteEntity(_biteEntity, false);
	}

	//__________________________________________________________________

	void CHound::primarySkill() {
		_doingPrimarySkill = true;
		_biteTimer = _biteDuration;

		// Creamos la entidad del mordisco y le indicamos que nosotros somos el owner
		// para que actualice su posicion en funcion de la nuestra
		CEntityFactory* factory = CEntityFactory::getSingletonPtr();
		Map::CEntity* houndTrigger = factory->getInfo("Bite");
		_biteEntity = factory->createEntity(houndTrigger, CServer::getSingletonPtr()->getMap(), _entity->getPosition(), Quaternion::IDENTITY, false);
		_biteEntity->activate();
		_biteEntity->start();

		CBite* biteComponent = _biteEntity->getComponent<CBite>("CBite");
		biteComponent->setOwner(_entity);

		// Queremos desactivar solo las colisiones con los players y los hitboxes
		unsigned int filterMask = Physics::CollisionGroup::ePLAYER | Physics::CollisionGroup::eHITBOX;
		// Calculamos el complementario
		filterMask = ~filterMask;
		// Eliminamos el filtro del player y los hitboxes usando el complementario
		// con la mascara que use por defecto el controller
		filterMask = _physicController->getDefaultFilterMask() & filterMask;

		// Mandar un mensaje al avatar controller de empujar al player en la direccion
		// en la que este mirando con el filtro cambiado
		std::shared_ptr<CMessageHoundCharge> houndChargeMsg = std::make_shared<CMessageHoundCharge>();
		houndChargeMsg->setFilterMask(filterMask);
		houndChargeMsg->setForce(_biteSpeed);
		_entity->emitMessage(houndChargeMsg);

		// Emitimos el sonido de carga
		emitSound("character/houndBite.wav", false, true, false, false, false);

		// �apa
		if( Net::CManager::getSingletonPtr()->imServer() )
			_physicController->deactivateSimulation();
	} // primarySkill

	//__________________________________________________________________

	void CHound::secondarySkill() {
		//Arrancamos el cronometro
		_berserkerTimer = _berserkerDuration;

		auto materialMsg = std::make_shared<CMessageParticleVisibility>();
		materialMsg->setNameParticle("Odor");
		materialMsg->setVisibility(true);
		_entity->emitMessage(materialMsg);

		_doingSecondarySkill = true;

		emitSound("character/houndSmell.wav", false, true, true, false, false);
	} // secondarySkill
	

} // namespace Logic

