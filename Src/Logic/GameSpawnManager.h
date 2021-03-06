//---------------------------------------------------------------------------
// GameSpawnManager.h
//---------------------------------------------------------------------------

/**
@file GameSpawnManager.h

Contiene la declaraci�n del gestor de los mensajes de red durante la partida.

@see Logic::CGameSpawnManager

@author Jose Antonio Garc�a Y��ez
@date Febrero, 2013
*/

#ifndef __GameSpawnManager_H
#define __GameSpawnManager_H

#include "Entity\Entity.h"

namespace Logic 
{
	/**
	Este m�dulo es un singleton que se usa como manager de los puntos de spawn.

	@author Jose Antonio Garc�a Y��ez
	@date Febrero, 2013
	*/

	class CGameSpawnManager 
	{
	public:

		/**
		Devuelve la �nica instancia de la clase CGameSpawnManager.
		
		@return �nica instancia de la clase CGameSpawnManager.
		*/
		static CGameSpawnManager* getSingletonPtr() {return _instance;}

		/**
		Inicializa la instancia

		@return Devuelve false si no se ha podido inicializar.
		*/
		static bool Init();

		/**
		Libera la instancia de CGameSpawnManager. Debe llamarse al finalizar la 
		aplicaci�n.
		*/
		static void Release();

		/**
		Funci�n llamada para activar la escucha.
		*/
		void activate();

		/**
		Funci�n llamada desactivar la escucha.
		*/
		void deactivate();


		/**
		Devuelve una entidad de spawn si la encuentre en un m�ximo de _maxTrys sino NULL.
		*/
		CEntity* getSpawnPosition();

	protected:
		
		/** 
		Constructor de la clase 
		*/
		CGameSpawnManager();

		/** 
		Destructor 
		*/
		~CGameSpawnManager();

		/**
		Lista de puntos
		*/
		std::vector<CEntity*> _listSpawnPoints;

	private:
		/**
		�nica instancia de la clase.
		*/
		static CGameSpawnManager* _instance;

		/**
		N�mero de intentos para encontrar un punto de spawn.
		*/
		int _maxTrys;

	}; // CGameSpawngManager

} // namespace Logic

#endif //  __GameSpawnManager_H
