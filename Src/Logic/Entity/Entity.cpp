/**
@file Entity.cpp

Contiene la implementaci�n de la clase Entity, que representa una entidad
de juego. Es una colecci�n de componentes.

@see Logic::CEntity
@see Logic::IComponent

@author David Llans�
@date Julio, 2010
*/
#include "Components\SpawnPlayer.h"

#include "Entity.h"

// Componentes
#include "Component.h"
#include "Logic/Server.h"
#include "Logic/Maps/Map.h"

#include "Logic/Entity/Components/Graphics.h"
#include "Logic/Entity/Components/ArrayGraphics.h"
#include "Map/MapEntity.h"

#include "Logic/Messages/MessageTransform.h"

#include "Input/Server.h"
#include "Input/PlayerController.h"

#include "../../Audio/Server.h"

namespace Logic 
{
	CEntity::CEntity(TEntityID entityID) : _entityID(entityID), 
				_map(0), _type(""), _name(""), _transform(Matrix4::IDENTITY),
				_isPlayer(false), _activated(false)
	{

	} // CEntity
	
	//---------------------------------------------------------

	CEntity::~CEntity()
	{
		assert(!_map && "��Antes de destruir la entidad debe desacoplarse del mapa!!");

		destroyAllComponents();

	} // ~CEntity
	
	//---------------------------------------------------------

	bool CEntity::dynamicSpawn(CMap* map, Map::CEntity* entityInfo) {
		Map::CEntity* newEntityInfo = entityInfo->clone(); 
		
		int entityId = getEntityID();
		std::ostringstream convert;
		convert << entityId;
		std::string nameId = convert.str();

		newEntityInfo->setName(entityInfo->getStringAttribute("name") + nameId);

		return spawn(map, newEntityInfo);
	}

	//---------------------------------------------------------

	bool CEntity::spawn(CMap *map, const Map::CEntity *entityInfo) 
	{
		// Leemos las propiedades comunes
		_map = map;
		_type = entityInfo->getType();

		if(entityInfo->hasAttribute("name")) {
			_name = entityInfo->getStringAttribute("name");
		}

		if(entityInfo->hasAttribute("position"))
		{
			Vector3 position = entityInfo->getVector3Attribute("position");
			_transform.setTrans(position);
		}

		// Por comodidad en el mapa escribimos los �ngulos en grados.
		if(entityInfo->hasAttribute("orientation"))
		{
			float yaw = Math::fromDegreesToRadians(entityInfo->getFloatAttribute("orientation"));
			Math::yaw(yaw,_transform);
		}

		if(entityInfo->hasAttribute("isPlayer"))
			_isPlayer = entityInfo->getBoolAttribute("isPlayer");
		

		// Inicializamos los componentes
		TComponentList::const_iterator it = _componentList.begin();

		bool correct = true;

		for(; it != _componentList.end() && correct; ++it )
			correct = (*it)->spawn(this,map,entityInfo) && correct;

		return correct;

	} // spawn

	//---------------------------------------------------------

	bool CEntity::activate() 
	{
		// Si somos jugador, se lo decimos al servidor
		// y nos registramos para que nos informen
		// de los movimientos que debemos realizar

		//Ademas como el sonido necesita saber la posici�n para actualizarse lo seteamos tambien

		if (isPlayer())
		{
			CServer::getSingletonPtr()->setPlayer(this);
			Audio::CServer::getSingletonPtr()->setSoundAvatar(this);
			Input::CServer::getSingletonPtr()->getPlayerController()->setControlledAvatar(this);
		}

		// Activamos los componentes
		TComponentList::const_iterator it = _componentList.begin();

		// Solo si se activan todos los componentes correctamente nos
		// consideraremos activados.
		_activated = true;

		for(; it != _componentList.end(); ++it){
			(*it)->activate();
			_activated = (*it)->isActivated() && _activated;
		}

		return _activated;

	} // activate

	//---------------------------------------------------------

	void CEntity::deactivate() 
	{
		// Si �ramos el jugador, le decimos al servidor que ya no hay.
		// y evitamos que se nos siga informando de los movimientos que 
		// debemos realizar
		if (isPlayer())
		{
			Input::CServer::getSingletonPtr()->getPlayerController()->setControlledAvatar(0);
			CServer::getSingletonPtr()->setPlayer(0);
		}


		TComponentList::const_iterator it = _componentList.begin();

		// Desactivamos los componentes
		for(; it != _componentList.end(); ++it)
			(*it)->deactivate();

		_activated = false;

	} // deactivate
	//---------------------------------------------------------

	
	void CEntity::deactivateAllComponentsExcept (std::vector<std::string> exceptionList) {
		TComponentMap::iterator it = _components.begin();

		bool desactivar=true;
		for(; it!=_components.end(); ++it){

			for(int i = 0; i < exceptionList.size(); ++i){
				if( exceptionList[i] == it->first )
					desactivar=false;
			}
			if(desactivar)
				(it->second)->deactivate();
			desactivar=true;
		}


	}// deactivateAllComponentsExcept

	//---------------------------------------------------------

