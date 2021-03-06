/**
@file LocalShadow.h

Contiene la declaraci�n del componente que controla los efectos de
c�mara de la entidad.

@see Logic::IComponent

@author Rub�n Mulero
@date Agosto, 2010
*/
#ifndef __Logic_LocalShadow_H
#define __Logic_LocalShadow_H

#include "Logic/Entity/Component.h"

// Predeclaraci�n de clases para ahorrar tiempo de compilaci�n
namespace Graphics 
{
	class CScene;
}

//declaraci�n de la clase
namespace Logic 
{
	/**
	Componente que se encarga del manejo gr�fico del shadow a nivel de jugador
	local, cambiando y creando los compositors adecuados para que la c�mara
	muestre el feedback que representa a la clase Shadow

	@ingroup logicGroup

	@author Rub�n Mulero Guerrero
	@date May, 2010

	*/

	class CLocalShadow: public IComponent
	{
		DEC_FACTORY(CLocalShadow);

	public:
		/**
		Constructor por defecto; inicializa los atributos a su valor por 
		defecto.
		*/
		CLocalShadow(): _scene(0) {}

		/**
		Destructor (virtual); Quita de la escena y destruye la entidad gr�fica.
		*/
		virtual ~CLocalShadow(){}
		
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
		M�todo virtual que elige que mensajes son aceptados.

		@param message Mensaje a chequear.
		@return true si el mensaje es aceptado.
		*/
		virtual bool accept(const std::shared_ptr<CMessage>& message);

		/**
		M�todo que activa el componente; invocado cuando se activa
		el mapa donde est� la entidad a la que pertenece el componente.
		*/
		virtual void onActivate();

		/**
		M�todo que desactiva el componente, de manera que no recibe mensajes
		ni hace tick
		*/
		//virtual void deactivate();


		/**
		M�todo virtual que procesa un mensaje.

		@param message Mensaje a procesar.
		*/
		virtual void process(const std::shared_ptr<CMessage>& message);

	private:

		Graphics::CScene* _scene;

		std::string _distorsion;
		std::string _colourEffect;
	};//class CLocalShadow
	REG_FACTORY(CLocalShadow);
}

#endif