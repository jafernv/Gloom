/**
@file PhysicController.h

Contiene la declaraci�n del componente que se utiliza para representar jugadores y enemigos en
el mundo f�sico usando character controllers.

@see Logic::PhysicController
@see Logic::CPhysicEntity
@see Logic::IPhysics

@author Francisco Aisa Garc�a
@date Febrero, 2013
*/

#ifndef __Logic_PhysicController_H
#define __Logic_PhysicController_H

#include "Physics.h"
#include "Physics/CharacterController.h"

// Predeclaraci�n de tipos
namespace physx {
	class PxCapsuleController;
	struct PxControllersHit;
	struct PxControllerShapeHit;
};

// Los componentes se definen dentro del namespace Logica
namespace Logic {
	
	/**
	Componente que se utiliza para representar jugadores y enemigos en el mundo f�sico usando 
	character controllers.
	<p>
	Este componente recibe mensajes de tipo AVATAR_WALK indicando el movimiento que se quiere 
	realizar. A continuaci�n se le indica al motor de f�sica el movimiento deseado, y en el
	siguiente tick se actualiza la posici�n de la entidad l�gica. Debemos tener en cuenta que 
	la posici�n final la determina el motor de f�sica, ya que durante el movimiento podemos 
	colisionar con otra entidades, resbalar sobre paredes, etc.
	<p>
	Este componente tambi�n simula el efecto de la gravedad sobre el jugador (los controllers
	de PhysX no se ven afectados por la gravedad).
	
    @ingroup logicGroup

	@author Francisco Aisa Garc�a
	@date Febrero, 2013
	*/

	class CPhysicController : public IPhysics {
		DEC_FACTORY(CPhysicController);
	public:

		/**
		Constructor por defecto.
		*/
		CPhysicController();

		/**
		Destructor.
		*/
		virtual ~CPhysicController();
		
		/**
		Inicializa el componente usando los atributos definidos en el fichero de mapa.
		*/
		virtual bool spawn(CEntity* entity, CMap *map, const Map::CEntity *entityInfo);

		virtual std::string getPhysicName();

		/**
		Setea la posici�n del controlador
		*/
		void  setPhysicPosition (const Vector3 &position);

		/**
		Obtiene la posicion del controlador
		*/
		const Vector3& getPhysicPosition();

		/**
		Se invoca cuando se produce una colisi�n entre una entidad f�sica y un trigger.
		*/
		virtual void  onTrigger(IPhysics *otherComponent, bool enter);

		virtual void onContact(IPhysics *otherComponent, const Physics::CContactPoint& contactPoint, bool enter);

		/**
		Se invoca cuando se produce una colisi�n entre un character controller y una entidad f�sica.
		*/
		//void onShapeHit (const physx::PxControllerShapeHit &hit);

		virtual void onShapeHit(IPhysics *otherComponent, const Vector3& colisionPos, const Vector3& colisionNormal);

		/**
		Se invoca cuando se produce una colisi�n entre dos character controllers.
		*/
		//void onControllerHit (const physx::PxControllersHit &hit);

		void activateSimulation();

		void deactivateSimulation();

		/**
		Dado un vector de desplazamiento mueve la c�psula del player en esa direcci�n.
		Si se produce alguna colisi�n durante el desplazamiento se levantan los flags
		que correspondan en el entero devuelto.

		@param movement Vector de desplazamiento.
		@param msecs Tiempo durante el que queremos que se produzca el movimiento.
		@return Los flags de colisi�n que indican las colisiones que se han producido
		durante el desplazamiento del controlador.
		*/
		unsigned move(const Vector3& movement, unsigned int msecs);

		/**
		Dado un vector de desplazamiento mueve la c�psula del player en esa direcci�n.
		Si se produce alguna colisi�n durante el desplazamiento se levantan los flags
		que correspondan en el entero devuelto.

		@param movement Vector de desplazamiento.
		@param customFilterMask M�scara que establece con que grupos queremos colisionar
		durante la ejecuci�n de �ste move. Notar que este nuevo filtro ignora a el filtro
		por defecto.
		@param msecs Tiempo durante el que queremos que se produzca el movimiento.
		@return Los flags de colisi�n que indican las colisiones que se han producido
		durante el desplazamiento del controlador.
		*/
		unsigned move(const Vector3& movement, unsigned int customFilterMask, unsigned int msecs);

		unsigned int getDefaultFilterMask();

		float getCapsuleRadius();

		float getCapsuleHeight();

		bool isSimulating() { return _simulating; }

	protected:

		virtual void onActivate() { _simulating = true; }
		virtual void onDeactivate() { _simulating = false; }

	private:


		void readCollisionGroupInfo(const Map::CEntity *entityInfo, int& group, std::vector<int>& groupList);

		/**
		Crea el character controller de PhysX que representa la entidad f�sica a partir de la
		informaci�n del mapa.
		*/
		void createController(const Map::CEntity *entityInfo);

		// Character controller que representa la entidad f�sica en PhysX
		Physics::CCharacterController _controller;

		bool _simulating;

	}; // class CPhysicController

	REG_FACTORY(CPhysicController);

} // namespace Logic

#endif // __Logic_PhysicController_H
