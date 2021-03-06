#include "MessageChangeWeaponGraphics.h"

#include "Logic/Entity/MessageFactory.h"

#include <string>

namespace Logic {

	IMP_FACTORYMESSAGE(CMessageChangeWeaponGraphics);

	CMessageChangeWeaponGraphics::CMessageChangeWeaponGraphics() : CMessage(Message::CHANGE_WEAPON_GRAPHICS) {
		// Nada que hacer
	}//
	//----------------------------------------------------------
	void CMessageChangeWeaponGraphics::setWeapon(int weapon){
		_weapon = weapon;
	}//
	//----------------------------------------------------------
	int CMessageChangeWeaponGraphics::getWeapon(){
		return _weapon;
	}//
	//----------------------------------------------------------
	Net::CBuffer CMessageChangeWeaponGraphics::serialize() {
		Net::CBuffer buffer(sizeof(int) + sizeof(_weapon));
		buffer.serialize(std::string("CMessageChangeWeaponGraphics"),true);
		buffer.serialize(_weapon);
		
		return buffer;
	}//
	//----------------------------------------------------------

	void CMessageChangeWeaponGraphics::deserialize(Net::CBuffer& buffer) {
		buffer.deserialize(_weapon);
	}

};
