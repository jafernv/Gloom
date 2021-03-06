/**
@file Light.cpp

Contiene la implementación del componente que controla la vida de una entidad.
 
@see Logic::CLight
@see Logic::IComponent

@author Antonio Jesus Narvaez
@author Francisco Aisa García
@date Julio, 2013
*/

#include "Light.h"

#include "Application/BaseApplication.h"
#include "Logic/LightManager.h"
#include "Map/MapEntity.h"

using namespace std;

namespace Logic {

	IMP_FACTORY(CLight);

	CLight::CLight() : _light(NULL),
					   _lightGroup(0),
					   _isOn(true),
					   _static(false),
					   _position(Vector3::ZERO),
					   _direction(Vector3::NEGATIVE_UNIT_Y),
					   _color(Vector3::ZERO),
					   _range(0.0f),
					   _attenuation(Vector3::ZERO),
					   _innerAngle(0.0f),
					   _outerAngle(0.0f) {
		
		// Nada que hacer
	}

	//________________________________________________________________________

	CLight::~CLight() {
		turnOff();
	}
	
	//________________________________________________________________________
	
	bool CLight::spawn(CEntity *entity, CMap *map, const Map::CEntity *entityInfo) {
		if( !IComponent::spawn(entity,map,entityInfo) ) return false;

		// ATRIBUTOS OBLIGATORIOS
		assert( entityInfo->hasAttribute("lightType") );

		std::string lightTypeString = entityInfo->getStringAttribute("lightType");
		if(lightTypeString == "directional")
			_lightType = Graphics::LightType::eDIRECTIONAL_LIGHT;
		else if(lightTypeString == "point")
			_lightType = Graphics::LightType::ePOINT_LIGHT;
		else
			_lightType = Graphics::LightType::eSPOT_LIGHT;

		assert( entityInfo->hasAttribute("controlledByManager") );
		_controlledByManager = entityInfo->getBoolAttribute("controlledByManager");

		if( entityInfo->hasAttribute("position") )
			_position = entityInfo->getVector3Attribute("position");
		else
			assert( !_controlledByManager );

		// ATRIBUTOS OPCIONALES
		if( entityInfo->hasAttribute("lightGroup") )
			_lightGroup = entityInfo->getIntAttribute("lightGroup");

		if( entityInfo->hasAttribute("lightIsStatic") )
			_static = entityInfo->getBoolAttribute("lightIsStatic");

		if( entityInfo->hasAttribute("on") )
			_isOn = entityInfo->getBoolAttribute("on");

		if( entityInfo->hasAttribute("direction") )
			_direction = entityInfo->getVector3Attribute("direction");

		if( entityInfo->hasAttribute("lightRotation") ) {
			Quaternion rotation = entityInfo->getQuaternionAttribute("lightRotation");
			_direction = -rotation.zAxis();
		}

		if( entityInfo->hasAttribute("color") )
			_color = entityInfo->getVector3Attribute("color");

		if( entityInfo->hasAttribute("attenuation") )
			_attenuation = entityInfo->getVector3Attribute("attenuation");

		// Si el rango es 0 o no se especifica entonces asumimos que el rango
		// es infinito. En caso contrario aplicamos el rango dado.
		// OJITO! Ogre usa el rango para dejar de renderizar las luces cuando no miran
		// a camara, si el rango no esta bien calculado es un canteo (sobretodo para
		// el calculo de difuso, el de especular da igual xq tiene que mirar
		// a la luz)
		if( entityInfo->hasAttribute("range") )
			_range = entityInfo->getFloatAttribute("range");

		if( entityInfo->hasAttribute("innerAngle") )
			_innerAngle = entityInfo->getFloatAttribute("innerAngle");

		if( entityInfo->hasAttribute("outerAngle") )
			_outerAngle = entityInfo->getFloatAttribute("outerAngle");

		return true;

	} // spawn

	//________________________________________________________________________

