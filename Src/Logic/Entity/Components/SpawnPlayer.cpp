/**
@file SpawnPlayer.cpp

Contiene la implementaci�n del componente que gestiona el spawn del jugador.
 
@see Logic::CSpawnPlayer
@see Logic::IComponent

*/

#include "SpawnPlayer.h"
#include "AvatarController.h"
#include "PhysicController.h"

#include "Net/Manager.h"
#include "Audio/Server.h"

#include "Logic/Maps/EntityFactory.h"
#include "Logic/Entity/Entity.h"
#include "Map/MapEntity.h"
#include "Logic/Maps/Map.h"
#include "Logic/Server.h"
#include "Logic/GameSpawnManager.h"
#include "PhysicController.h"
#include "Logic/GameNetMsgManager.h"
#include "Logic/Entity/Components/Life.h"
#include "Logic/Maps/WorldState.h"

#include "Logic/Messages/MessagePlayerDead.h"
#include "Logic/Messages/MessagePlayerSpawn.h"
#include "Logic/Messages/MessageSetPhysicPosition.h"
#include "Logic/Messages/MessageHudSpawn.h"
#include "Logic/Messages/MessageCreateParticle.h"
#include "Logic/Messages/MessageSpawnIsLive.h"

#include "Graphics/Scene.h"
#include "Graphics/Camera.h"

namespace Logic 
{
	IMP_FACTORY(CSpawnPlayer);
	
	//---------------------------------------------------------

	bool CSpawnPlayer::spawn(CEntity *entity, CMap *map, const Map::CEntity *entityInfo) 
	{
		if(!IComponent::spawn(entity,map,entityInfo))
			return false;

		if(entityInfo->hasAttribute("timeSpawn")){
			_timeSpawn= entityInfo->getIntAttribute("timeSpawn");
			_timeSpawn*=1000;
		}

		if(entityInfo->hasAttribute("audioSpawn"))
			_audioSpawn =  entityInfo->getStringAttribute("audioSpawn");

		return true;

	} // spawn
	//---------------------------------------------------------

	void CSpawnPlayer::onActivate()
	{
		_isDead=false;
		_actualTimeSpawn=_inmunityTimer=0;
		_reactivePhysicSimulation=false;

	} // activate
	//--------------------------------------------------------


	bool CSpawnPlayer::accept(const std::shared_ptr<CMessage>& message)
	{
		return message->getMessageType() == Message::PLAYER_DEAD;
	} // accept
	//---------------------------------------------------------


	void CSpawnPlayer::process(const std::shared_ptr<CMessage>& message)
	{
		switch(message->getMessageType())
		{
		case Message::PLAYER_DEAD:
			dead();
			break;
		}

	} // process
	//---------------------------------------------------------

