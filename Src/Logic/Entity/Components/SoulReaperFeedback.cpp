/**
@file SoulReaper.cpp

Contiene la implementacion del componente
de disparo de la cabra.

@see Logic::CSoulReaperFeedback
@see Logic::IWeaponFeedback

@author Francisco Aisa Garc�a
@date Mayo, 2013
*/

#include "SoulReaperFeedback.h"
#include "HudWeapons.h"

#include "Audio/Server.h"
#include "Logic/Maps/EntityFactory.h"
#include "Logic/Maps/Map.h"
#include "Logic/Server.h"
#include "Map/MapEntity.h"

using namespace std;

namespace Logic {

	IMP_FACTORY(CSoulReaperFeedback);

	//__________________________________________________________________

	CSoulReaperFeedback::CSoulReaperFeedback() : IWeaponFeedback("soulReaper") {
		// Nada que hacer
	}

	//__________________________________________________________________

	CSoulReaperFeedback::~CSoulReaperFeedback() {
		// Nada que hacer
	}

	//__________________________________________________________________

	bool CSoulReaperFeedback::spawn(CEntity* entity, CMap *map, const Map::CEntity *entityInfo) {
		if( !IWeaponFeedback::spawn(entity, map, entityInfo) ) return false;

		return true;
	} // spawn

	//__________________________________________________________________

	void CSoulReaperFeedback::primaryFire() {
		// Animacion de golpe con el soul Reaper
		_hudWeapon->shootAnim(5.0f);

		//Sonido
		//Audio::CServer::getSingletonPtr()->playSound("weapons/soulReaper/hammer.wav", false, false);
	} // primaryFire

	//__________________________________________________________________

	void CSoulReaperFeedback::secondaryFire() {
		_hudWeapon->linking(true);
	}

	//__________________________________________________________________

	void CSoulReaperFeedback::stopSecondaryFire() {
		_hudWeapon->linking(false);
	}

}//namespace Logic