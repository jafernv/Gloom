//---------------------------------------------------------------------------
// SpectatorController.h
//---------------------------------------------------------------------------

/**
@file SpectatorController.h

Contiene la declaraci�n del componente que controla el movimiento 
del espectador.

@see Logic::CSpectatorController
@see Logic::IComponent

@author Francisco Aisa Garc�a
@date Abril, 2013
*/

#ifndef __Logic_SpectatorController_H
#define __Logic_SpectatorController_H

#include "Logic/Entity/Component.h"
#include "Logic/Maps/WorldState.h"

// Predeclaraci�n de clases
namespace Logic {
	class CMessageControl;
	class CPhysicController;
	class CSpectatorHud;
}

// Declaraci�n de la clase
namespace Logic {

	/**
	Componente que controla el manejo del espectador. Se encarga
	de mandar los comandos de movimiento al controlador f�sico y
	de procesar los comandos de movimiento recibidos.

	@ingroup logicGroup

	@author Francisco Aisa Garc�a
	@date Abril, 2013
	*/

	class CSpectatorController : public IComponent, public Logic::CWorldState::IObserver {
		DEC_FACTORY(CSpectatorController);
	public:


		// =======================================================================
		//                      CONSTRUCTORES Y DESTRUCTOR
		// =======================================================================


		/** Constructor por defecto. */
		CSpectatorController();
		
		//________________________________________________________________________

		/** Destructor. */
		virtual ~CSpectatorController();


		// =======================================================================
		//                   METODOS OBSERVADORES DE WORLDSTATE
		// =======================================================================


		/**
		Cada vez que una entidad se destruye se llama a este metodo.

		@param entity Entidad que va a ser destruida.
		*/
		virtual void entityDestroyed(CEntity* entity);


		// =======================================================================
		//                    METODOS HEREDADOS DE ICOMPONENT
		// =======================================================================


		/**
		Inicializaci�n del componente utilizando la informaci�n extra�da de
		la entidad le�da del mapa (Maps::CEntity). Toma del mapa el atributo
		speed que indica a la velocidad a la que se mover� el jugador.

		Inicializaci�n del componente a partir de la informaci�n extraida de la entidad
		leida del mapa:
		<ul>
			<li><strong>acceleration:</strong> Aceleraci�n del movimiento del espectador. </li>
			<li><strong>maxVelocity:</strong> M�xima velocidad a la que se puede mover el espectador. </li>
			<li><strong>frictionCoef:</strong> Coeficiente de deslizamiento del espectador. </li>
		</ul>

		@param entity Entidad a la que pertenece el componente.
		@param map Mapa L�gico en el que se registrar� el objeto.
		@param entityInfo Informaci�n de construcci�n del objeto le�do del fichero de disco.
		@return Cierto si la inicializaci�n ha sido satisfactoria.
		*/
		virtual bool spawn(CEntity* entity, CMap *map, const Map::CEntity *entityInfo);

		//________________________________________________________________________

		/**
		Metodo que se llama al activar el componente.
		Resetea los valores de inercia y desplazamiento.
		*/
		virtual void onActivate();

		//________________________________________________________________________

		/** 
		Este componente acepta los siguientes mensajes:

		<ul>
			<li>CONTROL</li>
		</ul>
		
		@param message Mensaje a chequear.
		@return true si el mensaje es aceptado.
		*/
		virtual bool accept(const std::shared_ptr<CMessage>& message);

		//________________________________________________________________________

		/**
		M�todo virtual que procesa un mensaje.

		@param message Mensaje a procesar.
		*/
		virtual void process(const std::shared_ptr<CMessage>& message);

		
		// =======================================================================
		//                            METODOS PROPIOS
		// =======================================================================


		/**
		Dado una rotacion en radianes del eje X e Y rota la entidad,
		limitando el pitch a +90/-90 grados.

		@param XYturn array con los valores de giro del raton.
		*/
		void rotationXY(float XYturn[]);

		//________________________________________________________________________

		/**
		Dado un enumerado indicando el tipo de movimiento se desplaza al player.

		@param commandType Enumerado que indica el tipo de movimiento a realizar.
		*/
		void executeMovementCommand(ControlType commandType);
		

	protected:


		// =======================================================================
		//                          M�TODOS PROTEGIDOS
		// =======================================================================


		/**
		Se ejecuta la primera vez que la entidad se activa. Garantiza que todas las 
		entidades (incluidos sus componentes) han ejecutado el spawn y est�n listas
		para hacer el tick.
		*/
		virtual void onStart();

		//________________________________________________________________________

		/**
		M�todo llamado en cada frame que actualiza el estado del componente.
		<p>
		Se encarga de mover la entidad en cada vuelta de ciclo cuando es
		necesario (cuando est� andando o desplaz�ndose lateralmente).

		@param msecs Milisegundos transcurridos desde el �ltimo tick. Siempre son
		constantes.
		*/
		virtual void onFixedTick(unsigned int msecs);

		//________________________________________________________________________

		/**
		Dado un vector de direcci�n simplificado (con cada coordenada entre 0 y 1)
		devuelve la direcci�n en la que el player deber�a mirar una vez aplicada
		la rotaci�n.

		@param displ Direcci�n relativa en la que queremos que el player se
		desplace.
		*/
		Vector3 estimateMotionDirection(const Vector3& displ) const;

		//________________________________________________________________________

		/**
		Calcula el desplazamiento del espectador.

		@param msecs Milisegundos transcurridos desde el �ltimo tick.
		*/
		void estimateMotion(unsigned int msecs);

		//________________________________________________________________________

		/** Normaliza el vector de desplazamiento al m�ximo establecido. */
		void normalizeDirection();

		//________________________________________________________________________	

		/**
		Inicializa el array que contiene los vectores que corresponden a cada comando
		de movimiento.
		*/
		void initMovementCommands();

		//________________________________________________________________________	

		/**
		Devuelve la siguiente entidad sobre la que vamos a espectar.

		@return Siguiente jugador para espectar.
		*/
		CEntity* getNextPlayerToSpectateAs();


		// =======================================================================
		//                          MIEMBROS PROTEGIDOS
		// =======================================================================


		/** Velocidad m�xima a la que nuestro personaje se puede desplazar. */
		float _maxVelocity;

		/** 
		Coeficiente de rozamiento del aire, entre 0 y 1. A menor valor, menor
		recorrido en el aire. 
		*/
		float _frictionCoef;

		/** Velocidad de aceleraci�n del personaje al desplazarse. */
		float _acceleration;

		/** Vector que indica la direcci�n del desplazamiento que el controlador debe realizar. */
		Vector3 _displacementDir;

		/** Vector de inercia. */
		Vector3 _momentum;

		/** Puntero a la entidad que estamos siguiendo */
		CEntity* _currentFollowedPlayer;

		/** Puntero al controlador fisico del player. Nos lo quedamos por motivos de eficiencia. */
		CPhysicController* _physicController;

		CSpectatorHud * _hud;

		/** 
		Array que contiene los vectores que corresponden a cada uno de los movimientos
		de desplazamiento y salto que se pueden realizar. 
		*/
		Vector3 _movementCommands[18];

		std::pair<std::string, CEntity*> _playerClasses[4];
		unsigned int _currentPlayerClass;

	}; // class CSpectatorController

	REG_FACTORY(CSpectatorController);

} // namespace Logic

#endif // __Logic_SpectatorController_H