	void CSpawnPlayer::onTick(unsigned int msecs) {
		//Solamente si estamos muertos (se recibi� el mensaje)
		if(_isDead){
			_actualTimeSpawn+=msecs;
			//Si superamos el tiempo de spawn tenemos que revivir
			if(_actualTimeSpawn>_timeSpawn){
				_isDead = false;
				_actualTimeSpawn = 0;
				//LLamamos al manager de spawn que nos devolver� una posici�n
				CEntity *spawn = CServer::getSingletonPtr()->getSpawnManager()->getSpawnPosition();

				Vector3 spawnPos = spawn->getPosition();

				Map::CEntity* entityInfo = CEntityFactory::getSingletonPtr()->getInfo("SpawnParticles");
				CEntity* spawnParticles = CEntityFactory::getSingletonPtr()->createEntity(entityInfo, _entity->getMap(), spawnPos, Quaternion::IDENTITY, false);
				spawnParticles->activate();
				spawnParticles->start();

				if( !Net::CManager::getSingletonPtr()->imServer() )
					Audio::CServer::getSingletonPtr()->playSound3D("gameplay/spawn.wav", spawnPos, Vector3::ZERO, false, false);

				//Ponemos la entidad f�sica en la posici�n instantaneamente ( no se puede permitir el envio de mensajes )
				//La simulacion fisica tiene que ser activada en el siguiente tick, para que el player se resitue bien
				_entity->getComponent<CPhysicController>("CPhysicController")->setPhysicPosition(spawnPos);
				_reactivePhysicSimulation=true;

				//Volvemos a activar todos los componentes(lo que hace resetea _isDead y _actualTimeSpawn)
				_entity->activate();

				// Informamos al estado del mundo de que ha cambiado el estado de muerte de la entidad
				Logic::CWorldState::getSingletonPtr()->deleteChange(_entity, Message::PLAYER_DEAD);

				//Establecemos la orientaci�n adecuada segun la devoluci�n del manager de spawn
				_entity->setOrientation(spawn->getYaw());
				
				// Si eres el server mandar un mensaje de spawn
				std::shared_ptr<CMessagePlayerSpawn> spawnMsg = std::make_shared<CMessagePlayerSpawn>();
				spawnMsg->setSpawnPosition( _entity->getPosition() );
				spawnMsg->setSpawnOrientation( _entity->getYaw());
				_entity->emitMessage(spawnMsg);

				std::cout << "enviando mensaje de spawn" << std::endl;

				CEntity* camera = CServer::getSingletonPtr()->getMap()->getEntityByName("Camera");
				assert(camera != NULL && "Error: Esto no se puede hacer asi que sois unos lamers, ahora el servidor que hace?");
				camera->emitMessage(spawnMsg);
				
				//if(Net::CManager::getSingletonPtr()->imServer())
				//	Logic::CGameNetMsgManager::getSingletonPtr()->sendMessageToOne(new CMessagePlayerSpawn(), camera->getEntityID(), _entity->getEntityID());
				//Mirar porque se cre� esto, lo mismo antonio sabe
				/*
				std::shared_ptr<CMessageCreateParticle> particle =std::make_shared<CMessageCreateParticle>();
				particle->setParticle("SpawnParticle");
				particle->setPosition(_entity->getPosition());
				*/

				_inmunityTimer = 0;
			}
		}
		else if(_reactivePhysicSimulation){
			_entity->getComponent<CPhysicController>("CPhysicController")->activateSimulation();
			CAvatarController* avatarComp = _entity->getComponent<CAvatarController>("CAvatarController");
			if(avatarComp != NULL) 
				avatarComp->wakeUp();

			// Comprobamos el timer de inmunidad al respawnear
			_inmunityTimer += msecs;
			if(_inmunityTimer > _inmunityTime) {
				_inmunityTimer = 0;
				_entity->emitMessage( std::make_shared<CMessageSpawnIsLive>() );

				_reactivePhysicSimulation=false;
				putToSleep();
			}
		}
	} // tick
	//---------------------------------------------------------

	void CSpawnPlayer::dead()
	{
		//Si no esto muerto ya hago las acciones
		if(!_isDead){
			//Desactivamos todos menos el cspawnplayer
			std::set<std::string> except;
			except.insert( std::string("CAnimatedGraphics") );
			except.insert( std::string("CSpawnPlayer") );
			except.insert( std::string("CHudOverlay") );
			except.insert( std::string("CHud") );
			except.insert( std::string("CNetConnector") );
			except.insert( std::string("CAvatarController") );
			except.insert( std::string("CAudio") );
			except.insert( std::string("CPlayerSnapshot") );
			except.insert( std::string("CParticle") );
			except.insert( std::string("CRagdoll") );

			//Desactivamos la simulaci�n f�sica (no puede estar activo en la escena f�sica al morir)
			_entity->getComponent<CPhysicController>("CPhysicController")->deactivateSimulation();
			CAvatarController* avatarComp = _entity->getComponent<CAvatarController>("CAvatarController");
			if(avatarComp != NULL) 
				avatarComp->putToSleep(true);

			_entity->deactivateAllComponentsExcept(except);
			_isDead=true;
			//Mensaje para el Hud (tiempo de spawn)
			std::shared_ptr<CMessageHudSpawn> m = std::make_shared<CMessageHudSpawn>();
			m->setTime(_timeSpawn/1000);
			_entity->emitMessage(m);
		}

	} // process
	//---------------------------------------------------------

} // namespace Logic

