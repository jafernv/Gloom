//---------------------------------------------------------------------------
// MenuState.h
//---------------------------------------------------------------------------

/**
@file MenuState.h

Contiene la declaraci�n del estado de men�.

@see Application::CApplicationState
@see Application::CMenuState

@author David Llans�
@date Agosto, 2010
*/

#ifndef __Application_NetMenuState_H
#define __Application_NetMenuState_H

#include "ApplicationState.h"

#include "Hikari.h"

// Predeclaraci�n de clases para ahorrar tiempo de compilaci�n
namespace Application 
{
	class CBaseApplication;
}

namespace Application 
{
	/**
	Como su nombre indica, esta clase es la clase del men�
	principal del juego. Es muy sencilla y lo �nico que hace es cargar
	un layout de Hikari al inicio y activarlo y desactivarlo cuando
	se activa o desactiva el estado (haci�ndo visible/invisible tambi�n
	el puntero del rat�n). Tambi�n asocia los eventos de los botones 
	del men� a las funciones C++ que se deben invocar cuando los botones
	son pulsados.
	<p>
	// HACK Arreglar
	Este estado es Hikari dependiente, lo cual no es deseable, la aplicaci�n
	deber�a ser independiente de las tecnolog�as usadas.

	@ingroup applicationGroup

	@author David Llans�
	@date Agosto, 2010
	*/
	class CNetMenuState : public CApplicationState 
	{
	public:
		/** 
		Constructor de la clase 
		*/
		CNetMenuState(CBaseApplication *app) : CApplicationState(app)
				{}

		/** 
		Destructor 
		*/
		virtual ~CNetMenuState();

		/**
		Funci�n llamada cuando se crea el estado (se "engancha" en la
		aplicaci�n, para que inicialice sus elementos.

		@return true si todo fue bien.
		*/
		virtual bool init();

		/**
		Funci�n llamada cuando se elimina ("desengancha") el
		estado de la aplicaci�n.
		*/
		virtual void release();

		/**
		Funci�n llamada por la aplicaci�n cuando se activa
		el estado.
		*/
		virtual void activate();

		/**
		Funci�n llamada por la aplicaci�n cuando se desactiva
		el estado.
		*/
		virtual void deactivate();

		/**
		Funci�n llamada por la aplicaci�n para que se ejecute
		la funcionalidad del estado.

		@param msecs N�mero de milisegundos transcurridos desde
		la �ltima llamada (o desde la �ctivaci�n del estado, en caso
		de ser la primera vez...).
		*/
		virtual void tick(unsigned int msecs);

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

	private:
		/**
		Ventana que muestra el menu
		*/
		Hikari::FlashControl* _menu;
		
		/**
		Funci�n que se quiere realizar cuando se pulse el bot�n server.
		Simplemente cambia al estado de lobby server.
		*/
		Hikari::FlashValue serverReleased(Hikari::FlashControl* caller, const Hikari::Arguments& args);

		/**
		Funci�n que se quiere realizar cuando se pulse el bot�n client.
		Simplemente cambia al estado de lobby client.
		*/
		Hikari::FlashValue clientReleased(Hikari::FlashControl* caller, const Hikari::Arguments& args);

		/**
		Funci�n que se quiere realizar cuando se pulse el bot�n back.
		Simplemente cambia al estado de menu.
		*/
		Hikari::FlashValue backReleased(Hikari::FlashControl* caller, const Hikari::Arguments& args);

	}; // CMenuState

} // namespace Application

#endif //  __Application_MenuState_H
