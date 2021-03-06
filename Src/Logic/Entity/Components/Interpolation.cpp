/**
@file Interpolation.cpp

@author Francisco Aisa Garc�a
@author Ruben Mulero Guerrero
@date Febrero, 2013
*/

#include "Interpolation.h"

#include "Logic/Entity/Entity.h"
#include "PhysicController.h"
#include "AvatarController.h"
#include "Logic/Server.h"
#include "Logic/Maps/Map.h"
#include "Map/MapEntity.h"
#include "Basesubsystems/Math.h"

#include "Logic/Messages/MessageSyncPosition.h"
#include "Logic/Messages/MessageSetPhysicPosition.h"
#include "Logic/Messages/MessageControl.h"
#include "Logic/Messages/MessageMouse.h"

#include <math.h>

#include <iostream>

namespace Logic  {
	
	IMP_FACTORY(CInterpolation);

	//________________________________________________________________________

	bool CInterpolation::spawn(CEntity *entity, CMap *map, const Map::CEntity *entityInfo) {
		if(!IComponent::spawn(entity,map,entityInfo))
			return false;
		// Indicar parametros de interpolacion (�apeado de momento)
		_interpolating = false;
		_maxDistance = 25.0f;
		_minDistance = 0.05f;
		_minYaw = 1.5;
		_maxYaw = 15;
		_yawDifference = 0;
		_rotationSpeed = 0.2;
		_serverDirection = Vector3(0,0,0);
		_distance = 0;
		return true;
	} // spawn

	//________________________________________________________________________

	bool CInterpolation::accept(const std::shared_ptr<CMessage>& message) {
		return false;
	} // accept
	
	//________________________________________________________________________

	void CInterpolation::onFixedTick(unsigned int msecs){
		_msecs = msecs;

		//si no estamos interpolando, gl
		if(!_interpolating)
			return;

		//lo primero de todo, movemos la posici�n del servidor para poder interpolar con m�s exactitud
		Vector3 newPos;
		//calculamos la direccion en la que debemos interpolar
		Vector3 direction = (_serverDirection*Vector3(1,0,1)).normalisedCopy();
		//calculamos el movimiento que debe hacer el monigote, mucho mas lento del que debe hacer de normal
		direction*=(_entity->getComponent<CAvatarController>("CAvatarController")->getVelocity()*Vector3(1,0,1)).length()*0.25f;

		//si nos hemos pasado, debemos moverlo al sitio
		if(direction.length() > _distance){
			direction*=(_distance/direction.length());
		}
		_entity->getComponent<CPhysicController>("CPhysicController")->move(direction,msecs);
		_distance -= direction.length();

		//si hemos terminado de interpolar, lo dejamos
		if((_distance < _minDistance)){
			_interpolating = false;
		}
		
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////

	void CInterpolation::process(const std::shared_ptr<CMessage>& message) {
		switch(message->getMessageType())
		{
		case Message::SYNC_POSITION:
			{
			std::shared_ptr<CMessageSyncPosition> syncMsg = std::static_pointer_cast<CMessageSyncPosition>(message);

			// nos guardamos la posi que nos han dado por si tenemos que interpolar
			/*_serverPos = syncMsg->getTransform();
			//calculo el ping que tengo ahora mismo	
			int time = clock()+Logic::CServer::getSingletonPtr()->getDiffTime();

			std::cout << time << std::endl;

			int ping = clock()+Logic::CServer::getSingletonPtr()->getDiffTime()-syncMsg->getTime();
			_actualPing = abs(ping);
			//calculamos la interpolacion
			calculateInterpolation();*/
			break;
			}
		}
	} // process

////////////////////////////////////////////////////////////////////////////////////////////////////////

	void CInterpolation::calculateInterpolation(){
		
		//primero debemos setear la posici�n del servidor, suponiendo donde debe estar en el tiempo
		//que ha tardado la informaci�n en llegar hasta aqu� y utilizando la velocidad a la que
		//nos estamos moviendo actualmente
		Vector3 serverPos = _serverPos.getTrans();

		if((serverPos-_entity->getPosition()).length()< _minDistance)
			return;
		Vector3 serverDisplacement = _entity->getComponent<CAvatarController>("CAvatarController")->getVelocity();

		//esta es la posi que suponemos que tiene el server en eeste momento
		if(_actualPing > _msecs)
			serverDisplacement*=_actualPing/_msecs;
		else
			serverPos+=serverDisplacement;

		//direccion de interpolaci�n
		Vector3 intDirection = (serverPos - _entity->getPosition())*Vector3(1,0,1);

		float distance = intDirection.length();

		//si nuestra distancia es inadmisible, lo ponemos donde nos ha dicho el servidor mas lo que hemos supuesto
		if(distance > _maxDistance){
			_entity->getComponent<CPhysicController>("CPhysicController")->setPhysicPosition(serverPos);
			//Movemos la orientacion logica/grafica
			Matrix3 tmpMatrix;
			_serverPos.extract3x3Matrix(tmpMatrix);
			_entity->setOrientation(tmpMatrix);
			return;
		}
		//si la distancia es mayor que min distance y menor que maxDistance interpolamos
		else if(distance > _minDistance){
			//nos guardamos la direcci�n en la que tenemos que interpolar
			_serverDirection = intDirection.normalisedCopy();
			_distance = distance;

			_interpolating = true;

		}
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////

	void CInterpolation::setLerpLevel(Vector3 lerp){
		if(lerp==Vector3::ZERO){
			_interpolating=false;
			return;
		}
		_serverDirection = lerp;
		_distance = lerp.length();
		_interpolating = true;
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////

	void CInterpolation::onActivate(){
		_serverDirection = Vector3(0,0,0);
		_interpolating = false;
		_distance = 0;
	}

} // namespace Logic

