//---------------------------------------------------------------------------
// GameServerState.h
//---------------------------------------------------------------------------

/**
@file GameServerState.h

Contiene la declaraci�n del estado de juego.

@see Application::CGameServerState
@see Application::CGameState

@author Francisco Aisa Garc�a
@date Febrero, 2013
*/

#ifndef __Application_GameServerState_H
#define __Application_GameServerState_H

#include "GameState.h"
#include "Net/Manager.h"
#include "Net/buffer.h"
#include "Logic/Maps/WorldState.h"
#include "Logic/Maps/TeamFaction.h"

namespace Logic {
	class CGameNetPlayersManager;
	class CMap;
}

namespace Application {
	
	/**
	@ingroup applicationGroup

	Representa el estado de juego online para el servidor. En el futuro
	tendremos diferentes estados por cada modo de juego, pero de momento
	se queda asi hasta que por lo menos tengamos un deathmatch.

	Hereda de CGameState por que es un estado de juego y de Net::CManager::IObserver
	para que se le avise de que ha recibido paquetes.

	@author Francisco Aisa Garc�a
	@date Febrero, 2013
	*/

	class CGameServerState : public CGameState, public Net::CManager::IObserver, public Logic::CWorldState::IObserver {
	public:


		// =======================================================================
		//                      CONSTRUCTORES Y DESTRUCTOR
		// =======================================================================


		/**
		Constructor por parametro.

		@param app Aplicacion que se encarga de manejar los estados.
		*/
		CGameServerState(CBaseApplication *app, GameMode::Enum mode) : CGameState(app), _gameMode(mode), _playersMgr(NULL),
																	   _autoBalanceTeams(false) { }


		// =======================================================================
		//                           METODOS HEREDADOS
		// =======================================================================


		/**
		Funci�n llamada por la aplicaci�n cuando se activa
		el estado.
		*/
		virtual void activate();

		//______________________________________________________________________________

		/**
		Funci�n llamada por la aplicaci�n cuando se desactiva
		el estado.
		*/
		virtual void deactivate();

		//______________________________________________________________________________

		/**
		Se dispara cuando se recibe un paquete de datos.

		@see Net::CManager::IObserver
		@param packet Paquete de datos recibido.
		*/
		virtual void dataPacketReceived(Net::CPaquete* packet);

		//______________________________________________________________________________

		/**
		Se dispara cuando se recibe un paquete de conexion.

		@see Net::CManager::IObserver
		@param packet Paquete de conexion recibido.
		*/
		virtual void connectionPacketReceived(Net::CPaquete* packet);

		//______________________________________________________________________________

		/**
		Se dispara cuando se recibe un paquete de desconexion.

		@see Net::CManager::IObserver
		@param packet Paquete de desconexion recibido.
		*/
		virtual void disconnectionPacketReceived(Net::CPaquete* packet);

		//______________________________________________________________________________

		// M�todos de CKeyboardListener

		/**
		M�todo que ser� invocado siempre que se pulse una tecla. 
		Ser� la aplicaci�n qui�n llame a este m�todo cuando el 
		estado est� activo. Esta clase NO se registra en el 
		InputManager sino que es la aplicaci�n quien lo hace y 
		delega en los estados.

		@param key C�digo de la tecla pulsada.
		@return true si el evento ha sido procesado. En este caso 
		el gestor no llamar� a otros listeners.
		*/
		virtual bool keyPressed(Input::TKey key);

		/**
		M�todo que ser� invocado siempre que se termine la pulsaci�n
		de una tecla. Ser� la aplicaci�n qui�n llame a este m�todo 
		cuando el estado est� activo. Esta clase NO se registra en
		el InputManager sino que es la aplicaci�n quien lo hace y 
		delega en los estados.

		@param key C�digo de la tecla pulsada.
		@return true si el evento ha sido procesado. En este caso 
		el gestor no llamar� a otros listeners.
		*/
		virtual bool keyReleased(Input::TKey key);

		// M�todos de CMouseListener

		/**
		M�todo que ser� invocado siempre que se mueva el rat�n. La
		aplicaci�n avisa de este evento al estado actual.

		@param mouseState Estado del rat�n cuando se lanza el evento.
		@return true si el evento ha sido procesado. En este caso 
		el gestor no llamar� a otros listeners.
		*/
		virtual bool mouseMoved(const Input::CMouseState &mouseState);

		/**
		M�todo que ser� invocado siempre que se pulse un bot�n. La
		aplicaci�n avisa de este evento al estado actual.

		@param mouseState Estado del rat�n cuando se lanza el evento.
		@return true si el evento ha sido procesado. En este caso 
		el gestor no llamar� a otros listeners.
		*/
		virtual bool mousePressed(const Input::CMouseState &mouseState);

		/**
		M�todo que ser� invocado siempre que se termine la pulsaci�n
		de un bot�n. La aplicaci�n avisa de este evento al estado 
		actual.

		@param mouseState Estado del rat�n cuando se lanza el evento.
		@return true si el evento ha sido procesado. En este caso 
		el gestor no llamar� a otros listeners. 
		*/
		virtual bool mouseReleased(const Input::CMouseState &mouseState);

		void changeMap(const std::string& mapName) { /* @todo Cambiar el mapa */ }

		void serverSettings(const std::string& serverName, const std::string& serverPassword, unsigned int maxPlayers, 
							unsigned int maxSpectators, bool voteMap, bool voteKick);

	protected:

		inline void disconnect();

		Logic::CGameNetPlayersManager* _playersMgr;

		Net::CManager* _netMgr;

		Logic::CWorldState* _worldState;

		Logic::CMap* _map;

		// Server game settings
		std::string _serverName;
		std::string _serverPassword;
		unsigned int _maxPlayers;
		unsigned int _maxSpectators;
		bool _voteMap;
		bool _voteKick;

		GameMode::Enum _gameMode;

		bool _autoBalanceTeams;

	private:

		void sendMapInfo(Net::NetID playerNetId);

		void sendWorldState(Net::NetID playerNetId);

		void createAndMirrorSpectator(Net::NetID playerNetId);

		void createAndMirrorPlayer(int race, Net::NetID playerNetId, Logic::TeamFaction::Enum team);

		
	}; // CMultiplayerTeamDeathmatchServerState

} // namespace Application

#endif //  __Application_GameState_H
