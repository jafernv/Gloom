/**
@file AvatarController.cpp

Contiene la implementaci�n del componente que controla el movimiento 
de la entidad.
 
@see Logic::CAvatarController
@see Logic::IComponent

@author Rub�n Mulero Guerrero
@author Francisco Aisa Garc�a
@date Abril, 2013
*/

#include "AvatarController.h"
#include "Logic/Entity/Entity.h"
#include "Map/MapEntity.h"
#include "Logic/Messages/MessageControl.h"
#include "Logic/Entity/Components/PhysicController.h"

namespace Logic 
{
	IMP_FACTORY(CAvatarController);

	CAvatarController::CAvatarController(){}
	
	//---------------------------------------------------------

	bool CAvatarController::spawn(CEntity *entity, CMap *map, const Map::CEntity *entityInfo) 
	{
		if(!IComponent::spawn(entity,map,entityInfo))
			return false;
		
		if(entityInfo->hasAttribute("speed"))
			_speed = entityInfo->getFloatAttribute("speed");

		return true;

	} // spawn
	
	//---------------------------------------------------------

	void CAvatarController::activate()
	{
		IComponent::activate();
		_directionSpeed = Vector3::ZERO;
	} // activate
	
	//---------------------------------------------------------

	void CAvatarController::deactivate()
	{
		IComponent::deactivate();
	} // deactivate
	
	//---------------------------------------------------------

	bool CAvatarController::accept(CMessage *message)
	{
		return message->getMessageType() == Message::CONTROL;
	} // accept
	
	//---------------------------------------------------------

	void CAvatarController::process(CMessage *message)
	{
		//std::cout << "menssaje de avatar controller recibido" << std::endl;
		switch(message->getMessageType())
		{
			case Message::CONTROL:{
				CMessageControl* controlMsg = static_cast<CMessageControl*> (message) ;

				executeControlCommand(controlMsg);

				break;
			}
		}

	} // process
	
	//---------------------------------------------------------

	void CAvatarController::mouse(const float* amount) 
	{
		_entity->yaw(amount[0]);
		_entity->pitch(amount[1]);
	} // turn
	
	//---------------------------------------------------------

	void CAvatarController::tick(unsigned int msecs)
	{
		//@deprecated
		IComponent::tick(msecs);
		if(_directionSpeed == Vector3::ZERO)return;
		float yaw = asin(_directionSpeed.normalisedCopy().x);
		Matrix4 dir = _entity->getTransform();
		Math::yaw(yaw, dir);
		Vector3 direction;

		if(_directionSpeed.z!=0)
			direction = (Math::getDirection(dir)).normalisedCopy()*_directionSpeed.z;
		else
			direction = (Math::getDirection(dir)).normalisedCopy();
		direction*=_speed*msecs;

		unsigned flags = _entity->getComponent<CPhysicController>("CPhysicController")->moveController(direction,msecs);

		_entity->setPosition(_entity->getPosition()+direction);

	} // tick


	void CAvatarController::executeControlCommand(CMessageControl *controlMsg){


		switch(controlMsg->getType()){
			case Control::WALK:{
				_directionSpeed+=Vector3(0,0,1);
				break;
			}
			case Control::WALKBACK:{
				_directionSpeed+=Vector3(0,0,-1);
				break;
			}
			case Control::STRAFE_LEFT:{
				_directionSpeed+=Vector3(1,0,0);
				break;
			}

			case Control::STRAFE_RIGHT:{
				_directionSpeed+=Vector3(-1,0,0);
				break;
			}

			case Control::STOP_WALK:{
				_directionSpeed+=Vector3(0,0,-1);
				break;
			}
			case Control::STOP_WALKBACK:{
				_directionSpeed+=Vector3(0,0,1);
				break;
			}
			case Control::STOP_STRAFE_LEFT:{
				_directionSpeed+=Vector3(-1,0,0);
				break;
			}

			case Control::STOP_STRAFE_RIGHT:{
				_directionSpeed+=Vector3(1,0,0);
				break;
			}

		}
	}

} // namespace Logic