	bool CLight::accept(const std::shared_ptr<CMessage>& message) {
		TMessageType msgType = message->getMessageType();

		return msgType == Message::TOUCHED;
	} // accept

	//________________________________________________________________________

	void CLight::process(const std::shared_ptr<CMessage>& message) {
		switch( message->getMessageType() ) {
			case Message::TOUCHED: {
				CLightManager::getSingletonPtr()->createLight(_light, _lightType, _lightGroup, _static, this, _controlledByManager, _position, _direction);

				if(_light != NULL) {
					if( _color != Vector3::ZERO ) {
						_light->setColor(_color.x, _color.y, _color.z);
					}
					if( _attenuation != Vector3::ZERO ) {
						// Si el rango es 0, metemos como atenuacion "infinito"
						// porque sino Ogre automaticamente deja de renderizar la luz a esa distancia
						_light->setAttenuation(_range != 0.0f ? _range : 0xFFFFFFFF, _attenuation.x, _attenuation.y, _attenuation.z);
					}
					if( _innerAngle != 0.0f || _outerAngle != 0.0f ) {
						_light->setSpotLightParams(_innerAngle, _outerAngle);
					}
				}

				break;
			}
		}
	} // process

	//________________________________________________________________________

	void CLight::onStart() {
		if(_isOn) {
			CLightManager::getSingletonPtr()->createLight(_light, _lightType, _lightGroup, _static, this, _controlledByManager, _position, _direction);

			if(_light != NULL) {
				if( _color != Vector3::ZERO ) {
					_light->setColor(_color.x, _color.y, _color.z);
				}
				if( _attenuation != Vector3::ZERO ) {
					// De momento ignoramos el rango en los shaders
					_light->setAttenuation(_range != 0.0f ? _range : 0xFFFFFFFF, _attenuation.x, _attenuation.y, _attenuation.z);
				}
				if( _innerAngle != 0.0f || _outerAngle != 0.0f ) {
					_light->setSpotLightParams(_innerAngle, _outerAngle);
				}
			}
		}
	}

	//________________________________________________________________________

	void CLight::setColor(const Vector3& color) {
		this->_color = color;
		if(_light != NULL)
			_light->setColor(_color.x, _color.y, _color.z);
	}

	//________________________________________________________________________

	void CLight::setAttenuation(const Vector3& attenuation) {
		this->_attenuation = attenuation;
		if(_light != NULL)
			_light->setAttenuation(_range != 0.0f ? _range : 0xFFFFFFFF, _attenuation.x, _attenuation.y, _attenuation.z);
	}

	//________________________________________________________________________

	void CLight::setRange(float range) {
		this->_range = range;
		if(_light != NULL)
			_light->setAttenuation(_range != 0.0f ? _range : 0xFFFFFFFF, _attenuation.x, _attenuation.y, _attenuation.z);
	}

	//________________________________________________________________________

	void CLight::setSpotLightParams(float innerAngle, float outerAngle) {
		this->_innerAngle = innerAngle;
		this->_outerAngle = outerAngle;
		if(_light != NULL)
			_light->setSpotLightParams(_innerAngle, _outerAngle);
	}

	//________________________________________________________________________

	void CLight::setPosition(const Vector3& position) {
		this->_position = position;
		if(_light != NULL)
			_light->setPosition(position);
	}

	//________________________________________________________________________

	void CLight::setOrientation(const Quaternion& rotation) {
		this->_direction = -rotation.zAxis();
		if(_light != NULL)
			_light->setDirection(_direction);
	}

	//________________________________________________________________________

	void CLight::setDirection(const Vector3& direction) {
		this->_direction = direction;
		if(_light != NULL)
			_light->setDirection(direction);
	}

	//________________________________________________________________________

	void CLight::turnOff() {
		// Destruimos la luz
		CLightManager::getSingletonPtr()->destroyLight(_light, _controlledByManager);
		_light = NULL;
	}

} // namespace Logic


