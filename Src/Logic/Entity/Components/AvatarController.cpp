/**
@file AvatarController.cpp

Contiene la implementaci�n del componente que controla el movimiento 
de la entidad.
 
@see Logic::CAvatarController
@see Logic::IComponent

@author David Llans�
@date Agosto, 2010
*/

#include "AvatarController.h"

#include "Logic/Entity/Entity.h"
#include "Map/MapEntity.h"


namespace Logic 
{
	IMP_FACTORY(CAvatarController);
	
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

	bool CAvatarController::activate()
	{
		_jumping = false;
		_timeJump = 0.0;
		return true;
	} // activate
	
	//---------------------------------------------------------

	void CAvatarController::deactivate()
	{

	} // deactivate
	
	//---------------------------------------------------------

	bool CAvatarController::accept(const TMessage &message)
	{
		return message._type == Message::CONTROL;

	} // accept
	
	//---------------------------------------------------------

	void CAvatarController::process(const TMessage &message)
	{
		switch(message._type)
		{
		case Message::CONTROL:
			if(!message._string.compare("walk"))
				walk();
			else if(!message._string.compare("walkBack"))
				walkBack();
			else if(!message._string.compare("stopWalk"))
				stopWalk();
			else if(!message._string.compare("stopWalkBack"))
				stopWalkBack();
			else if(!message._string.compare("strafeLeft"))
				strafeLeft();
			else if(!message._string.compare("strafeRight"))
				strafeRight();
			else if(!message._string.compare("stopStrafeLeft"))
				stopStrafeLeft();
			else if(!message._string.compare("stopStrafeRight"))
				stopStrafeRight();
			else if(!message._string.compare("mouse"))
				mouse(message._mouse);
			else if(!message._string.compare("jump"))
				jump();

		}

	} // process
	
	//---------------------------------------------------------

	void CAvatarController::mouse(const float* amount) 
	{
		_entity->yaw(amount[0]);
		_entity->pitch(amount[1]);

	} // turn
	
	//---------------------------------------------------------

	void CAvatarController::walk() 
	{
		_walking = true;

	} // walk
	
	//---------------------------------------------------------

	void CAvatarController::walkBack() 
	{
		_walkingBack = true;

	} // walkBack
	
	//---------------------------------------------------------

	void CAvatarController::stopWalk() 
	{
		_walking = false;

	} // stopWalk
	
		//---------------------------------------------------------

	void CAvatarController::stopWalkBack() 
	{
		 _walkingBack = false;

	} // stopWalk


	//---------------------------------------------------------

	void CAvatarController::strafeLeft() 
	{
		_strafingLeft = true;

	} // walk
	
	//---------------------------------------------------------

	void CAvatarController::strafeRight() 
	{
		_strafingRight = true;

	} // walkBack
	
	//---------------------------------------------------------

	void CAvatarController::stopStrafeLeft() 
	{
		_strafingLeft = false;

	} // stopWalk

	//---------------------------------------------------------

		void CAvatarController::stopStrafeRight() 
	{
		_strafingRight = false;

	} // stopWalk

	//---------------------------------------------------------

	void CAvatarController::jump(){
		/*
		//si ya estaba saltando, pasamos
		if(_jumping)
			return;

		_jumping = true;
		*/
	}//jump
	
	//---------------------------------------------------------

	void CAvatarController::tick(unsigned int msecs)
	{
		IComponent::tick(msecs);

		//si estamos saltando, realizamos una simulaci�n de un salto (provisional hasta tener physX)
		if(_jumping){
			if(_timeJump > 600){
				_timeJump = 0.0;
				_jumping = false;
			}else if (_timeJump >=300){
				Vector3 direction(0,-1,0);
				direction *= msecs * 0.05;
				_timeJump+=msecs;
				_entity->setPosition(_entity->getPosition()+direction);
			}else{
				Vector3 direction(0,1,0);
				direction *= msecs * 0.05;
				_timeJump+=msecs;
				_entity->setPosition(_entity->getPosition()+direction);
			}
		}//if (_jumping)

		// Si nos estamos desplazando calculamos la pr�xima posici�n
		// Calculamos si hay vectores de direcci�n de avance y strafe,
		// hayamos la direcci�n de la suma y escalamos seg�n la
		// velocidad y el tiempo transcurrido.
		if(_walking || _walkingBack || _strafingLeft || _strafingRight)
		{
			Vector3 direction(Vector3::ZERO);
			Vector3 directionStrafe(Vector3::ZERO);

			if(_walking || _walkingBack)
			{
				direction = Math::getDirection(_entity->getYaw());
				if(_walkingBack)
					direction *= -1;
				//Anulacion forward/back
				if(_walking && _walkingBack)
					direction *= 0;
			}

			if(_strafingLeft || _strafingRight)
			{
				directionStrafe = 
						Math::getDirection(_entity->getYaw() + Math::PI/2);
				if(_strafingRight)
					directionStrafe *= -1;
				//Para facilitar el strafe, anulaci�n de strafes
				if(_strafingLeft && _strafingRight)
					directionStrafe *= 0;
			}

			direction += directionStrafe;
			direction.normalise();
			direction *= msecs * _speed;
			Vector3 newPosition = _entity->getPosition() + direction;
			_entity->setPosition(newPosition);

			TMessage message;
			message._type = Message::AVATAR_WALK;
			message._vector3 = direction;
			_entity->emitMessage(message);
		}

	} // tick

} // namespace Logic

