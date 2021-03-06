/**
@file PhysicStaticEntity.h

@see Logic::CPhysicStaticEntity
@see Logic::IComponent
@see Logic::CPhysicController

@author Francisco Aisa Garc�a
@date Marzo, 2013
*/

#ifndef __Logic_PhysicStaticEntity_H
#define __Logic_PhysicStaticEntity_H

#include "Physics.h"
#include "Physics/StaticEntity.h"

#include <vector>

// Predeclaraci�n de tipos
namespace physx {
	class PxRigidActor;
	class PxRigidStatic;
	class PxControllerShapeHit;
};

namespace Physics {
	class CServer;
	class CGeometryFactory;
	class CMaterialManager;
};

// Los componentes pertenecen al namespace Logic
namespace Logic {	
	
	/**

    @ingroup logicGroup

	@author Francisco Aisa Garc�a
	@date Marzo, 2013
	*/
	class CPhysicStaticEntity : public IPhysics {
		DEC_FACTORY(CPhysicStaticEntity);
	public:
	
		/**
		Constructor por defecto.
		*/
		CPhysicStaticEntity();

		/**
		Destructor. Elimina el objeto f�sico de la escena y lo destruye. 
		*/
		virtual ~CPhysicStaticEntity();
		
		/**
		Inicializa el componente usando los atributos definidos en el fichero de mapa.
		*/
		virtual bool spawn(CEntity* entity, CMap *map, const Map::CEntity *entityInfo);

		/**
		Este componente s�lo acepta mensajes de tipo KINEMATIC_MOVE. Estos mensajes  
		s�lo se utilizan para mover entidades de tipo cinem�tico.
		*/
		virtual bool accept(const std::shared_ptr<CMessage>& message);

		/**
		Cuando recibe mensajes de tipo KINEMATIC_MOVE almacena los movimientos para aplicarlos 
		en el pr�ximo tick sobre la entidad cinem�tica. Si en un ciclo se reciben varios 
		mensajes KINEMATIC_MOVE se acumulan. 
		*/
		virtual void process(const std::shared_ptr<CMessage>& message);


		virtual std::string getPhysicName();

 
		/**
		Se invoca cuando se produce una colisi�n entre una entidad f�sica y un trigger.
		*/
		virtual void onTrigger (IPhysics *otherComponent, bool enter);

		virtual void onContact(IPhysics *otherComponent, const Physics::CContactPoint& contactPoint, bool enter);

	    //void onShapeHit(const physx::PxControllerShapeHit &hit);

		virtual void onShapeHit(IPhysics *otherComponent, const Vector3& colisionPos, const Vector3& colisionNormal);

		//Metodo que devuelve el booleano _inTrigger que nos indica si hay alguien dentro del trigger
		bool getInTrigger(){ return _inTrigger; };

		//Metodo que devuelve el booleano _inContact que nos indica si hay alguien tocando la entidad
		bool getInContact(){ return _inContact; };

		bool getInControllerContact(){ return _inControllerContact; };

		void deactivateSimulation();

		void activateSimulation();

		void setInTrigger(bool enter){ _inTrigger=enter;};

	private:

		void readCollisionGroupInfo(const Map::CEntity *entityInfo, int& group, std::vector<int>& groupList);

		/**
		Crea el actor de PhysX que representa la entidad f�sica a partir de la
		informaci�n del mapa.
		*/
		void createPhysicEntity(const Map::CEntity *entityInfo);

		/**
		Crea un plano est�tico a partir de la informaci�n de mapa. 
		*/
		void createPlane(const Map::CEntity *entityInfo, int group, const std::vector<int>& groupList);
				
		/**
		Crea una entidad r�gida (est�tica, din�mica o cinem�tica) a partir de la informaci�n de mapa. 
		*/
		void createRigid(const Map::CEntity *entityInfo, int group, const std::vector<int>& groupList);
		
		/**
		Crea una entidad r�gida (est�tica, din�mica o cinem�tica) a partir de un fichero RepX
		exportando previamente con el plug-in the PhysX para 3ds Max.
		*/
		void createFromFile(const Map::CEntity *entityInfo, int group, const std::vector<int>& groupList);

		// Servidor de f�sica
		Physics::CServer* _server;

		Physics::CGeometryFactory* _geometryFactory;

		Physics::CMaterialManager* _materialManager;

		// Actor que representa la entidad f�sica en PhysX
		//physx::PxRigidActor *_actor;

		Physics::CStaticEntity _physicEntity;

		//Physics::CEntity _physicEntity;

		// Vector de deplazamiento recibido en el �ltimo mensaje de tipo KINEMATIC_MOVE. Sirve
		// para mover entidades f�sicas cinem�ticas.
		Vector3 _movement;

		//Booleano que controla si hay alguien dentro del posible trigger que puede tener la entidad, por defecto desactivado
		bool _inTrigger;

		//Booleano que controla si hay alguien en contacto con la entidad, por defecto desactivado
		bool _inContact;

		bool _inControllerContact;

		bool _sleepUntil;

	}; // class CPhysicStaticEntity

	REG_FACTORY(CPhysicStaticEntity);

} // namespace Logic

#endif // __Logic_PhysicEntity_H
