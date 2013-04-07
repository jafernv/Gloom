/**
@file Camera.cpp

Contiene la implementaci�n del componente que controla la c�mara gr�fica
de una escena.
 
@see Logic::CCamera
@see Logic::IComponent

@author David Llans�
@date Agosto, 2010
*/

#include "Camera.h"

#include "Logic/Server.h"
#include "Logic/Entity/Entity.h"
#include "Logic/Maps/Map.h"
#include "Map/MapEntity.h"

#include "Graphics/Scene.h"
#include "Graphics/Camera.h"

#include "Interpolation.h"
#include "AvatarController.h"

#include "Logic/Messages/MessageCameraToEnemy.h"

namespace Logic 
{
	IMP_FACTORY(CCamera);
	
	//---------------------------------------------------------

	bool CCamera::spawn(CEntity *entity, CMap *map, const Map::CEntity *entityInfo) 
	{
		if(!IComponent::spawn(entity,map,entityInfo))
			return false;
		Vector3 camPos;
		_graphicsCamera = _entity->getMap()->getScene()->getCamera();
		if(!_graphicsCamera)
			return false;

		if(entityInfo->hasAttribute("height"))
			_height = entityInfo->getFloatAttribute("height");
		if(entityInfo->hasAttribute("target"))
			_targetV = entityInfo->getVector3Attribute("target");
		if(entityInfo->hasAttribute("position")){
			camPos = entityInfo->getVector3Attribute("position");
			_graphicsCamera->setCameraPosition(camPos);
		}
		_dead=false;
		return true;

	} // spawn
	
	//---------------------------------------------------------

	void CCamera::activate()
	{
		IComponent::activate();
		
		_target = CServer::getSingletonPtr()->getPlayer();

		if(!_target){
			_graphicsCamera->setTargetCameraPosition(_targetV);
			_target = NULL;
			//deactivate();
		}

		//return true;

	} // activate
	
	//---------------------------------------------------------

	void CCamera::deactivate()
	{
		IComponent::deactivate();

		_target = 0;

	} // deactivate
	
	//---------------------------------------------------------

		bool CCamera::accept(CMessage *message)
	{
		return message->getMessageType() == Message::CAMERA_TO_ENEMY ||
				 message->getMessageType() == Message::PLAYER_SPAWN;
	} // accept
	//---------------------------------------------------------


	void CCamera::process(CMessage *message)
	{
		switch(message->getMessageType())
		{
		case Message::CAMERA_TO_ENEMY:
				setTargetEnemy((CMessageCameraToEnemy*)message);
			break;

		case Message::PLAYER_SPAWN:
				_dead=false;
				std::cout << "mensaje respawn recibido" << std::endl;
			break;
		}

	} // process
	//---------------------------------------------------------

	void CCamera::setTargetEnemy(CMessageCameraToEnemy* message){
		
		_enemy=message->getEnemy();
		_dead=true;
	}

	//---------------------------------------------------------

	void CCamera::tick(unsigned int msecs)
	{
		IComponent::tick(msecs);
		
		if(_target)
		{
			// Actualizamos la posici�n de la c�mara.
			Vector3 position = _target->getPosition();
			position.y+=_height;
			_graphicsCamera->setCameraPosition(position);

			
			if(!_dead){
				// Y la posici�n hacia donde mira la c�mara.
				Vector3 direction = Math::getDirection(_target->getOrientation());
				_graphicsCamera->setTargetCameraPosition(position + direction);
				//_graphicsCamera->setCameraDirection(direction);
			}
			//Si estamos muertos miramos al enemigo, diferenciamos entre nosotros mismos o el rival
			else if(_enemy){
				 if(_enemy->getType().compare("LocalPlayer")!=0)
				    _graphicsCamera->setTargetCameraPosition(_enemy->getPosition());
				 else{	
					 _graphicsCamera->setCameraPosition(_enemy->getPosition()+Vector3(0,50,0));
					 _graphicsCamera->setTargetCameraPosition(_enemy->getPosition());
				 }
			}

		}else{
			_graphicsCamera->setTargetCameraPosition(_targetV);
		}
	} // tick

} // namespace Logic

