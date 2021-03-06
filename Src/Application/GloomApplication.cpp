//---------------------------------------------------------------------------
// GaleonApplication.cpp
//---------------------------------------------------------------------------

/**
@file GaleonApplication.cpp

Contiene la implementación de la clase aplicacion para aplicaciones
basadas en Ogre. Esta clase maneja la ejecución de todo el juego.

@see Application::CGaleonApplication
@see Application::C3DApplication

@author David Llansó
@date Septiembre, 2010
*/
#include "GloomApplication.h"

#include "ExitState.h"
#include "MenuState.h"
#include "GameOverState.h"
#include "GameState.h"

#include "NetMenuState.h"
#include "LobbyClientState.h"
#include "LobbyServerState.h"
#include "DMServer.h"
#include "DMClient.h"
#include "TDMClient.h"
#include "TDMServer.h"
#include "SelectScenario.h"
#include "SinglePlayerState.h"

#include "BaseSubsystems/Math.h"

namespace Application {

	CGloomApplication::CGloomApplication() : C3DApplication()
	{

	} // CGloomApplication

	//--------------------------------------------------------

	CGloomApplication::~CGloomApplication()
	{

	} // ~CGloomApplication

	//--------------------------------------------------------

	bool CGloomApplication::init() {
		Math::seed();

		// Inicializamos la clase de la que heredamos.
		if (!C3DApplication::init())
			return false;

		// CREACION DE ESTADOS. 
		// La aplicación se hace responsable de destruirlos.
		if(!addState("gameOver", new CGameOverState(this)))
			return false;
		if(!addState("menu", new CMenuState(this)))
			return false;		
		if(!addState("singlePlayer", new CSinglePlayerState(this)))
			return false;
		if(!addState("exit", new CExitState(this)))
			return false;
		if(!addState("selectScenario", new CSelectScenario(this)))
			return false;

		// ESTADOS DE RED
		if(!addState("netmenu", new CNetMenuState(this)))
			return false;
		if(!addState("lobbyclient", new CLobbyClientState(this)))
			return false;
		if(!addState("lobbyserver", new CLobbyServerState(this)))
			return false;
		if(!addState("DMServer", new CDMServer(this)))
			return false;
		if(!addState("DMClient", new CDMClient(this)))
			return false;
		if(!addState("TDMClient", new CTDMClient(this)))
			return false;
		if(!addState("TDMServer", new CTDMServer(this)))
			return false;
		
		// ESTADO INICIAL
		if(!setState("menu"))
			return false;

		return true;

	} // init

	//--------------------------------------------------------

	void CGloomApplication::release()
	{
		// Desactivamos y eliminamos todos los estados.
		releaseAllStates();

		C3DApplication::release();

	} // release

} // namespace Application
