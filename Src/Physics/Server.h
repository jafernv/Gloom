/**
@file Server.h

Contiene la declaraci�n del servidor de f�sica. 

@see Physics::CServer

@author Francisco Aisa Garc�a
@date Marzo, 2013
*/

#ifndef __Physics_Server_H
#define __Physics_Server_H

#include "BaseSubsystems/Math.h"

#include "GeometryFactory.h"
#include "MaterialManager.h"
#include "RaycastHit.h"
#include "SweepHit.h"

#include <PxFiltering.h>

// Predeclaraci�n de tipos
namespace Logic {
	class CEntity;
	class CPhysicController;
	class CPhysicEntity;
	class IPhysics;
};

namespace Physics {
	class CCollisionManager;
	class CErrorManager;
};

namespace physx {
	class PxActor;
	class PxAggregate;
	class PxAllocatorCallback;
	class PxCapsuleController;
	class PxController;
	class PxControllerManager;
	class PxCooking;
	class PxDefaultAllocator;
	class PxDefaultCpuDispatcher;
	class PxErrorCallback;
	class PxFoundation;
	class PxMaterial;
	class PxPhysics;
	class PxProfileZoneManager;
	class PxRigidActor;
	class PxRigidDynamic;
	class PxRigidStatic;
	class PxScene;
	class PxRigidBody;
	class PxControllerBehaviorCallback;

	namespace pxtask {
		class CudaContextManager;
	};

	namespace debugger {
		namespace comm {
			class PvdConnection;
		};
	};
};

namespace Physics {

	struct CollisionGroup {
		enum Enum {
			eWORLD					= (1 << 0),
			// En single player este es el grupo de colision del
			// personaje
			ePLAYER					= (1 << 1), 
			// En multiplayer cada jugador tiene asignado un grupo
			// de colision particular
			ePLAYER1				= (1 << 1),
			ePLAYER2				= (1 << 2),
			ePLAYER3				= (1 << 3),
			ePLAYER4				= (1 << 4),
			ePLAYER5				= (1 << 5),
			ePLAYER6				= (1 << 6),
			ePLAYER7				= (1 << 7),
			ePLAYER8				= (1 << 8),
			eSPECTATOR				= (1 << 9),
			// En single player este es el grupo de colision
			// de los enemigos
			eENEMY					= (1 << 10),
			eTRIGGER				= (1 << 11),
			eITEM					= (1 << 12),
			eTRAP					= (1 << 13),
			ePROJECTILE				= (1 << 14),
			eMAGNETIC_PROJECTILE	= (1 << 15),
			eFIREBALL				= (1 << 16),
			eSCREAMER_SHIELD		= (1 << 17),
			eLIGHT					= (1 << 18),
			eHITBOX					= (1 << 19)
		};

		unsigned int getPlayersMask() {
			return ePLAYER1 |
				   ePLAYER2 |
				   ePLAYER3 |
				   ePLAYER4 |
				   ePLAYER5 |
				   ePLAYER6 |
				   ePLAYER7 |
				   ePLAYER8;
		}
	};

