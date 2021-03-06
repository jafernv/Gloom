//---------------------------------------------------------------------------
// CompositorListener.h
//---------------------------------------------------------------------------

/**
@file CompositorListener.h

Contiene la implementacion del compositor listener, este se usa para pasar parametros a los compositor (y los shader aplicados a estos)

@author Antonio Jesus Narvaez
@date Abril, 2013
*/

#ifndef __Graphics_CompositorListener_H
#define __Graphics_CompositorListener_H

#include <OgreCompositorInstance.h>

// Predeclaraci�n de clases para ahorrar tiempo de compilaci�n
namespace Ogre 
{
	class CompositorManager;
	class CompositorInstance;
	class Listener;
}
namespace Graphics 
{
	class CScene;
	class COverlay;
}

/**
Namespace con todo lo que se refiere a la parte gr�fica. Las diferentes clases
se apoyan en Ogre.
(para m�s informaci�n ver @ref graphicsGroup).

@author Antonio Jesus Narvaez
@date Abril, 2013
*/
namespace Graphics 
{
	/**
	Clase para administrar las variables pasadas a los compositors.

	@author Antonio Jesus Narvaez
	@date Abril, 2013
	*/
	class CCompositorListener:public Ogre::CompositorInstance::Listener
	{
	public:

		CCompositorListener();

		~CCompositorListener();

		void deleteCompositor(const std::string &nameCompositor);

		void inputCompositor(const std::string &nameVar, float value);

		void notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);

		void notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);

	protected:
		/**
		M�todo que setea las variables modificadas al material del compositor

		OJO, no soporta multipasada, hay que pensar si es necesario para el futuro
		*/
		void necesaryOperations(int pass_id, Ogre::MaterialPtr &mat);
		/**
		tipo para facilitad el par
		*/
		typedef std::map<std::string, float> TVariables;

		/**
		mapa para guardar por cada nombre su identificador
		*/
		std::map<std::string, int> _nameCompositorAndId;

		/**
		multimapa con las variables que tendra un compositor, es un multimapa por que puede tener varias variables
		*/
		TVariables _compositorVariables;

		//std::map < int, TkeyValue > _compositorsAndVariables;

		//std::map<int, std::map<std::string, int > >  __compositorsAndVariables;


		Ogre::CompositorManager *_compositorManager;
		Ogre::Viewport *_cameraViewport;

	};

} // namespace Graphics

#endif // __Graphics_Server_H
