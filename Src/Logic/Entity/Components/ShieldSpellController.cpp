/**
@file ShieldSpellController.cpp

Contiene la implementaci�n del componente que controla la vida c�pula del arc�ngel que da vida a una entidad.
 
@see Logic::CShieldSpellController
@see Logic::IComponent

@author Jaime Chapinal Cervantes
@date Mayo, 2013
*/

#include "ShieldSpellController.h"

#include "PhysicDynamicEntity.h"

#include "Graphics/Entity.h"
#include "Graphics.h"
#include "Logic/Maps/Map.h"
#include "Map/MapEntity.h"

#include "Archangel.h"
#include "Logic/Messages/MessageTouched.h"
#include "Logic/Messages/MessageDamaged.h"
#include "Logic/Messages/MessageSetOwner.h"

namespace Logic 
{
	IMP_FACTORY(CShieldSpellController);
		
	//________________________________________________________________________

	/*CShieldSpellController::CShieldSpellController() 
	{
					 //_owner(NULL) {

		// Nada que hacer
	}*/

	//________________________________________________________________________

	CShieldSpellController::~CShieldSpellController() {
		// Nada que borrar
	}

	//________________________________________________________________________

	bool CShieldSpellController::spawn(CEntity *entity, CMap *map, const Map::CEntity *entityInfo) 
	{
		if(!IComponent::spawn(entity,map,entityInfo))
			return false;

		

		// El tama�o de esta bola se indica en el archetypes.
		return true;

	} // spawn

	//________________________________________________________________________

	bool CShieldSpellController::accept(const std::shared_ptr<CMessage>& message) {
		Logic::TMessageType msgType = message->getMessageType();

		return	msgType == Message::TOUCHED || 
				msgType == Message::SET_OWNER;;
	} // accept
	
	//________________________________________________________________________

	void CShieldSpellController::process(const std::shared_ptr<CMessage>& message) {
		switch( message->getMessageType() ) {
			case Message::TOUCHED: {				
				auto msg = std::static_pointer_cast<CMessageTouched>(message);
				touched(msg->getEntity());
				break;
			}
			case Message::SET_OWNER: {
				setOwner( std::static_pointer_cast<CMessageSetOwner>(message)->getOwner() );
				break;
			}
		}
	} // process

	//________________________________________________________________________

	void CShieldSpellController::onStart() {

	}
	//________________________________________________________________________
	
	void CShieldSpellController::setProperties(float damage){
		_damage = damage;
	} // setProperty
	//________________________________________________________________________

	void CShieldSpellController::setOwner(Logic::CEntity *owner){

		_physicComponent = _entity->getComponent<CPhysicDynamicEntity>("CPhysicDynamicEntity"); 	

		_cGraph = _entity->getComponent<CGraphics>("CGraphics");

		_owner = owner;

		printf("\nOwner puesto \n");
	} // setOwner
	//________________________________________________________________________

	void CShieldSpellController::onFixedTick(unsigned int msecs) {
		
		if(_owner){
			if (_physicComponent){
				_physicComponent->setPosition(_owner->getPosition(), true);
			}
		
			if (_cGraph){
				
				_entity->setPosition(_owner->getPosition());
			}	
			/*
			printf("\nOwner: %s position %d, %d, %d",_owner->getName().c_str(),_owner->getPosition().x,_owner->getPosition().y,_owner->getPosition().z);
			printf("\nShield: %s position %d, %d, %d",_entity->getName().c_str(),_entity->getPosition().x,_entity->getPosition().y,_entity->getPosition().z);
			*/
		}
	}
	

	//________________________________________________________________________

	void CShieldSpellController::onActivate() {
		//_reducedDamageAbsorption = 0;
	}
	//________________________________________________________________________

	void CShieldSpellController::touched(CEntity *entityTouched)
	{
		//std::cout << "Tocado! al principio me toca a mi" << std::endl;
		if(entityTouched->getEntityID() != _owner->getEntityID()){
			// He de comprobar que es amigo, o eso, o en el filtro que solo le de a los amigos
			std::shared_ptr<CMessageDamaged> damageDone = std::make_shared<CMessageDamaged>();
			damageDone->setDamage(_damage);
			damageDone->setEnemy( _owner );
			entityTouched->emitMessage(damageDone);
		}

	}
	//________________________________________________________________________


} // namespace Logic

