//---------------------------------------------------------------------------
// GameState.cpp
//---------------------------------------------------------------------------

/**
@file GameState.cpp

Contiene la implementaci�n del estado de juego.

@see Application::CApplicationState
@see Application::CGameState

@author David Llans�
@date Agosto, 2010
*/

#include "GameState.h"
#include "OgreClock.h"

#include "Logic/Server.h"
#include "Logic/Maps/EntityFactory.h"
#include "Logic/Maps/Map.h"

#include "Input/Server.h"
#include "Input\PlayerController.h"

#include "Physics/Server.h"
#include "Audio\Server.h"

namespace Application {

	bool CGameState::init() 
	{
		// Ejecutamos la inicializaci�n de la clase padre
		// En este caso no hace nada, solo retorna true
		CApplicationState::init();

		_time = 0;
		_timelogic=0;
		_timephysics=0;
		return true;
	} // init

	//--------------------------------------------------------

	void CGameState::release() 
	{
		// Liberar los recursos reservados para la escena y las
		// clases construidas a partir de los blueprints
		Logic::CServer::getSingletonPtr()->unLoadLevel();
		Logic::CEntityFactory::getSingletonPtr()->unloadBluePrints();
		// Liberar la escena f�sica usando el motor de f�sica
		Physics::CServer::getSingletonPtr()->destroyScene();

		// Llamar al m�todo padre por si acaso tiene que hacer algo
		CApplicationState::release();

	} // release

	//--------------------------------------------------------

	void CGameState::activate() 
	{
		CApplicationState::activate();
		
		// Activamos el mapa que ha sido cargado para la partida.
		Logic::CServer::getSingletonPtr()->activateMap();

		// Queremos que el GUI maneje al jugador.
		Input::CServer::getSingletonPtr()->getPlayerController()->activate();
		Logic::CEntityFactory::getSingletonPtr()->dynamicCreation(true);

		//paramos el sonido de menu
		Audio::CServer::getSingletonPtr()->stopSound("theme");
	} // activate

	//--------------------------------------------------------

	void CGameState::deactivate() 
	{
		std::cout << "tiempo de procesado fisico: " << _timephysics << std::endl;
		std::cout << "tiempo de procesado logico: " << _timelogic << std::endl;
		std::cout << "tiempo total jugado: " << _time << std::endl;
		// Desactivamos la clase que procesa eventos de entrada para 
		// controlar al jugador.
		Input::CServer::getSingletonPtr()->getPlayerController()->deactivate();

		Logic::CServer::getSingletonPtr()->unLoadLevel();
		
		CApplicationState::deactivate();

		Logic::CEntityFactory::getSingletonPtr()->dynamicCreation(false);
	} // deactivate

	//--------------------------------------------------------

	void CGameState::tick(unsigned int msecs) 
	{
		unsigned int time = clock();
		// TODO: realizar la simulaci�n f�sica
		Physics::CServer::getSingletonPtr()->tick(msecs);
		//tiempo que ha tardado la fisica
		time = clock()-time;
		_timephysics+=time;
		
		time = clock();

		// Actualizamos la l�gica de juego.
		Logic::CServer::getSingletonPtr()->tick(msecs);
		//tiempo que ha tardado la logica
		time = clock()-time;
		_timelogic+=time;
		_time += msecs;

	} // tick

	//--------------------------------------------------------

	bool CGameState::keyPressed(Input::TKey key)
	{
		return false;

	} // keyPressed

	//--------------------------------------------------------

	bool CGameState::keyReleased(Input::TKey key)
	{
		switch(key.keyId)
		{
		case Input::Key::ESCAPE:
			
			//Logic::CServer::getSingletonPtr()->unLoadLevel();
/*
			Logic::CServer::getSingletonPtr()->unLoadLevel();
		Logic::CEntityFactory::getSingletonPtr()->unloadBluePrints();
		// Liberar la escena f�sica usando el motor de f�sica
		Physics::CServer::getSingletonPtr()->destroyScene();

		// Llamar al m�todo padre por si acaso tiene que hacer algo
		CApplicationState::release();

		CApplicationState::init();
		*/
			_app->setState("menu");
			break;
		default:
			return false;
		}
		return true;

	} // keyReleased

	//--------------------------------------------------------
	
	bool CGameState::mouseMoved(const Input::CMouseState &mouseState)
	{
		return false;

	} // mouseMoved

	//--------------------------------------------------------
		
	bool CGameState::mousePressed(const Input::CMouseState &mouseState)
	{
		return false;

	} // mousePressed

	//--------------------------------------------------------


	bool CGameState::mouseReleased(const Input::CMouseState &mouseState)
	{
		return false;

	} // mouseReleased

} // namespace Application
