//---------------------------------------------------------------------------
// StaticEntity.cpp
//---------------------------------------------------------------------------

/**
@file StaticEntity.cpp

@see Physics::CStaticEntity

@author Francisco Aisa Garc�a
@date Marzo, 2013
*/

#include "StaticEntity.h"

#include <assert.h>

#include "Physics/Server.h"
#include "Physics/CollisionManager.h"
#include "Conversions.h"

#include "Logic/Entity/Components/Physics.h"

#include <PxPhysics.h>
#include <PxScene.h>
#include <PxShape.h>
#include <PxRigidActor.h>
#include <PxRigidDynamic.h>
#include <PxRigidStatic.h>
#include <extensions/PxDefaultSimulationFilterShader.h>
#include <extensions/PxSimpleFactory.h>
#include <geometry/PxGeometryHelpers.h>

using namespace physx;

namespace Physics {

	void CStaticEntity::load(const Vector3 &position, const const physx::PxGeometry& geometry, physx::PxMaterial& material, 
					         bool trigger, int group, const std::vector<int>& groupList, const Logic::IPhysics *component) {

		assert(_scene);
						
		// Creamos una esfera est�tica
		PxTransform globalPose(Vector3ToPxVec3(position));
		
		// Transformaci�n de coordenadas l�gicas a coodenadas de PhysX
		PxTransform shapeOffset( PxVec3(0, getLogicPivotOffset(geometry), 0) );
		
		_actor = PxCreateStatic(*_physxSDK, globalPose, geometry, material, shapeOffset);

		// Transformarlo en trigger si es necesario
		if (trigger) {
			PxShape *shape;
			_actor->getShapes(&shape, 1, 0);
			shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
		}

		// Asociar el componente l�gico asociado a la entidad f�sica
		_actor->userData = (void*) component;

		// Establecer el grupo de colisi�n
		PxSetGroup(*_actor, group);
		// Establecer los filtros en base al grupo de colision
		Physics::CServer::getSingletonPtr()->setupFiltering(_actor, group, groupList);

		// A�adir el actor a la escena
		_scene->addActor(*_actor);
	}

} // namespace Physics
