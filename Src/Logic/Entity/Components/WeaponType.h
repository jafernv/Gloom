#ifndef __Logic_WeaponType_H
#define __Logic_WeaponType_H

#include <string>

namespace Logic {

	struct WeaponType {
		enum Enum {
			eHAMMER,
			eSNIPER,
			eSHOTGUN,
			eMINIGUN,
			eROCKET_LAUNCHER,
			eGRENADE_LAUNCHER
		};

		static std::string toString(WeaponType::Enum weaponType) {
			switch(weaponType) {
				case eHAMMER:
					return "hammer";
				case eSNIPER:
					return "sniper";
				case eSHOTGUN:
					return "shotgun";
				case eMINIGUN:
					return "minigun";
				case eROCKET_LAUNCHER:
					return "rocketLauncher";
				case eGRENADE_LAUNCHER:
					return "grenadeLauncher";

			}
		}
	};

}

#endif