//---------------------------------------------------------------------------
// GameSpawnManager.cpp
//---------------------------------------------------------------------------

/**
@file GameSpawnManager.cpp

Contiene la implementaci�n del gestor del spawn/respawn de jugadores.

@see Logic::CGameSpawnManager

@author Jose Antonio Garc�a Y��ez
@date Febrero, 2013
*/

#include "GameSpawnManager.h"
#include "Logic\Entity\Components\PhysicStaticEntity.h"

#include "Server.h"
#include "Logic/Maps/Map.h"

#include <cassert>


namespace Logic {

	CGameSpawnManager* CGameSpawnManager::_instance = 0;

	//--------------------------------------------------------

	CGameSpawnManager::CGameSpawnManager()
	{
		_instance = this;

	} // CServer

	//--------------------------------------------------------

	CGameSpawnManager::~CGameSpawnManager()
	{
		_instance = 0;

	} // ~CServer
	
	//--------------------------------------------------------

	bool CGameSpawnManager::Init()
	{
		assert(!_instance && "Segunda inicializaci�n de Logic::CGameNetMsgManager no permitida!");

		new CGameSpawnManager();

		return true;

	} // Init

	//--------------------------------------------------------

	void CGameSpawnManager::Release()
	{
		assert(_instance && "Logic::CGameSpawnManager no est� inicializado!");

		if(_instance)
			delete _instance;

	} // Release

	//--------------------------------------------------------

	void CGameSpawnManager::activate() 
	{
		//Cargamos los las entidades de tipo SpawnPoint para tenerlas
		CEntity *entidad;
		entidad=CServer::getSingletonPtr()->getMap()->getEntityByType("SpawnPoint");
		if(entidad!=NULL)
			_listSpawnPoints.push_back(entidad);
		while(entidad!=NULL){
			entidad=CServer::getSingletonPtr()->getMap()->getEntityByType("SpawnPoint",entidad);
			if(entidad!=NULL)
				_listSpawnPoints.push_back(entidad);
		}
		_maxTrys=30;
	} // activate

	//--------------------------------------------------------

	void CGameSpawnManager::deactivate() 
	{	
		_listSpawnPoints.clear();
	} // deactivate

	//---------------------------------------------------------
	
	CEntity* CGameSpawnManager::getSpawnPosition(){
		/*int disponibles=0;
		for(int i =0;i<_listSpawnPoints.size();i++)
			if(!_listSpawnPoints[i]->getComponent<CPhysicStaticEntity>("CPhysicStaticEntity")->getInTrigger())
				disponibles++;*/
		int random=(rand()*clock())%_listSpawnPoints.size();
		//Mientras que nos devuelva que el trigger esta activado buscamos otro punto
		int intentos=0;
		while(_listSpawnPoints[random]->getComponent<CPhysicStaticEntity>("CPhysicStaticEntity")->getInTrigger()){
			std::cout << "try: " << intentos+1 << ". Random: " << random << std::endl;
			random=(rand()*clock())%_listSpawnPoints.size();
			std::cout << "clock vale " << clock() << std::endl;
			intentos++;
			if(intentos>_maxTrys){
				_listSpawnPoints[random]->getComponent<CPhysicStaticEntity>("CPhysicStaticEntity")->setInTrigger(true);
				return _listSpawnPoints[random];
			}
		}
		//Ademas por si acaso se pide mas de un punto en el mismo tick hay que marcarlo instantaneamente
		_listSpawnPoints[random]->getComponent<CPhysicStaticEntity>("CPhysicStaticEntity")->setInTrigger(true);
		return _listSpawnPoints[random];
	}

} // namespace Logic
