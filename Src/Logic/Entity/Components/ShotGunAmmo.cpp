/**
@file ShotGun.cpp

Contiene la implementacion del componente
de disparo de la cabra.

@see Logic::CShotGunAmmo
@see Logic::IWeaponAmmo

@author Francisco Aisa Garc�a
@date Mayo, 2013
*/

#include "ShotGunAmmo.h"
#include "FireBallController.h"
#include "HudWeapons.h"

#include "Logic/Maps/EntityFactory.h"
#include "Logic/Maps/Map.h"
#include "Logic/Server.h"
#include "Map/MapEntity.h"

#include "ShotGun.h"
#include "ShotGunFeedback.h"
#include "Logic/Messages/MessageAudio.h"

using namespace std;

namespace Logic {

	IMP_FACTORY(CShotGunAmmo);

	//__________________________________________________________________

	CShotGunAmmo::CShotGunAmmo() : IWeaponAmmo("shotGun"),
									_primaryFireCooldown(0),
									_defaultPrimaryFireCooldown(0),
									_primaryFireCooldownTimer(0),
									_primaryFireDispersion(0),
									_numberOfShots(0),
									_shotGunComponent(0){
		// Nada que hacer
	}

	//__________________________________________________________________

	CShotGunAmmo::~CShotGunAmmo() {
		// Nada que hacer
	}

	//__________________________________________________________________

	bool CShotGunAmmo::spawn(CEntity* entity, CMap *map, const Map::CEntity *entityInfo) {
		Map::CEntity* weapon = CEntityFactory::getSingletonPtr()->getInfo(_weaponName);

		if( !IWeaponAmmo::spawn(entity, map, weapon) ) return false;

		// Nos aseguramos de tener todos los atributos que necesitamos
		assert( weapon->hasAttribute("PrimaryFireCooldown") );
		assert( weapon->hasAttribute("PrimaryFireDispersion") );
		assert( weapon->hasAttribute("NumberOfShots") );
		

		// Cooldown del disparo principal
		_defaultPrimaryFireCooldown = _primaryFireCooldown = weapon->getFloatAttribute("PrimaryFireCooldown") * 1000;

		_primaryFireDispersion = weapon->getFloatAttribute("PrimaryFireDispersion");
		_numberOfShots = weapon->getIntAttribute("NumberOfShots");
		

		 _friend[_friends] = _shotGunComponent = _entity->getComponent<Logic::CShotGun>("CShotGun");
		if(_friend[_friends]) ++_friends;
		_friend[_friends] = _entity->getComponent<Logic::CShotGunFeedback>("CShotGunFeedback");
		if(_friend[_friends]) ++_friends;
		if(_friends == 0) assert("\nTiene que tenes alguno de los dos componentes");

		return true;
	}

	//__________________________________________________________________

	void CShotGunAmmo::onActivate() {
		//Reiniciamos el cooldown
		reduceCooldown(0);
	}

	//__________________________________________________________________

	void CShotGunAmmo::onAvailable() {
		IWeaponAmmo::onAvailable();
		
	}

	//__________________________________________________________________

	void CShotGunAmmo::onTick(unsigned int msecs) {
	
		// Controlamos el cooldown
		if(_primaryFireCooldownTimer > 0) {
			_primaryFireCooldownTimer -= msecs;
			
			if(_primaryFireCooldownTimer < 0)
				_primaryFireCooldownTimer = 0;
		}
	}

	//__________________________________________________________________

	bool CShotGunAmmo::canUsePrimaryFire() {
		if(_currentAmmo==0)
			emitSound(_noAmmoSound, false, true, false, false);
		return _primaryFireCooldownTimer == 0 && _currentAmmo > 0;
	}

	//__________________________________________________________________

	bool CShotGunAmmo::canUseSecondaryFire() {
		return true;
	}

	//__________________________________________________________________

	void CShotGunAmmo::primaryFire() {
		IWeaponAmmo::primaryFire();

		_primaryFireCooldownTimer = _primaryFireCooldown;

		int shots = _numberOfShots <= _currentAmmo ? _numberOfShots : _currentAmmo;
		for(int i = 0; i < shots; ++i) {
			decrementAmmo();
		}
	}

	//__________________________________________________________________

	void CShotGunAmmo::stopPrimaryFire() {
		IWeaponAmmo::stopPrimaryFire();
	}

	//__________________________________________________________________

	void CShotGunAmmo::addAmmo(int weapon, int ammo, bool iAmCatch){
		IWeaponAmmo::addAmmo(weapon, ammo, iAmCatch);

		if(_shotGunComponent)
			_shotGunComponent->setCurrentAmmo(_currentAmmo);
	}

	void CShotGunAmmo::reduceCooldown(unsigned int percentage) {
		// Si es 0 significa que hay que restaurar al que habia por defecto,
		// sino decrementamos conforme al porcentaje dado.
		_primaryFireCooldown = percentage == 0 ? _defaultPrimaryFireCooldown : (_defaultPrimaryFireCooldown - (percentage * _primaryFireCooldown * 0.01f));
	}

}//namespace Logic