/**
@file PhysicDynamicEntity.h

@see Logic::CPhysicDynamicEntity
@see Logic::IComponent

@author Francisco Aisa Garc�a
@date Marzo, 2013
*/

#include "PhysicDynamicEntity.h"
#include "Logic/Entity/Entity.h"
#include "Map/MapEntity.h"
#include "Physics/Server.h"
#include "Physics/GeometryFactory.h"
#include "Physics/MaterialManager.h"

#include "Logic/Messages/MessageKinematicMove.h"
#include "Logic/Messages/MessageTouched.h"
#include "Logic/Messages/MessageActivate.h"
#include "Logic/Messages/MessageUntouched.h"
#include "Logic/Messages/MessageSetPhysicPosition.h"
#include "Logic/Messages/MessageTransform.h"
#include "Logic/Messages/MessageAddForcePhysics.h"
#include "Logic/Messages/MessageContactEnter.h"
#include "Logic/Messages/MessageContactExit.h"

#define _USE_MATH_DEFINES
#include <math.h>

using namespace Logic;
using namespace Physics;

IMP_FACTORY(CPhysicDynamicEntity);

//---------------------------------------------------------

CPhysicDynamicEntity::CPhysicDynamicEntity() : _movement(0,0,0),
											   _inTrigger(false),
											   _noGravity(false),
											   _inContact(false),
											   _sleepUntil(false) {

	_server = CServer::getSingletonPtr();
	_geometryFactory = Physics::CGeometryFactory::getSingletonPtr();
	_materialManager = Physics::CMaterialManager::getSingletonPtr();
}

//---------------------------------------------------------

CPhysicDynamicEntity::~CPhysicDynamicEntity() {
	// El destructor de _physicEntity ya sabe que hacer

	_server = NULL;
} 

////---------------------------------------------------------

bool CPhysicDynamicEntity::spawn(Logic::CEntity *entity, CMap *map, const Map::CEntity *entityInfo) {
	// Invocar al m�todo de la clase padre
	if( !IComponent::spawn(entity,map,entityInfo) ) return false;

	// Crear el objeto f�sico asociado al componente
	createPhysicEntity(entityInfo);

	return true;
} 

//---------------------------------------------------------

bool CPhysicDynamicEntity::accept(const std::shared_ptr<CMessage>& message) {
	Logic::TMessageType msgType = message->getMessageType();

	return	msgType == Message::ACTIVATE				||
			msgType == Message::KINEMATIC_MOVE			||
			msgType == Message::SET_PHYSIC_POSITION		||
			msgType == Message::ADD_FORCE_PHYSICS		||
			msgType == Message::SET_TRANSFORM;
}

//---------------------------------------------------------

void CPhysicDynamicEntity::process(const std::shared_ptr<CMessage>& message) {
	switch( message->getMessageType() ) {
		case Message::ACTIVATE: {
			bool activate = std::static_pointer_cast<CMessageActivate>(message)->getActivated();
			if(activate)
				activateSimulation();
			else
				deactivateSimulation();
			break;
		}
		case Message::SET_PHYSIC_POSITION: {
			std::shared_ptr<CMessageSetPhysicPosition> setPosMsg = std::static_pointer_cast<CMessageSetPhysicPosition>(message);
			setPosition( setPosMsg->getPosition(), setPosMsg->getMakeConversion() );
			break;
		}
		case Message::SET_TRANSFORM: {
			std::shared_ptr<CMessageTransform> transMsg = std::static_pointer_cast<CMessageTransform>(message);
			setGlobalPose(transMsg->getPosition(),transMsg->getOrientation(),transMsg->getMakeConversion());
			break;
		}
		case Message::ADD_FORCE_PHYSICS: {
			std::shared_ptr<CMessageAddForcePhysics> forceMsg = std::static_pointer_cast<CMessageAddForcePhysics>(message);

			if( !forceMsg->getGravity() )
				_physicEntity.disableGravity(true);

			addForce( forceMsg->getForceVector(), forceMsg->getForceMode() );
			break;
		}
	}
}

//---------------------------------------------------------

std::string CPhysicDynamicEntity::getPhysicName() {
	return _physicEntity.getName();
}

//---------------------------------------------------------

void CPhysicDynamicEntity::onFixedTick(unsigned int msecs) {
	// Actualizar la posici�n y la orientaci�n de la entidad l�gica usando la 
	// informaci�n proporcionada por el motor de f�sica	
	_entity->setPosition( _physicEntity.getPosition() );
	_entity->setOrientation( _physicEntity.getOrientation() );
}

//---------------------------------------------------------