	/**
	Esta funci�n es la que realmente se encarga que ocurra la magia de las notificaciones.
	Solo seran notificados de colisiones y triggers aquellos actores cuyos grupos de colisi�n
	hayan sido incluidos en la lista de grupos de colisi�n del otro actor con el que colisionan.

	Las m�scaras y grupos de colisi�n se fijan en la funci�n setupFiltering.

	@param attributes0 Informaci�n general sobre el primer objeto.
	@param filterData0 Datos de filtro del primer objeto.
	@param attributes1 Informaci�n general sobre el segundo objeto.
	@param filterData1 Datos de filtro del segundo objeto.
	@param pairFlags Flags de la pareja.
	@param constantBlock Puntero al bloque de datos.
	@param constantBlockSize Tama�o del bloque de datos.
	@return Los flags que indican que tipo de notificaci�n debe dispararse.
	*/
	physx::PxFilterFlags customFilterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
											physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
											physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize);

	//________________________________________________________________________

	/**
	Clase principal del proyecto de f�sicas que se encarga de la inicializaci�n,
	gesti�n y liberaci�n del motor de f�sicas.
	
	El resto de clases del proyecto se apoyan en la comunicaci�n con esta clase
	para interactuar con el motor.

	De cara a la l�gica, el servidor abstrae la inicializaci�n y gesti�n de PhysX, 
	sirviendo de puente para la comunicaci�n entre la f�sica y el resto de proyectos.

	Es importante notar que esta clase no se encarga del manejo de las entidades de PhysX,
	tan solo de su simulaci�n; cada uno de los distintos tipos de entidades f�sicas que la 
	l�gica puede utilizar se encuentran abstraidos en clases distintas que se comunican 
	con el servidor (por motivos de coherencia y modularidad).
	
	IMPORTANTE: Esta clase est� implementada como un Singleton de inicializaci�n expl�cita: es necesario 
	invocar al m�todo Init() al principio de la aplicaci�n y al m�todo Release() al final.

	@ingroup physicGroup

	@author Francisco Aisa Garc�a
	@date Marzo, 2013
	*/

	class CServer {
	public:


		// =======================================================================
		//           M�TODOS DE INICIALIZACI�N Y LIBERACI�N DE RECURSOS
		// =======================================================================


		/**
		Devuelve la �nica instancia de la clase.

		@return Puntero al servidor f�sico.
		*/
		static CServer* getSingletonPtr() { return _instance; }

		//________________________________________________________________________
		
		/**
		Inicializa el servidor f�sico. Esta operaci�n s�lo es necesario realizarla
		una vez durante la inicializaci�n de la aplicaci�n. 

		@return Devuelve false si no se ha podido inicializar.
		*/
		static bool Init();

		//________________________________________________________________________

		/**
		Libera el servidor f�sico. Debe llamarse al finalizar la aplicaci�n.
		*/
		static void Release();

		//________________________________________________________________________

		/**
		Elimina una entidad f�sica de la escena y libera los recursos que tenga asociados.
		NO DEBE USARSE CON CHARACTER CONTROLLERS.

		@param actor Actor f�sico asociado a la entidad.
		 */
		void destroyActor(physx::PxActor* actor);

		//________________________________________________________________________

		/**
		Elimina un agregado de la escena junto con todos los actores asociados a dicho
		agregado.

		@param aggregate Agregado asociado a la entidad.
		 */
		void destroyAggregate(physx::PxAggregate* aggregate);


		// =======================================================================
		//								   TICK
		// =======================================================================


		/**
		Realiza la simulaci�n f�sica y actualiza la posici�n de todas las 
		entidades f�sicas.

		La simulaci�n f�sica se hacen en peque�os steps de tiempo para evitar
		que se produzcan situaciones inesperadas al haber ticks muy grandes.

		@param secs Millisegundos transcurridos desde la �ltima actualizaci�n.
		@return Valor booleano indicando si todo fue bien.
		*/
		bool tick(unsigned int msecs);


		// =======================================================================
		//                 M�TODOS DE GESTI�N DE LA ESCENA F�SICA
		// =======================================================================


		/**
		Crea la escena f�sica. De momento asumimos que solo vamos a tener una escena
		f�sica.

		En caso contrario ser�a conveniente crear un gestor de escenas y abstraer
		la escena de PhysX en una nueva clase.
		*/
		void createScene();

		//________________________________________________________________________

		/**
		Destruye la escena f�sica.
		*/
		void destroyScene();


		// =======================================================================
		//            M�TODOS PARA LA GESTI�N DE LOS GRUPOS DE COLISI�N
		// =======================================================================


		/**
		Establece si debe haber colisiones entre dos grupos de colisi�n. Al crear los objetos
		f�sicos se establece el grupo de colisi�n al que pertenecen. Usando este m�todo es
		posible activar / desactivar las colisiones entre grupos.

		IMPORTANTE: Activar los grupos de colisi�n solo funciona para entidades y controladores.
		Los triggers no est�n incluidos. Para desactivar triggers y notificaciones hay
		que hacer uso del filter shader.

		@param group1 Primer grupo de colisi�n.
		@param group2 Segundo grupo de colisi�n
		@param enable Activar o desactivar las colisiones entre los dos grupos anteriores.
		*/
		void setGroupCollisions(int group1, int group2, bool enable);

		//________________________________________________________________________
		
		/**
		Dado un actor y su grupo de colisi�n, establece con que grupos de colisi�n
		debe haber notificaciones. Es importante notar que para que dos actores sean
		notificados de una colisi�n o de la activaci�n/desactivaci�n de un trigger,
		los grupos de colisi�n de ambos deben estar incluidos en la lista de grupos
		con los que colisionan.

		Esta funci�n hace uso de m�scaras de bits. Dichas m�scaras se asignan en funci�n
		de los grupos de colisi�n asignados a las entidades. Para llevar a cabo
		esta tarea existe otra funci�n (customFilterShader) que se pasa como callback a
		PhysX.

		@param actor Actor de PhysX al que queremos configurar sus filtros.
		@param group Grupo de colisi�n del actor.
		@param groupList Grupos de colisi�n con los que queremos que el actor sea notificado
		en caso de colisi�n.
		*/
		void setupFiltering(physx::PxRigidActor* actor, int group, const std::vector<int>& groupList);

		
		// =======================================================================
		//                     QUERIES DE OVERLAP Y RAYCAST
		// =======================================================================


		/**
		Lanza un rayo y devuelve informaci�n sobre las entidades golpeadas. En caso de no golpear
		a ninguna entidad nbHits es igual a 0.

		@param ray Rayo que queremos disparar.
		@param maxDistance Longitud m�xima del rayo.
		@param hits Array de hits obtenido al lanzar el rayo. Si no se ha producido ning�n hit
		el vector devuelto estar� vacio. IMPORTANTE: El vector no se limpia al entrar, as� que
		tened cuidado de no pasarle un vector que ya tenga datos.
		@param sortResultingArray Por defecto el array de colisiones devuelto no est� ordenado.
		Si ponemos este parametro a true, se ordenan los resultados por distancia, es decir,
		primero estaran los elementos m�s cercanos al origen del raycast que han golpeado con
		el rayo.
		@param filterMask M�scara que indica contra que grupos de colisi�n queremos que choque
		el raycast. En la cabecera del Server (este mismo documento) se especifican los enumerados
		que corresponden a cada grupo de colisi�n. Se entiende que en el mapa deben usarse 
		estos grupos con la misma numeraci�n (si no, no funciona nada). Si por ejemplo
		quisieramos que el raycast chocase con el mundo y los players tendr�amos que indicar
		lo siguiente como m�scara: filterMask = CollisionGroup::eWORLD | CollisionGroup::ePLAYER.
		Por defecto la m�scara es 0. He utilizado ese valor para indicarle al raycast
		que ejecute el comportamiento por defecto (sin filtros).
		*/
		void raycastMultiple(const Ray& ray, float maxDistance, std::vector<CRaycastHit>& hits, bool sortResultingArray = false,
							 unsigned int filterMask = 0) const;

		//________________________________________________________________________

		/**
		Lanza un rayo y devuelve la primera entidad contra la que golpea. En caso de no
		golpear contra ninguna entidad devuelve false.

		@param ray Rayo que queremos disparar.
		@param maxDistance Longitud m�xima del rayo.
		@param hit Estructura que devuelve el punto de colisi�n, la distancia y la normal
		del golpeo.
		@param filterMask M�scara que indica contra que grupos de colisi�n queremos que choque
		el raycast.
		@return true si se ha golpeado a una entidad.
		*/
		bool raycastSingle(const Ray& ray, float maxDistance, CRaycastHit& hit, unsigned int filterMask = 0) const;

		//________________________________________________________________________

		/**
		Lanza un rayo y devuelve true si se ha golpeado algo. Es el m�s barato de todas las
		queries de raycast. Si solo nos interesa saber si golpeamos algo usad esta query.

		@param ray Rayo que queremos disparar
		@param maxDistance Longitud m�xima del rayo.
		@param filterMask M�scara que indica contra que grupos de colisi�n queremos que choque
		el raycast.
		@return true si se ha golpeado algo, false en caso contrario.
		*/
		bool raycastAny(const Ray& ray, float maxDistance, unsigned int filterMask = 0) const;

		//________________________________________________________________________

		/**
		Dada una geometr�a, realiza una query de overlap y devuelve un vector con los punteros
		a las entidades que colisionan con dicha geometr�a.

		IMPORTANTE: Por motivos de eficiencia esta funci�n aplico un filtro a esta funci�n para que
		solo tenga en cuenta a las entidades din�micas.

		@param geometry Geometr�a para la query de overlap. Mirar la documentaci�n para ver cuales
		est�n soportadas.
		@param position Posici�n en la que queremos situar el centro de la geometr�a.
		@param entitiesHit Array que contiene los punteros a las entidades golpeadas en el overlap. Si
		no se ha golpeado a ninguna entidad en el overlap el tama�o del vector ser� 0. IMPORTANTE:
		El vector que se pasa debe de no haber sido inicializado previamente.
		@param filterMask M�scara que indica contra que grupos de colisi�n queremos que choque
		el overlap.
		*/
		void overlapMultiple(const physx::PxGeometry& geometry, const Vector3& position, std::vector<Logic::CEntity*>& entitiesHit, unsigned int filterMask = 0) const;

		//________________________________________________________________________

		/**
		Dada una geometr�a, realiza una query de overlap y devuelve true si la geometr�a dada
		colisiona contra alg�n actor (din�mico o est�tico).

		Es mucho m�s eficiente que overlapMultiple. Usar cuando no nos interese saber contra
		qu� colisiona la geometr�a.

		@param geometry Geometr�a para la query de overlap. Mirar la documentaci�n para ver que geometr�as
		est�n soportadas.
		@param position Posici�n donde queremos colocar el centro de la geometr�a dada.
		@param filterMask M�scara que indica contra que grupos de colisi�n queremos que choque
		el overlap.
		@return True si existe colisi�n con alg�n actor, falso en caso contrario.
		*/
		bool overlapAny(const physx::PxGeometry& geometry, const Vector3& position, unsigned int filterMask = 0) const;


		//________________________________________________________________________

		/**
		Dada una geometr�a, realiza una query de sweep y devuelve un vector con los puntos
		de colisi�n encontrados.

		IMPORTANTE: Esta funci�n puede ser realmente cara. Usar con mucho cuidado.

		@param geometry Geometr�a para la query de sweep. Mirar la documentaci�n para ver cuales
		est�n soportadas.
		@param position Posici�n en la que queremos situar el centro de la geometr�a.
		@param unitDir Direcci�n unitaria en la que queremos que se realice el barrido.
		@param distance Distancia m�xima del barrido.
		@param hitSpots Array que devuelve los puntos de colision encontrados. Si no se 
		encuentras puntos de contacto el tama�o del vector resultante es 0. IMPORTANTE:
		El vector que se pasa por par�metro debe estar vacio.
		@param filterMask M�scara que indica contra que grupos de colisi�n queremos que choque
		la query de sweep.
		*/
		void sweepMultiple(const physx::PxGeometry& geometry, const Vector3& position,
						   const Vector3& unitDir, float distance, std::vector<CSweepHit>& hitSpots, 
						   bool sortResultingArray = false, unsigned int filterMask = 0) const;

		//________________________________________________________________________

		/**
		Dada una geometr�a, realiza una query de sweep y devuelve true si la geometr�a dada
		colisiona contra un actor (din�mico o est�tico). Adem�s devuelve informaci�n sobre
		la colisi�n.

		Es mucho m�s eficiente que sweepMultiple. Usar cuando solo estemos interesados
		en un hit.

		@param geometry Geometr�a para la query de sweep. Mirar la documentaci�n para ver que geometr�as
		est�n soportadas.
		@param position Posici�n donde queremos colocar el centro de la geometr�a dada.
		@param unitDir Direcci�n unitaria en la que queremos que se realice el barrido.
		@param distance Distancia m�xima del barrido.
		@param hitSpot Si ha habido colisi�n en el barrido, se guarda en esta variable
		el punto de colisi�n.
		@param filterMask M�scara que indica contra que grupos de colisi�n queremos que choque
		la query de sweep.
		@return True si existe colisi�n con alg�n actor, falso en caso contrario.
		*/
		bool sweepSingle(const physx::PxGeometry& sweepGeometry, const Vector3& position, 
						 const Vector3& unitDir, float distance, Vector3& hitSpot, unsigned int filterMask = 0) const;

		//________________________________________________________________________

		/**
		Dada una geometr�a, realiza una query de sweep y devuelve true si la geometr�a dada
		colisiona contra un actor (din�mico o est�tico).

		Es la query de sweep m�s eficiente.

		@param geometry Geometr�a para la query de sweep. Mirar la documentaci�n para ver que geometr�as
		est�n soportadas.
		@param position Posici�n donde queremos colocar el centro de la geometr�a dada.
		@param unitDir Direcci�n unitaria en la que queremos que se realice el barrido.
		@param distance Distancia m�xima del barrido.
		@param filterMask M�scara que indica contra que grupos de colisi�n queremos que choque
		la query de sweep.
		@return true Si el barrido ha impactado contra algo.
		*/
		bool sweepAny(const physx::PxGeometry& sweepGeometry, const Vector3& position, 
					  const Vector3& unitDir, float distance, unsigned int filterMask = 0) const;


		// =======================================================================
		//               M�TODOS PARA LA OBTENCI�N DE INFO DE PHYSX
		// =======================================================================


		/**
		Devuelve un puntero a la escena actual de PhysX.

		@return Puntero a la escena de PhysX.
		*/
		physx::PxScene* getActiveScene() const { return _scene; }

		//________________________________________________________________________

		/**
		Devuelve un puntero al core de PhysX.

		@return Puntero al core de PhysX.
		*/
		physx::PxPhysics* getPhysxSDK() const { return _physics; }

		//________________________________________________________________________

		/**
		Devuelve un puntero al gestor de controladores de PhysX.

		@return Puntero al gestor de controladores de PhysX.
		*/
		physx::PxControllerManager* getControllerManager() const { return _controllerManager; }

		//________________________________________________________________________

		/**
		Devuelve un puntero al sistema de cocinado de PhysX.

		@return Puntero al sistema de cocinado de PhysX.
		*/
		physx::PxCooking* getCooking() const  { return _cooking; }

		//________________________________________________________________________

		/**
		Devuelve un puntero a foundation.

		@return Puntero a foundation.
		*/
		physx::PxFoundation* getFoundation() const { return _foundation; }

		//________________________________________________________________________

		/**
		Devuelve un puntero al gestor de colisiones.

		@return Puntero al gestor de colisiones.
		*/
		CCollisionManager* getCollisionManager() const { return _collisionManager; }

	private:	


		// =======================================================================
		//                      CONSTRUCTORES Y DESTRUCTOR
		// =======================================================================


		/** Constructor de la clase. */
		CServer();

		//________________________________________________________________________

		/** Destructor de la clase. */
		virtual ~CServer();


		// =======================================================================
		//                          MIEMBROS PRIVADOS
		// =======================================================================


		/** Instancia �nica de la clase (por ser singleton). */
		static CServer* _instance;

		/** Puntero al gestor de errores. */
		physx::PxErrorCallback* _errorManager;

		/** Puntero al gestor de memoria de PhysX. */
		physx::PxAllocatorCallback* _allocator;

		/** Foundation de PhysX. */
		physx::PxFoundation* _foundation;

		/** Profile zone manager. */
		physx::PxProfileZoneManager* _profileZoneManager;

		/** CPU dispatcher de PhysX. Puede ser sobreescrito. */
		physx::PxDefaultCpuDispatcher* _cpuDispatcher;
		
		/** Manejador del procesamiento de c�lculos en GPU. */
		physx::pxtask::CudaContextManager* _cudaContextManager;
		
		/** Para la gesti�n del debugger. */
		physx::debugger::comm::PvdConnection* _pvdConnection;
		
		/** Puntero al sistema de cocinado de PhysX. */
		physx::PxCooking* _cooking;

		/** Puntero al core de PhysX. */
		physx::PxPhysics* _physics;

		/** Puntero a la escena activa de PhysX. */
		physx::PxScene* _scene;

		/** Puntero al gestor de controladores de PhysX. */
		physx::PxControllerManager* _controllerManager;

		/** Puntero al gestor de colisiones. */
		CCollisionManager* _collisionManager;

		/** Tiempo real acumulado. Sirve para comprobar si tenemos que realizar un o varios steps de simulaci�n f�sica. */
		unsigned int _acumTime;

		/** Tama�o del timestep que tomamos para realizar una simulaci�n. */
		unsigned int _fixedTime;

	}; // class CServer

}; // namespace Physics

#endif // __Physics_Server_H
