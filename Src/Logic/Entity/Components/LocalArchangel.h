/**
@file LocalArchangel.h

Contiene la declaraci�n del componente que controla los efectos de
c�mara de la entidad archangel.

@see Logic::IComponent

@author Antonio Jesus Narvaez
@date Mayo, 2013
*/
#ifndef __Logic_LocalArchangel_H
#define __Logic_LocalArchangel_H

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

	class CLocalArchangel: public IComponent
	{
		DEC_FACTORY(CLocalArchangel);

	public:
		/**
		Constructor por defecto; inicializa los atributos a su valor por 
		defecto.
		*/
		CLocalArchangel(): _scene(0) {}

		/**
		Destructor (virtual); Quita de la escena y destruye la entidad gr�fica.
		*/
		virtual ~CLocalArchangel(){}
		
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
	
	protected:

		/**
		Invocado al despertar un componente.
		*/
		virtual void onWake();

		/**
		M�todo llamado en cada frame que actualiza el estado del componente de la particula.
				

		@param msecs Milisegundos transcurridos desde el �ltimo tick.
		*/
		virtual void onTick(unsigned int msecs);
	private:

		Graphics::CScene* _scene;

		std::string _effect;
		std::string _strengthEffect;

		int _timestamp;
		
	};//class CLocalArchangel
	REG_FACTORY(CLocalArchangel);
}

#endif