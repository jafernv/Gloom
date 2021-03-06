/**
@file Graphics.h

Contiene la declaraci�n del componente que controla la representaci�n
gr�fica de la entidad.

@see Logic::CGraphics
@see Logic::IComponent

@author David Llans�
@date Agosto, 2010
*/
#ifndef __Logic_Graphics_H
#define __Logic_Graphics_H

#include "Logic/Entity/Component.h"
#include "Graphics/Entity.h"

#include "Graphics/Scene.h"

#include "OgreSceneManager.h"


// Predeclaraci�n de clases para ahorrar tiempo de compilaci�n
namespace Graphics 
{
	class CEntity;
	class CGlowingEntity;
	class CScene;

	class SceneManager;
}

//declaraci�n de la clase
namespace Logic 
{
/**
	Componente que se encarga de la representaci�n gr�fica de una entidad.
	En concreto se encarga de las entidades con representaciones gr�ficas
	no animadas. Para otros tipos de representaciones hay otros componentes
	que especializan �ste (CAnimatedGraphics y CStaticGraphics).
	<p>
	Acepta mensajes de cambio de posici�n y orientaci�n (matriz de 
	transformaci�n) mediante el mensaje SET_TRANSFORM.
	
    @ingroup logicGroup

	@author David Llans� Garc�a
	@date Agosto, 2010
*/
	class CGraphics : public IComponent
	{
		DEC_FACTORY(CGraphics);
	public:

		/**
		Constructor por defecto; inicializa los atributos a su valor por 
		defecto.
		*/
		CGraphics() : _graphicsEntity(NULL), _primerDecal(true), _iMaxDecals(50), _iContadorDecals(0) {
		}

		/**
		Destructor (virtual); Quita de la escena y destruye la entidad gr�fica.
		*/
		virtual ~CGraphics();
		
		/**
		Inicializaci�n del componente, utilizando la informaci�n extra�da de
		la entidad le�da del mapa (Maps::CEntity). Toma del mapa el atributo
		model con el modelo que se deber� cargar e invoca al m�todo virtual
		createGraphicsEntity() para generar la entidad gr�fica.

		@param entity Entidad a la que pertenece el componente.
		@param map Mapa L�gico en el que se registrar� el objeto.
		@param entityInfo Informaci�n de construcci�n del objeto le�do del
			fichero de disco.
		@return Cierto si la inicializaci�n ha sido satisfactoria.
		*/
		virtual bool spawn(CEntity* entity, CMap *map, const Map::CEntity *entityInfo);

		/**
		M�todo virtual que elige que mensajes son aceptados. Son v�lidos
		SET_TRANSFORM.

		@param message Mensaje a chequear.
		@return true si el mensaje es aceptado.
		*/
		virtual bool accept(const std::shared_ptr<CMessage>& message);

		/**
		M�todo que activa el componente; invocado cuando se activa
		el mapa donde est� la entidad a la que pertenece el componente.
		<p>
		Si el componente pertenece a la entidad del jugador, el componente
		se registra as� mismo en el controlador del GUI para que las ordenes 
		se reciban a partir de los eventos de teclado y rat�n.

		@return true si todo ha ido correctamente.
		*/
		virtual void onActivate();

		/**
		M�todo virtual que procesa un mensaje.

		@param message Mensaje a procesar.
		*/
		virtual void process(const std::shared_ptr<CMessage>& message);

		void setTransform(const Vector3 &position, const Ogre::Quaternion &orientation);

		void setVisible(bool renderGraphicEntity);

		virtual void changeMaterial(const std::string& materialName);

		virtual void changeMaterial(const std::list<std::string>& materialList);

		virtual void onTick(unsigned int msecs);

		std::string getMeshName(){return _model;}

		/**
		M�todo que se encarga de cambiar la escala de la entidad gr�fica.

		@param newScale escala nueva.
		*/
		void changeScale(float newScale);


		void					setPosition			(Vector3 vPos);

		Graphics::CEntity* getGraphicEntity() { return _graphicsEntity; }


		Ogre::Entity*			getOgreMesh			()		{return _graphicsEntity->getEntity(); }

		Ogre::SceneNode*		getOgreSceneNode	()		{return _graphicsEntity->getSceneNode(); }

		Ogre::SceneManager*		getSceneManager		()		{return _graphicsEntity->getScene()->getSceneMgr(); }

		void					drawDecal			(Vector3 vPos, std::string vTexture, bool bRandomSize);


	protected:

		virtual void onDeactivate();

		virtual void onStart();

		/**
		M�todo virtual que construye la entidad gr�fica de la entidad. Otros
		componentes pueden sobreescribirlo para inicializar otro tipo de
		entidades gr�ficas (animadas, etc.).
		
		@param entityInfo Informaci�n de construcci�n del objeto le�do del
			fichero de disco.
		@return Entidad gr�fica creada, NULL si hubo alg�n problema.
		*/
		virtual Graphics::CEntity* createGraphicsEntity(const Map::CEntity *entityInfo);

		unsigned int readLightMask(const Map::CEntity *entityInfo);

		/**
		Atributo con el nombre del modelo gr�fico de la entidad.
		*/
		std::string _model;

		/**
		Entidad gr�fica.
		*/
		Graphics::CEntity *_graphicsEntity;

		unsigned int _lightMask;

		/**
		Escena gr�fica donde se encontrar�n las representaciones gr�ficas de
		las entidades. La guardamos para la destrucci�n de la entidad gr�fica.
		*/
		Graphics::CScene* _scene;

		//std::list<std::string> _material;
		std::string _material;

		Ogre::ManualObject*					decalObject;
		std::vector<Ogre::ManualObject*>	vListaDecals;

		bool					_primerDecal;

		int						_iContadorDecals;

		int						_iMaxDecals;

	}; // class CGraphics

	REG_FACTORY(CGraphics);

} // namespace Logic

#endif // __Logic_Graphics_H