void CPhysicDynamicEntity::readCollisionGroupInfo(const Map::CEntity *entityInfo, int& group, std::vector<int>& groupList) {
	// Leer el grupo de colisi�n (por defecto grupo 0)
	if (entityInfo->hasAttribute("physic_group"))
		group = entityInfo->getIntAttribute("physic_group");

	// Comprobamos los grupos con los que esta entidad deberia colisionar
	if (entityInfo->hasAttribute("physic_groupList")) {
		std::istringstream groupListString(entityInfo->getStringAttribute("physic_groupList"));

		// Para cada cadena entre comas...
		do {
			std::string groupNumber;
			std::getline(groupListString, groupNumber, ','); // linea entre delimitadores
				
			std::istringstream str(groupNumber);     // wrappeamos cadena como Input Stream
			do {									// Le quitamos los espacios...
				std::getline(str, groupNumber, ' ');  // linea entre espacios
			} while (groupNumber.size() == 0 && !str.eof());

			groupList.push_back( atoi(groupNumber.c_str()) );
		} while (!groupListString.eof());
	}
}

//---------------------------------------------------------

void CPhysicDynamicEntity::createPhysicEntity(const Map::CEntity *entityInfo) {
	// Leemos el tipo de entidad
	assert(entityInfo->hasAttribute("physic_entity"));
	const std::string physicEntity = entityInfo->getStringAttribute("physic_entity");
	assert((physicEntity == "rigid") || (physicEntity == "fromFile"));

	// Leemos la informacion de grupos de colision
	int group = 0;
	std::vector<int> groupList;
	readCollisionGroupInfo(entityInfo, group, groupList);
	
	if (physicEntity == "rigid") 
		createRigid(entityInfo, group, groupList);
	
	if (physicEntity == "fromFile")
		createFromFile(entityInfo, group, groupList);
}

//---------------------------------------------------------

void CPhysicDynamicEntity::createRigid(const Map::CEntity *entityInfo, int group, const std::vector<int>& groupList) {
	
	// Leer el tipo de entidad: est�ticos, din�mico o cinem�tico
	assert(entityInfo->hasAttribute("physic_type"));
	const std::string physicType = entityInfo->getStringAttribute("physic_type");
	assert((physicType == "dynamic") || (physicType == "kinematic"));

	// Leer la forma (shape)
	assert(entityInfo->hasAttribute("physic_shape"));
	const std::string physicShape = entityInfo->getStringAttribute("physic_shape");
	assert(physicShape == "box" || physicShape == "sphere" || physicShape == "capsule");

	// Leer si es un trigger (por defecto no)
	bool isTrigger = false;
	if (entityInfo->hasAttribute("physic_trigger"))
		isTrigger = entityInfo->getBoolAttribute("physic_trigger");

	// Leer la masa (por defecto 0)
	float mass = 0;
	if (entityInfo->hasAttribute("physic_mass"))
		mass = entityInfo->getFloatAttribute("physic_mass");
		
	// Leer si se trata de un actor cinem�tico
	bool isKinematic = (physicType == "kinematic");

	//Leemos si se le quita la gravedad al din�mico
	if (entityInfo->hasAttribute("physic_noGravity"))
		_noGravity = entityInfo->getBoolAttribute("physic_noGravity");

	if (physicShape == "box") {
		// Leer las dimensiones de la caja
		assert(entityInfo->hasAttribute("physic_dimensions"));
		const Vector3 physicDimensions = entityInfo->getVector3Attribute("physic_dimensions");

		Physics::BoxGeometry box = _geometryFactory->createBox(physicDimensions);
		Physics::Material* defaultMaterial = _materialManager->getMaterial(Physics::MaterialType::eDEFAULT);
		float density = mass / (physicDimensions.x * physicDimensions.y * physicDimensions.z);
		
		_physicEntity.load(_entity->getPosition(), _entity->getOrientation(), box, *defaultMaterial, density, isKinematic, isTrigger, _noGravity, group, groupList, this);
	}
	else if (physicShape == "sphere") {
		assert(entityInfo->hasAttribute("physic_radius"));
		const float physicRadius = entityInfo->getFloatAttribute("physic_radius");
		
		Physics::SphereGeometry sphere = _geometryFactory->createSphere(physicRadius);
		Physics::Material* defaultMaterial = _materialManager->getMaterial(Physics::MaterialType::eDEFAULT);
		float density = mass / (4.0f/3.0f * Math::PI * physicRadius * physicRadius * physicRadius);

		_physicEntity.load(_entity->getPosition(), _entity->getOrientation(), sphere, *defaultMaterial, density, isKinematic, isTrigger, _noGravity, group, groupList, this);
	}
	else if(physicShape == "capsule") {
		assert(entityInfo->hasAttribute("physic_radius"));
		assert(entityInfo->hasAttribute("physic_height"));
		const float radius = entityInfo->getFloatAttribute("physic_radius");
		const float height = entityInfo->getFloatAttribute("physic_height");
		float cilinderHeight = height - (2 * radius); // Altura del cilindro
		
		Physics::CapsuleGeometry capsule = _geometryFactory->createCapsule(radius, height);
		Physics::Material* defaultMaterial = _materialManager->getMaterial(Physics::MaterialType::eDEFAULT);
		// Densidad = Masa / volumen del cilindro + volumen de las dos semiesferas (o volumen de una esfera)
		float density = mass / (4.0f/3.0f * Math::PI * pow(radius, 3)) + (cilinderHeight * Math::PI * pow(radius, 2));

		_physicEntity.load(_entity->getPosition(), _entity->getOrientation(), capsule, *defaultMaterial, density, isKinematic, isTrigger, _noGravity, group, groupList, this);
	}
}

