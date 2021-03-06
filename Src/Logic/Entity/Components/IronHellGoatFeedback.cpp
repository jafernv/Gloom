/**
@file IronHellGoat.cpp

Contiene la implementacion del componente
de disparo de la cabra.

@see Logic::CIronHellGoatFeedback
@see Logic::IWeaponFeedback

@author Francisco Aisa Garc�a
@date Mayo, 2013
*/

#include "IronHellGoatFeedback.h"
#include "HudWeapons.h"
#include "DynamicLight.h"

#include "Logic/Maps/EntityFactory.h"
#include "Logic/Maps/Map.h"
#include "Logic/Server.h"
#include "Map/MapEntity.h"

using namespace std;

namespace Logic {

	IMP_FACTORY(CIronHellGoatFeedback);

	//__________________________________________________________________

	CIronHellGoatFeedback::CIronHellGoatFeedback() : IWeaponFeedback("ironHellGoat") {
		// Nada que hacer
	}

	//__________________________________________________________________

	CIronHellGoatFeedback::~CIronHellGoatFeedback() {
		// Nada que hacer
	}

	//__________________________________________________________________

	bool CIronHellGoatFeedback::spawn(CEntity* entity, CMap *map, const Map::CEntity *entityInfo) {
		if( !IWeaponFeedback::spawn(entity, map, entityInfo) ) return false;

		return true;
	}

	//__________________________________________________________________

	void CIronHellGoatFeedback::primaryFire() {
		// Poner la animacion de carga inestable del arma
		_hudWeapon->loadingWeapon(true);
	}

	//__________________________________________________________________

	void CIronHellGoatFeedback::stopPrimaryFire() {
		// Parar la animacion de carga
		_hudWeapon->loadingWeapon(false);
		_hudWeapon->shootAnim(-1.85f);

		// Emitimos el sonido de lanzar la bola de fuego
		emitSound("weapons/ironHellGoat/shootFireBall2.wav", false, true, false, false);

		// Shoot flash
		CDynamicLight* shootFlash = _entity->getComponent<CDynamicLight>("CDynamicLight");
		shootFlash->setColor( Vector3(1.0f, 0.8f, 0.0f) );
		shootFlash->setAttenuation( Vector3(1.0f, 0.014f, 0.0007f) );
		shootFlash->setRange(325.0f);
		shootFlash->turnOn(Vector3(0.0f, _heightShoot, 0.0f), 0.1f);
	}

	//__________________________________________________________________

	void CIronHellGoatFeedback::secondaryFire() {
		// De momento n� pisha
	}

}//namespace Logic