//---------------------------------------------------------------------------
// 3DApplication.cpp
//---------------------------------------------------------------------------

/**
@file 3DApplication.cpp

Contiene la implementaci�n de la clase aplicacion para aplicaciones
basadas en Ogre. Esta clase maneja la ejecuci�n de todo el juego.

@see Application::C3DApplication
@see Application::CBaseApplication

@author David Llans�
@date Julio, 2010
*/
#include <boost/thread/thread.hpp>

#include "OgreClock.h"
#include "3DApplication.h"

#include "Graphics/Server.h"
#include "BaseSubsystems/Server.h"
#include "Input/InputManager.h"
#include "Input/Server.h"
#include "GUI/Server.h"
#include "Logic/Server.h"
#include "Logic/Maps/ComponentFactory.h"
#include "Physics/Server.h"
#include "net/manager.h"
#include "Audio/Server.h"
#include <cassert>

#include <iostream>

namespace Application {

	C3DApplication::C3DApplication() : CBaseApplication() {
	} // C3DApplication

	//--------------------------------------------------------

	C3DApplication::~C3DApplication() {

	} // ~C3DApplication

	//--------------------------------------------------------

	bool C3DApplication::init() 
	{
		// Inicializamos la clase base.
		if (!CBaseApplication::init())
			return false;

		// Inicializamos los diferentes subsistemas, los de 
		// Ogre entre ellos.
		if (!BaseSubsystems::CServer::Init())
			return false;

		// Inicializamos el servidor gr�fico.
		if (!Graphics::CServer::Init())
			return false;

		// Inicializamos el gestor de entrada de perif�ricos.
		if (!Input::CInputManager::Init())
			return false;

		// Nos registramos como oyentes de los eventos del teclado.
		Input::CInputManager::getSingletonPtr()->addKeyListener(this);

		// Y como oyentes de los eventos del rat�n.
		Input::CInputManager::getSingletonPtr()->addMouseListener(this);

		// Inicializamos el servidor de interfaz con el usuario.
		if (!GUI::CServer::Init())
			return false;

		// Inicializamos el gestor de entrada de perif�ricos.
		if (!Input::CServer::Init())
			return false;

		// Inicializaci�n del servidor de f�sica.
		if (!Physics::CServer::Init())
			return false;

		// Inicializamos el servidor de la l�gica.
		if (!Logic::CServer::Init())
			return false;

		//Iniciamos el audio
		if(!Audio::CServer::Init())
			return false;

		// Inicializamos la red
		if (!Net::CManager::Init())
			return false;

		// Creamos el reloj basado en Ogre.
		_clock = new COgreClock();

		return true;

	} // init

	//--------------------------------------------------------

	void C3DApplication::release() {
		// Eliminamos el reloj de la aplicaci�n.
		delete _clock;

		// Destruimos la factor�a de componentes. La factor�a
		// de componentes no es de construcci�n y destrucci�n expl�cita
		// debido a como se registran los componentes. Por ello Init y
		// Release no son sim�tricos.
		if(Logic::CComponentFactory::getSingletonPtr())
			delete Logic::CComponentFactory::getSingletonPtr();

		if(Logic::CServer::getSingletonPtr())
			Logic::CServer::Release();

		// Liberar los recursos del servidor de f�sica
		if (Physics::CServer::getSingletonPtr())
			Physics::CServer::Release();

		//Liberar los recursos del servidor de audio
		if(Audio::CServer::getSingletonPtr())
			Audio::CServer::Release();

		//liberamos los recursos del servidor de GUI
		if(GUI::CServer::getSingletonPtr())
			GUI::CServer::Release();
		
		if(Input::CServer::getSingletonPtr())
			Input::CServer::Release();

		if(Input::CInputManager::getSingletonPtr()) {
			// Dejamos de ser oyentes de los eventos del teclado.
			Input::CInputManager::getSingletonPtr()->removeKeyListener(this);
			// Y de los eventos del rat�n
			Input::CInputManager::getSingletonPtr()->removeMouseListener(this);
			Input::CInputManager::Release();
		}
		
		if(Graphics::CServer::getSingletonPtr())
			Graphics::CServer::Release();

		if(BaseSubsystems::CServer::getSingletonPtr())
			BaseSubsystems::CServer::Release();

		CBaseApplication::release();

	} // release

	//--------------------------------------------------------

	void C3DApplication::tick(unsigned int msecs) {
		// TICK DE RED
		//boost::thread t(&Net::CManager::tick, Net::CManager::getSingletonPtr(), msecs);
		Net::CManager::getSingletonPtr()->tick(msecs);
		// TICK DE INPUT
		//boost::thread y(&Input::CInputManager::tick, Input::CInputManager::getSingletonPtr() , msecs);

		Input::CInputManager::getSingletonPtr()->tick(msecs);

		// TICK DE LOGICA-FISICA
		CBaseApplication::tick(msecs);

		// TICK DE AUDIO
		Audio::CServer::getSingletonPtr()->tick(msecs);
		//boost::thread audio( &Audio::CServer::tick, Audio::CServer::getSingletonPtr(), msecs);

		// TICK DEL GUI
		GUI::CServer::getSingletonPtr()->tick();

		// TICK DE GR�FICOS
		Graphics::CServer::getSingletonPtr()->tick(msecs/1000.0f);
		
		//audio.join();
	} // tick

} // namespace Application
