/**
@file ClientRespawn.cpp

@author Francisco Aisa Garc�a
@date Febrero, 2013
*/

#include "ClientRespawn.h"
#include "AvatarController.h"

#include "Logic/Maps/EntityFactory.h"
#include "Logic/Entity/Entity.h"
#include "PhysicController.h"
#include "Logic/Server.h"
#include "Logic/Maps/Map.h"
#include "Logic/Maps/Scoreboard.h"
#include "Map/MapEntity.h"
#include "Basesubsystems/Math.h"
#include "Interpolation.h"
#include "Audio/Server.h"

#include "Logic/Messages/MessagePlayerDead.h"
#include "Logic/Messages/MessagePlayerSpawn.h"
#include "Logic/Maps/GUIKillersMessage.h"
#include "../../GameNetPlayersManager.h"
#include "Logic/Messages/MessageHudSpawn.h"

#include <math.h>

#include <iostream>

namespace Logic  {
	
	IMP_FACTORY(CClientRespawn);

	//________________________________________________________________________
	
	bool CClientRespawn::spawn(CEntity *entity, CMap *map, const Map::CEntity *entityInfo) 
	{
		if(!IComponent::spawn(entity,map,entityInfo))
			return false;

		if(entityInfo->hasAttribute("audioSpawn"))
			_audioSpawn =  entityInfo->getStringAttribute("audioSpawn");

		return true;

	} // spawn
	//________________________________________________________________________

	bool CClientRespawn::accept(const std::shared_ptr<CMessage>& message) {
		TMessageType msgType = message->getMessageType();

		return msgType == Message::PLAYER_DEAD		||
			   msgType == Message::PLAYER_SPAWN;
		//return false;
	} // accept

	//________________________________________________________________________

	void CClientRespawn::process(const std::shared_ptr<CMessage>& message) {
		switch( message->getMessageType() ) {
			case Message::PLAYER_DEAD: {
				// @deprecated Esto no deberia hacerse aqui pero como ma�ana es la gamelab
				// lo dejo asi. El problema es que el componente del servidor no procesa
				// el mensaje de audio de muerte mandado desde el cliente
				/*std::shared_ptr<CMessageAudio> audioMsg = std::make_shared<CMessageAudio>();
				audioMsg->setAudioName("slapchot.wav");
				audioMsg->is3dSound(true);
				_entity->emitMessage(audioMsg);*/

				// El servidor nos notifica de que hemos muerto, desactivamos
				// la entidad al completo y su simulacion fisica para que no
				// podamos colisionar con la c�psula del jugador.

				// Desactivamos todos los componentes menos estos
				std::set<std::string> exceptionList;
				exceptionList.insert( std::string("CClientRespawn") );
				exceptionList.insert( std::string("CAnimatedGraphics") );
				exceptionList.insert( std::string("CPlayerInterpolator") );
				exceptionList.insert( std::string("CHudOverlay") );
				exceptionList.insert( std::string("CHud") );
				exceptionList.insert( std::string("CNetConnector") );
				exceptionList.insert( std::string("CAudio") );
				exceptionList.insert( std::string("CRagdoll") );
				exceptionList.insert( std::string("CAvatarController") );
				
				// En caso de estar simulando fisica en el cliente, desactivamos
				// la c�psula.
				_entity->getComponent<CPhysicController>("CPhysicController")->deactivateSimulation();

				CAvatarController* component = _entity->getComponent<CAvatarController>("CAvatarController");
				if(component != NULL) {
					exceptionList.insert( std::string("CAvatarController") );
					_entity->getComponent<CAvatarController>("CAvatarController")->putToSleep(true);
				}

				_entity->deactivateAllComponentsExcept(exceptionList);

				//mostramos en pantalla el mensaje de quien ha matado a quien
				std::shared_ptr<CMessagePlayerDead> playerDeadMsg = std::static_pointer_cast<CMessagePlayerDead> (message);
				CEntity* killer = playerDeadMsg->getKiller();
			
				if(killer != NULL)
					updateGUI(killer);

				break;
			}
			case Message::PLAYER_SPAWN: {

				std::cout << "mensaje spawn recibido" << std::endl;


				// El servidor nos notifica de que debemos respawnear. Activamos
				// todos los componentes de la entidad y seteamos nuestra posicion
				// en el lugar indicado por el mensaje recibido del servidor.

				std::shared_ptr<CMessagePlayerSpawn> playerSpawnMsg = std::static_pointer_cast<CMessagePlayerSpawn>(message);
				// Colocamos al player en la posicion dada por el manager de spawn del server,
				// podemos asumir sin problemas que el player tiene capsula

				Vector3 spawnPos = playerSpawnMsg->getSpawnPosition();

				Map::CEntity* entityInfo = CEntityFactory::getSingletonPtr()->getInfo("SpawnParticles");
				CEntity* spawnParticles = CEntityFactory::getSingletonPtr()->createEntity(entityInfo, _entity->getMap(), spawnPos, Quaternion::IDENTITY, false);
				spawnParticles->activate();
				spawnParticles->start();

				Audio::CServer::getSingletonPtr()->playSound3D("gameplay/spawn.wav", spawnPos, Vector3::ZERO, false, false);

				_entity->getComponent<CPhysicController>("CPhysicController")->setPhysicPosition(spawnPos);
				// Activamos la simulacion aqui sin problemas. El componente life ignora los mensajes de da�o
				// hasta que no desaparece la inmunidad del respawn. 
				_entity->getComponent<CPhysicController>("CPhysicController")->activateSimulation();

				CAvatarController* component = _entity->getComponent<CAvatarController>("CAvatarController");
				if(component != NULL) {
					component->wakeUp();
				}

				// Seteamos la orientacion a la dada por el server
				_entity->setOrientation(playerSpawnMsg->getSpawnOrientation());


				// Volvemos a activar todos los componentes
				_entity->activate();

				Vector3 entityPos = _entity->getPosition();

				std::shared_ptr<CMessagePlayerSpawn> cameraPlayerSpawnMsg = std::make_shared<CMessagePlayerSpawn>();
				CEntity* cam = CServer::getSingletonPtr()->getMap()->getEntityByName("Camera");
				assert(cam != NULL && "Error: Esto no se puede hacer asi que sois unos lamers, ahora el servidor que hace?");
				cam->emitMessage(cameraPlayerSpawnMsg);

				std::shared_ptr<CMessageHudSpawn> messageHudSpawn = std::make_shared<CMessageHudSpawn>();
				messageHudSpawn->setTime(0);
				_entity->emitMessage(messageHudSpawn);

				//Sonido Spawn
				Audio::CServer::getSingletonPtr()->playSound3D(_audioSpawn, _entity->getPosition(), Vector3::ZERO);

				break;
			}
		}
	} // process