//---------------------------------------------------------

void CPhysicDynamicEntity::createFromFile(const Map::CEntity *entityInfo, int group, const std::vector<int>& groupList) {
	// Leer la ruta del fichero RepX
	assert(entityInfo->hasAttribute("physic_file"));
	const std::string file = entityInfo->getStringAttribute("physic_file");

	// Crear el actor a partir del fichero RepX
	_physicEntity.load(file, group, groupList, this);
	// Los colliders se crean con nombres al ser leidos de fichero
	_collidersHaveName = true;
}

//---------------------------------------------------------

void CPhysicDynamicEntity::onTrigger(IPhysics *otherComponent, bool enter) {
	// Mediante patron observador
	for(auto it = _observers.begin(); it != _observers.end(); ++it) {
		(*it)->onTrigger(otherComponent, enter);
	}

	// Construimos un mensaje de tipo TOUCHED o UNTOUCHED y lo enviamos a 
	// todos los componentes de la entidad. 

	if (enter) {
		_inTrigger=true;
		std::shared_ptr<CMessageTouched> m = std::make_shared<CMessageTouched>();
		m->setEntity(otherComponent->getEntity());
		_entity->emitMessage(m);
	} else {
		_inTrigger=false;
		std::shared_ptr<CMessageUntouched> m = std::make_shared<CMessageUntouched>();
		m->setEntity(otherComponent->getEntity());
		_entity->emitMessage(m);
	}
}

//---------------------------------------------------------

void CPhysicDynamicEntity::onContact(IPhysics *otherComponent, const Physics::CContactPoint& contactPoint, bool enter) {
	// Mediante patron observador
	for(auto it = _observers.begin(); it != _observers.end(); ++it) {
		(*it)->onContact(otherComponent, contactPoint, enter);
	}

	if (enter) {
		_inContact=true;
		std::shared_ptr<CMessageContactEnter> msg = std::make_shared<CMessageContactEnter>();
		msg->setEntity( otherComponent->getEntity() );
		msg->setContactPoint(contactPoint);
		_entity->emitMessage(msg);
	} else {
		_inContact=false;
		std::shared_ptr<CMessageContactExit> m = std::make_shared<CMessageContactExit>();
		m->setEntity(otherComponent->getEntity()->getEntityID());
		_entity->emitMessage(m);
	}
}

//---------------------------------------------------------

void CPhysicDynamicEntity::onShapeHit(IPhysics *otherComponent, const Vector3& colisionPos, const Vector3& colisionNormal) {
	// Mediante patron observador
	for(auto it = _observers.begin(); it != _observers.end(); ++it) {
		(*it)->onShapeHit(otherComponent, colisionPos, colisionNormal);
	}
}

//---------------------------------------------------------

void CPhysicDynamicEntity::move(const Vector3& disp) {
	_physicEntity.move(disp);
}

//---------------------------------------------------------

void CPhysicDynamicEntity::setPosition(const Vector3 &position, bool makeConversionToLogicWorld) {
	_physicEntity.setPosition(position, makeConversionToLogicWorld);
}
//---------------------------------------------------------

void CPhysicDynamicEntity::setGlobalPose(const Vector3 &position, const Quaternion &orientation, bool makeConversionToLogicWorld) {
	_physicEntity.setGlobalPose(position, orientation, makeConversionToLogicWorld);
}
//---------------------------------------------------------

void CPhysicDynamicEntity::addForce(const Vector3& force, Physics::ForceMode mode, bool autowake) {
	_physicEntity.addForce(force, mode, autowake);
}

//---------------------------------------------------------

void CPhysicDynamicEntity::clearForce(Physics::ForceMode mode, bool autowake) {
	_physicEntity.clearForce(mode,autowake);
}

//---------------------------------------------------------

void CPhysicDynamicEntity::addTorque(const Vector3& force, Physics::ForceMode mode, bool autowake) {
	_physicEntity.addTorque(force, mode, autowake);
}

//---------------------------------------------------------

void CPhysicDynamicEntity::deactivateSimulation() {
	_physicEntity.deactivateSimulation();
}

//---------------------------------------------------------

void CPhysicDynamicEntity::activateSimulation() {
	_physicEntity.activateSimulation();
}