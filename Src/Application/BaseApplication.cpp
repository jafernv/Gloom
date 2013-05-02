//---------------------------------------------------------------------------
// BaseApplication.cpp
//---------------------------------------------------------------------------

/**
@file BaseApplication.cpp

Contiene la implementaci�n de la clase aplicacion, que maneja la ejecuci�n
de todo el juego.

@see Application::CBaseApplication
@see Application::CApplicationState

@author Marco Antonio G�mez Mart�n & David Llans�
@date Julio, 2010
*/

#include "BaseApplication.h"
#include "ApplicationState.h"
#include "Clock.h"
#include <iostream>

#include <assert.h>

namespace Application {

	CBaseApplication *CBaseApplication::_instance = 0;

	CBaseApplication::CBaseApplication() : 
		_initialized(false),
		_currentState(0),
		_nextState(0),
		_exit(false),
		_clock(0)
	{
		assert(!_instance && "No puede crearse m�s de una aplicaci�n");

		_instance = this;

	} // CBaseApplication

	//--------------------------------------------------------

	CBaseApplication::~CBaseApplication()
	{
		_instance = 0;

	} // ~CBaseApplication

	//--------------------------------------------------------

	bool CBaseApplication::init() 
	{
		assert(!_initialized && "La aplicaci�n ya est� inicializada");

		_initialized = true;
		playing=false;
		return true;

	} // init

	//--------------------------------------------------------

	void CBaseApplication::release()
	{
		assert(_initialized && "La aplicaci�n no est� inicializada");

		// Desactivamos y eliminamos todos los estados.
		releaseAllStates();

		_initialized = false;

	} // release

	//--------------------------------------------------------

	void CBaseApplication::releaseAllStates()
	{
		// Desactivamos el estado actual
		if (_currentState)
		{
			_currentState->deactivate();
			_currentState = 0;
		}

		// Eliminamos los estados

		TStateTable::const_iterator it, end;

		for (it = _states.begin(), end = _states.end(); 
			 it != end; ++it) 
		{
			it->second->release();
			delete it->second;
		}
		_states.clear();

	} // releaseAllStates

	//--------------------------------------------------------

	bool CBaseApplication::addState(const std::string &name,
					   CApplicationState *newState) 
	{
		TStateTable::const_iterator it;

#ifdef _DEBUG
		// Comprobamos que no existe un estado con ese nombre.
		// Otra posibilidad es no hacerlo en Debug, sino siempre,
		// y, en caso de que ya exista, eliminarlo (pues la aplicaci�n
		// acepta la responsabilidad de borrar los estados que contiene).
		// Sin embargo, en ese caso, habr�a que comprobar que no es
		// el estado actual, ni el estado siguiente al que se espera ir...
		it = _states.find(name);
		assert(it == _states.end());
#endif
		_states[name] = newState;
		return newState->init();

	} // addState

	//--------------------------------------------------------

	bool CBaseApplication::setState(const std::string &name) 
	{
		// Buscamos el estado.
		TStateTable::const_iterator it;
		it = _states.find(name);

		// Si no hay ning�n estado con ese nombre, no hacemos nada
		if (it == _states.end())
			return false;

		_nextState = it->second;

		

		return true;

	} // setState

	//--------------------------------------------------------

	void CBaseApplication::run() 
	{
		assert(_clock && "Asegurate de haber creado un reloj en el init de la clase de tu aplicacion!");
		unsigned int resto;
		unsigned int multiplo;
		unsigned int fixedTick;

		// Actualizamos una primera vez el tiempo, antes de
		// empezar, para que el primer frame tenga un tiempo
		// de frame razonable.
		_clock->updateTime();
		// Ejecuci�n del bucle principal. Simplemente miramos si
		// tenemos que hacer una transici�n de estado, y si no hay que
		// hacerla, ejecutamos la vuelta
		_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
		//_CrtSetBreakAlloc(598913);
		while (!exitRequested()) 
		{
			if (!_currentState ||
					(_nextState && (_currentState != _nextState))){
				changeState();
				_clock->updateTime();
				continue;
			}

			_clock->updateTime();

			//Como m�nimo 16 milisegundos para ejecutar un tick desde la ultima vez, el resto me lo guardo
			//Salvo que sea multiplo en cuyo caso ejecuto el tick entero (Ej; 32 del tiron y no guardo nada)
			_clock->setAcumTime(_clock->getAcumTime()+_clock->getLastFrameDuration());		
			if(_clock->getAcumTime()>=16000){
				multiplo=_clock->getAcumTime()/16000;
				fixedTick=16*multiplo;
				tick(fixedTick);
				resto=_clock->getAcumTime()-fixedTick*1000;
				_clock->setAcumTime(resto);
			}
		
		}
		//_CrtDumpMemoryLeaks();
		
	} // run

	//--------------------------------------------------------

	unsigned int CBaseApplication::getAppTime() 
	{
		return _clock->getTime();

	} // getAppTime

	//--------------------------------------------------------

	void CBaseApplication::changeState() 
	{
		// Avisamos al estado actual que le vamos a eliminar
		if (_currentState)
			_currentState->deactivate();

		assert(_nextState);
		_nextState->activate();
		_currentState = _nextState;
		
		//std::cout << "frames: " << _frames << std::endl;
		//std::cout << "tiempo de procesado gr�fico: " << _timegraphics << std::endl;

		_timerAudio=0;
		_timegraphics=0;
		_frames=0;

	} // changeState

	//--------------------------------------------------------

	void CBaseApplication::tick(unsigned int msecs) 
	{
		// Aparentemente esta funci�n es sencilla. Aqu� se pueden
		// a�adir otras llamadas que sean comunes a todos los estados
		// de todas las aplicaciones.
		// El m�todo es virtual. Si para una aplicaci�n concreta, se
		// identifican cosas comunes a todos los estados, se pueden
		// a�adir en la implementaci�n del m�todo de esa aplicaci�n.

		if (_currentState)
			_currentState->tick(msecs);

	} // tick

	//--------------------------------------------------------

	bool CBaseApplication::keyPressed(Input::TKey key)
	{
		// Avisamos al estado actual de la pulsaci�n.
		if (_currentState)
			return _currentState->keyPressed(key);
		
		return false;

	} // keyPressed

	//--------------------------------------------------------

	bool CBaseApplication::keyReleased(Input::TKey key)
	{
		// Avisamos al estado actual del fin de la pulsaci�n.
		if (_currentState)
			return _currentState->keyReleased(key);
		
		return false;

	} // keyReleased

	//--------------------------------------------------------
	
	bool CBaseApplication::mouseMoved(const Input::CMouseState &mouseState)
	{
		// Avisamos al estado actual del movimiento.
		if (_currentState)
			return _currentState->mouseMoved(mouseState);
		
		return false;

	} // mouseMoved

	//--------------------------------------------------------
		
	bool CBaseApplication::mousePressed(const Input::CMouseState &mouseState)
	{
		// Avisamos al estado actual de la pulsaci�n.
		if (_currentState)
			return _currentState->mousePressed(mouseState);
		
		return false;

	} // mousePressed

	//--------------------------------------------------------


	bool CBaseApplication::mouseReleased(const Input::CMouseState &mouseState)
	{
		// Avisamos al estado actual del fin de la pulsaci�n.
		if (_currentState)
			return _currentState->mouseReleased(mouseState);
		
		return false;

	} // mouseReleased

} // namespace Application
