//---------------------------------------------------------------------------
// PlayerInfo.h
//---------------------------------------------------------------------------

/**
@file PlayerInfo.h

Contiene la declaracion de la clase PlayerInfo para el proyecto de logica.

@see Logic::CPlayerInfo

@author Francisco Aisa Garc�a
@date Febrero, 2013
*/

#ifndef __Logic_PlayerInfo_H
#define __Logic_PlayerInfo_H

#include <string>
#include <set>
#include "Logic/Maps/TeamFaction.h"

// Predeclaracion del typedef NetID
namespace Net {
	typedef unsigned int NetID;
};

namespace Logic {

	// Predeclaracion del typedef TEntityID
	typedef unsigned int TEntityID;

	/**
	Informaci�n sobre los clientes. El manager de red debe mantener una
	lista de clientes conectados con informaci�n asociada a ellos. Esta
	clase sirve de abstracci�n para introducir todos aquellos detalles
	de los clientes que consideremos relevantes.

	@ingroup LogicGroup

	@author Francisco Aisa Garc�a
	@date Febrero, 2013
	*/

	class CPlayerInfo {
	public:

		// =======================================================================
		//                      CONSTRUCTORES Y DESTRUCTOR
		// =======================================================================


		/** Constructor por defecto. */
		CPlayerInfo() : _entityIdInitialized(false), _entityId(0) {}
		
		//________________________________________________________________________

		/**
		Constructor por parametro
		
		@param netId Identificador de red del player que se va a crear.
		*/
		CPlayerInfo(Net::NetID netId);

		//________________________________________________________________________

		/**
		Constructor por parametro
		
		@param netId Identificador de red del player que se va a crear.
		@param nickname Nickname del player que se va a crear.
		*/
		CPlayerInfo(Net::NetID netId, const std::string& nickname);
		
		//________________________________________________________________________

		/** 
		Constructor de copias.

		@param rhs objeto que se a partir del cual vamos a inicializar el nuestro.
		*/
		CPlayerInfo(const CPlayerInfo& rhs);
		
		//________________________________________________________________________

		/** Destructor */
		~CPlayerInfo() { /* No hay memoria dinamica que liberar */ };


		// =======================================================================
		//                              OPERADORES
		// =======================================================================


		/**
		Operador de asignacion.

		@param Objeto que vamos a asignar a nuestra clase.
		@return Referencia a nuestro objeto.
		*/
		CPlayerInfo& operator=(const CPlayerInfo& rhs);

		//________________________________________________________________________
		
		/**
		Operador de flujo de salida.

		@param out Flujo de salida.
		@param playerInfo Informacion del player.
		@return Referencia al flujo de salida modificado.
		*/
		friend std::ostream& operator<<(std::ostream& out, const CPlayerInfo& playerInfo);


		// =======================================================================
		//                               GETTERS
		// =======================================================================


		/**
		Devuelve el nickname del player.

		@return El nickname del player.
		*/
		std::string getName();

		//________________________________________________________________________

		/**
		Devuelve el nombre del equipo al que pertenece el player.

		@return El equipo al que pertenece el jugador.
		*/
		TeamFaction::Enum getTeam();

		//________________________________________________________________________

		/**
		Devuelve el n�mero de muertes que le han hecho al jugador.

		@return Devuelve el n�mero de muertes que se le han hecho al jugador.
		*/
		inline unsigned int getDeaths() { return _deaths; }

		//________________________________________________________________________

		/**
		Devuelve la clase que esta usando el player.

		@return La clase del player.
		*/
		std::string getPlayerClass();

		//________________________________________________________________________

		/**
		Devuelve el identificador de la entidad logica que corresponde al player que
		describe este CPlayerInfo.

		@return El identificador de la entidad y un booleano que indica si el id de
		entidad ha sido inicializado.
		*/
		std::pair<TEntityID, bool> getEntityId();

		//________________________________________________________________________

		/**
		Devuelve el identificador de red de la entidad que corresponde al player que
		describe este CPlayerInfo.

		@return El identificador de red de la entidad.
		*/
		Net::NetID getNetId();

		//________________________________________________________________________

		/**
		Devuelve la cantidad de clientes que este player ha cargado.

		@return Numero de clientes que este player ha cargado.
		*/
		unsigned int playersLoaded();

		//________________________________________________________________________

		/**
		Indica si el player ha sido spawneado en la partida online o no.

		@return True si la entidad ha sido spawneada.
		*/
		bool isSpawned();

		//________________________________________________________________________

		/**
		Devuelve el n�mero de frags que lleva el jugador.

		@return N�mero de frags que lleva el jugador.
		*/
		inline int getFrags() { return _frags; }

		// =======================================================================
		//                               SETTERS
		// =======================================================================


		/**
		Asigna un nickname al player.

		@param name Nombre que se le desea asignar al player.
		*/
		void setName(const std::string& name);

		//________________________________________________________________________

		/**
		Asigna un equipo al jugador.

		@param team Equipo al que pertenece el jugador
		*/
		void setTeam(TeamFaction::Enum team);

		//________________________________________________________________________

		/**
		Asigna un identificador de entidad al player.

		@param entityId Identificador de entidad que se le va a asignar al player.
		*/
		void setEntityId(TEntityID entityId);

		//________________________________________________________________________

		/**
		Asigna un identificador de red al player.

		@param Identificador de red que se le va a asignar al player.
		*/
		void setNetId(Net::NetID netId);

		//________________________________________________________________________

		/**
		Asigna el numero de frags dados al player.

		@param frags Numero de frags que queremos asignar al player.
		*/
		void setFrags(int frags);

		//________________________________________________________________________

		/**
		Asigna el numero de muertes dadas al player.

		@param deaths Numero de muertes que queremos asignar al player.
		*/
		void setDeaths(int deaths);

		//________________________________________________________________________
		
		/**
		Setea el estado del jugador, si jugando o espectando/conectandose.

		@param playing True si el jugador esta spawneado.
		*/
		void isSpawned(bool playing);

		//________________________________________________________________________

		/**
		Incrementa el n�mero de frags anotados para el jugador dado.
		*/
		inline unsigned int addFrag() { 
			++_currentSpree;
			if(_currentSpree > _bestSpree)
				_bestSpree = _currentSpree;

			return ++_frags; 
		}

		//________________________________________________________________________

		/**
		Decrementa el n�mero de frags anotados para el jugador dado.
		*/
		inline void substractFrag() { --_frags; }

		inline unsigned int addDeath() { 
			_currentSpree = 0;

			return ++_deaths; 
		}

		inline void subDeath() { --_deaths; }

		inline unsigned int getBestSpree() { return _bestSpree; }


	private:


		// =======================================================================
		//                          MIEMBROS PRIVADOS
		// =======================================================================


		/** Nickname del player. */
		std::string _name;

		/** Frags que el jugador lleva. */ 
		int _frags;

		unsigned int _deaths;

		unsigned int _bestSpree;

		unsigned int _currentSpree;

		unsigned int _ping;

		std::string _race;

		/** Equipo al que pertenece el jugador. */
		TeamFaction::Enum _team;

		/** Identificador de la entidad logica */
		TEntityID _entityId;

		/** True si el identificador de entidad ha sido inicializado */
		bool _entityIdInitialized;

		/** Identificador de la entidad de red */
		Net::NetID _netId;

		/** 
		True si el jugador esta dentro de la partida, false si se esta conectando
		o est� en modo espectador. */
		bool _isPlaying;
	};

};

#endif