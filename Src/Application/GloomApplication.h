//---------------------------------------------------------------------------
// GaleonApplication.h
//---------------------------------------------------------------------------

/**
@file GaleonApplication.h

Contiene la declaraci�n de la clase aplicacion para aplicaciones
basadas en Ogre. Esta clase maneja la ejecuci�n de todo el juego.

@see Application::CGaleonApplication
@see Application::C3DApplication

@author David Llans�
@date Septiembre, 2010
 */

#ifndef __Application_GloomApplication_H
#define __Application_GloomApplication_H

#include "3DApplication.h"
#include "Input/InputManager.h"

// Predeclaraci�n de clases para ahorrar tiempo de compilaci�n
namespace Application 
{
  class CApplicationState;
}
namespace Graphics 
{
  class CServer;
}
namespace Ogre 
{
  class Timer;
}

namespace Application 
{
	/**
	Clase CGloomApplication, que extiende C3DApplication para a�adir
	los estados concretos que se desean crear en la partida.

	@ingroup applicationGroup

	@author David Llans�
	@date Septiembre, 2010
	*/
	class CGloomApplication : public C3DApplication
	{
	public:
		/**
		Constructor de la clase
		*/
		CGloomApplication();

		/**
		Destructor de la aplicaci�n
		*/
		virtual ~CGloomApplication();

		/**
		Inicializa la aplicaci�n a�adiendo los estados concretos.

		@return false si la inicializaci�n ha fallado.
		*/
		virtual bool init();

		/**
		Finaliza la aplicaci�n, liberando todos los recursos
		utilizados y destruyendo los estados creados.
		*/
		virtual void release();

	}; // class GloomApplication

} // namespace Application

#endif // __Application_GloomApplication_H