	void CClientRespawn::updateGUI(CEntity* killer){
		//updateamos el mensaje de pepito mato a juanito
		std::string type = killer->getType();
		TEntityID killerId = killer->getEntityID();
		TEntityID playerId = _entity->getEntityID();
			
		if(type == "Screamer" || type == "Shadow" || type == "Hound" || type == "Archangel" ||
			type == "LocalScreamer" || type == "LocalShadow" || type == "LocalHound" || type == "LocalArchangel"){
			Logic::GUIKillersMessage::getSingletonPtr()->addKiller(
				killer->getName(),
				_entity->getName());

			Logic::CScoreboard::getSingletonPtr()->addKill(killer->getName());
			Logic::CScoreboard::getSingletonPtr()->addDeath(_entity->getName());

			Logic::CGameNetPlayersManager* playersMgr = Logic::CGameNetPlayersManager::getSingletonPtr();
			if(killer == _entity) // Nos hemos suicidado
				playersMgr->substractFragUsingEntityID(killerId);
			else {
				TeamFaction::Enum killerTeam = playersMgr->getTeamUsingEntityId(killerId);
				TeamFaction::Enum playerTeam = playersMgr->getTeamUsingEntityId(playerId);

				if(killerTeam != TeamFaction::eNONE || playerTeam != TeamFaction::eNONE || killerTeam != playerTeam)
					playersMgr->addFragUsingEntityID(killerId); // Hemos matado a un enemigo, contamos el frag
				else
					playersMgr->substractFragUsingEntityID(killerId); // Hemos matado a un compa�ero restamos frags
			}

			playersMgr->addDeathUsingEntityID(playerId);
		}
		//sino ha sido un player es que se ha suicidado el retard
		else{
			Logic::GUIKillersMessage::getSingletonPtr()->suicide(_entity->getName());
			Logic::CScoreboard::getSingletonPtr()->addDeath(_entity->getName());
			Logic::CScoreboard::getSingletonPtr()->subKill(_entity->getName());

			Logic::CGameNetPlayersManager* playersMgr = Logic::CGameNetPlayersManager::getSingletonPtr();
			playersMgr->substractFragUsingEntityID(playerId);
			playersMgr->addDeathUsingEntityID(playerId);
		}
	}


} // namespace Logic