	void CEntity::tick(unsigned int msecs) 
	{
		TComponentList::const_iterator it = _componentList.begin();

		IComponent* component;
		for(; it != _componentList.end(); ++it) {
			component = *it;
			if( component->isActivated() ) {
				// De momento se hace esta comprobaci�n extra para
				// ver si lanzamos onStart
				// Lo suyo es hacer colas distintas para evitar estas
				// comprobaciones tontas
				if( component->isStartingUp() ) {
					component->onStart(msecs);
				}
				else {
					component->tick(msecs);
				}
			}
		}

	} // tick

	//---------------------------------------------------------

	void CEntity::addComponent(IComponent* component, const std::string& id)
	{
		_components[id] = component;
		_componentList.push_back(component);

		component->setEntity(this);
	} // addComponent

	//---------------------------------------------------------

	bool CEntity::removeComponent(IComponent* component)
	{
		TComponentMap::const_iterator itMap = _components.begin();
		TComponentList::const_iterator itList = _componentList.begin();

		bool mapErase = false;
		for(; itMap != _components.end() && !mapErase; ++itMap) {
			if(itMap->second == component) {
				_components.erase(itMap);
				mapErase = true;
			}
		}

		bool listErase = false;
		for(; itList != _componentList.end() && !listErase; ++itList) {
			if(*itList == component) {
				_componentList.erase(itList);
				listErase = true;
			}
		}

		return mapErase && listErase;
	} // removeComponent

	//---------------------------------------------------------

	void CEntity::destroyAllComponents()
	{
		_components.clear();

		TComponentList::const_iterator it = _componentList.begin();
		for(; it != _componentList.end(); ++it) {
			delete *it;
		}

	} // destroyAllComponents

	//---------------------------------------------------------

	bool CEntity::emitMessage(CMessage *message, IComponent* emitter)
	{
		
		// Interceptamos los mensajes que adem�s de al resto de los
		// componentes, interesan a la propia entidad.
		switch(message->getMessageType())
		{
		case Message::SET_TRANSFORM:
			_transform = ((CMessageTransform*)message)->getTransform();
		}
		

		TComponentList::const_iterator it = _componentList.begin();
		//Por si nadie quiso el mensaje
		message->addSmartP();
		// Para saber si alguien quiso el mensaje.
		bool anyReceiver = false;
		for(; it != _componentList.end(); ++it)
		{
			// Al emisor no se le envia el mensaje y si esta desactivado el componente tampoco se le envia
			if( emitter != *it && (*it)->isActivated() )
				anyReceiver = (*it)->set(message) || anyReceiver;
		}
		//Por si nadie quiso el mensaje
		message->subSmartP();

		return anyReceiver;

	} // emitMessage

	//---------------------------------------------------------

	void CEntity::setTransform(const Matrix4& transform) 
	{
		_transform = transform;

		// Avisamos a los componentes del cambio.
		Logic::CMessageTransform *m=new Logic::CMessageTransform();
		m->setTransform(_transform);
		emitMessage(m);
	} // setTransform

	//---------------------------------------------------------

	void CEntity::setPosition(const Vector3 &position) 
	{
		_transform.setTrans(position);
		if(getType()=="Screamer")
			int a = 5;
		// Avisamos a los componentes del cambio.
		Logic::CMessageTransform *m=new Logic::CMessageTransform();
		m->setTransform(_transform);
		emitMessage(m);
		
	} // setPosition

	//---------------------------------------------------------

	void CEntity::setOrientation(const Matrix3& orientation) 
	{
		_transform = orientation;

		// Avisamos a los componentes del cambio.
		Logic::CMessageTransform *m=new Logic::CMessageTransform();
		m->setTransform(_transform);
		emitMessage(m);

	} // setOrientation

	//---------------------------------------------------------

	Matrix3 CEntity::getOrientation() const
	{
		Matrix3 orientation;
		_transform.extract3x3Matrix(orientation);
		return orientation;

	} // getOrientation

	//---------------------------------------------------------

	void CEntity::setYaw(float yaw) 
	{
		Math::setYaw(yaw,_transform);

		// Avisamos a los componentes del cambio.
		Logic::CMessageTransform *m=new Logic::CMessageTransform();
		m->setTransform(_transform);
		emitMessage(m);

	} // setYaw

	//---------------------------------------------------------

	void CEntity::yaw(float yaw) 
	{
		Math::yaw(yaw,_transform);

		// Avisamos a los componentes del cambio.
		Logic::CMessageTransform *m=new Logic::CMessageTransform();
		m->setTransform(_transform);
		emitMessage(m);
	} // yaw

	//---------------------------------------------------------

	void CEntity::setPitch(float pitch) 
	{
		Math::setYaw(pitch,_transform);

		// Avisamos a los componentes del cambio.
		Logic::CMessageTransform *m=new Logic::CMessageTransform();
		m->setTransform(_transform);
		emitMessage(m);
	} // setPitch

	//---------------------------------------------------------

	void CEntity::pitch(float pitch) 
	{
		Math::pitch(pitch,_transform);

		// Avisamos a los componentes del cambio.
		Logic::CMessageTransform *m=new Logic::CMessageTransform();
		m->setTransform(_transform);
		emitMessage(m);
	} // pitch

	//---------------------------------------------------------

	


} // namespace Logic
