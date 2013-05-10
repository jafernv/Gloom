/**
@file Components/Component.cpp

Contiene la implementación de la clase base de los componentes.
 
@see Logic::IComponent
@see Logic::CCommunicationPort

@author Francisco Aisa García
@date Abril, 2013
*/

#include "Component.h"
#include "Logic/Server.h"

namespace Logic {
	IComponent::IComponent() : _entity(0), 
							   _isActivated(false),
							   _deepSleep(false),
							   _wantsTick(true),
							   _state(ComponentState::eAWAKE),
							   _tickMask(TickMode::eTICK | TickMode::eFIXED_TICK) {

		// Espia de debug
		Logic::CServer::getSingletonPtr()->COMPONENT_CONSTRUCTOR_COUNTER += 1;
	}

	//__________________________________________________________________

	IComponent::~IComponent() {
		Logic::CServer::getSingletonPtr()->COMPONENT_DESTRUCTOR_COUNTER += 1;
	}

	//__________________________________________________________________

	bool IComponent::spawn(CEntity *entity, CMap *map, const Map::CEntity *entityInfo) {
		_entity = entity;
		return true;
	} // spawn

	//__________________________________________________________________
	
	//@deprecated -> estoy hay que implementarlo como dios manda
	void IComponent::activate() {
		if(_isActivated) return;
		
		// Por defecto no forzamos a que se arranque onStart
		// queda abierto para que el cliente lo haga
		// solo tendria que setear updater como onStart
		// al ejecutar onStart se vuelve a setear tick
		
		_isActivated = true;
		clearMessages();

		// Llamamos al activar que los hijos deben redefinir
		onActivate();
	}

	//__________________________________________________________________

	void IComponent::onActivate() {
		// Redefinir por los hijos
	}

	//__________________________________________________________________

	void IComponent::deactivate() {
		_isActivated = false;
		onDeactivate();
	}

	//__________________________________________________________________

	void IComponent::onDeactivate() {
		// Los hijos deben redefinir su comportamiento
	}

	//__________________________________________________________________

	void IComponent::onStart() {
		// Los hijos deben redefinir su comportamiento
	}

	//__________________________________________________________________

	void IComponent::onTick(unsigned int msecs) {
		_wantsTick = false;
		_tickMask = _tickMask & TickMode::eFIXED_TICK;
	}

	//__________________________________________________________________

	void IComponent::onFixedTick(unsigned int msecs) {
		_wantsTick = false;
		_tickMask = _tickMask & TickMode::eTICK;
	}

	//__________________________________________________________________

	void IComponent::putToSleep(bool deepSleep) {
		if(_state != ComponentState::eSLEEPING) {
			_state = ComponentState::eSLEEPING;
			this->_deepSleep = deepSleep;

			onSleep();
		}
	}

	//__________________________________________________________________

	void IComponent::onSleep() {
		// Los hijos deben redefinir su comportamiento
	}

	//__________________________________________________________________

	void IComponent::wakeUp() {
		if(_state == ComponentState::eSLEEPING) {
			_state = ComponentState::eAWAKE;

			onWake();
		}
	}

	//__________________________________________________________________

	void IComponent::onWake() {
		// Los hijos deben redefinir su comportamiento
	}

	//__________________________________________________________________

	void IComponent::stayBusy() {
		if(_state != ComponentState::eBUSY) {
			_state = ComponentState::eBUSY;

			onBusy();
		}
	}

	//__________________________________________________________________

	void IComponent::onBusy() {
		// Los hijos deben redefinir su comportamiento
	}

	//__________________________________________________________________

	void IComponent::stayAvailable() {
		if(_state == ComponentState::eBUSY) {
			_state = ComponentState::eAWAKE;

			onAvailable();
		}
	}

	//__________________________________________________________________

	void IComponent::onAvailable() {
		// Los hijos deben redefinir su comportamiento
	}

} // namespace Logic
