/**
@file Shoot.cpp

Contiene la implementaci�n del componente que gestiona las armas y que administra el disparo.
 
@see Logic::CShoot
@see Logic::IComponent

*/

#include "WeaponsManagerClient.h"

#include "Logic/Entity/Entity.h"
#include "Map/MapEntity.h"
#include "Physics/Server.h"
#include "Graphics/Server.h"
#include "Graphics/Scene.h"
#include "Logic/Entity/Components/ArrayGraphics.h"
#include "Logic/Maps/WorldState.h"
#include "Logic/Maps/Map.h"

#include "WeaponAmmo.h"
/*#include "ShootShotGunAmmo.h"
#include "ShootSniperAmmo.h"
#include "ShootMiniGunAmmo.h"
#include "ShootSoulReaperAmmo.h"
#include "IronHellGoatAmmo.h"*/

#include "ShotGunAmmo.h"
#include "SniperAmmo.h"
#include "MiniGunAmmo.h"
#include "SoulReaperAmmo.h"
#include "IronHellGoatAmmo.h"

#include "Logic/Messages/MessageChangeWeapon.h"
#include "Logic/Messages/MessageChangeWeaponGraphics.h"
#include "Logic/Messages/MessageAddAmmo.h"
#include "Logic/Messages/MessageAddWeapon.h"

#include "Logic/Messages/MessageHudWeapon.h"
#include "Logic/Messages/MessageHudAmmo.h"

#include "Logic/Messages/MessageDamageAmplifier.h"
#include "Logic/Messages/MessageReducedCooldown.h"


namespace Logic 
{
	IMP_FACTORY(CWeaponsManagerClient);

	//---------------------------------------------------------
	
	CWeaponsManagerClient::CWeaponsManagerClient() : _currentWeapon(0), _coolDownTimeStamp(0), _amplifydamageTimeStamp(0) {
		
	}

	//---------------------------------------------------------

	bool CWeaponsManagerClient::spawn(CEntity *entity, CMap *map, const Map::CEntity *entityInfo) {
		if(!IComponent::spawn(entity,map,entityInfo)) return false;

		// Inicializamos el vector de armas.
		// De momento no tiene mucho sentido que comprobemos el n�mero de armas que hay
		// por que luego usamos el getComponent a pi�on.
		_weaponry.resize(WeaponType::eSIZE);

		// Rellenamos el vector con los punteros a los componentes correspondientes
		_weaponry[WeaponType::eSOUL_REAPER].second = _entity->getComponent<Logic::CSoulReaperAmmo>("CSoulReaperAmmo");
		_weaponry[WeaponType::eSNIPER].second = _entity->getComponent<Logic::CSniperAmmo>("CSniperAmmo");
		_weaponry[WeaponType::eSHOTGUN].second = _entity->getComponent<Logic::CShotGunAmmo>("CShotGunAmmo");
		_weaponry[WeaponType::eMINIGUN].second = _entity->getComponent<Logic::CMiniGunAmmo>("CMiniGunAmmo");
		_weaponry[WeaponType::eIRON_HELL_GOAT].second = _entity->getComponent<Logic::CIronHellGoatAmmo>("CIronHellGoatAmmo");


		_currentScene = _entity->getMap()->getScene();
		return true;

	} // spawn
	
	//---------------------------------------------------------

	void CWeaponsManagerClient::onActivate() {
		
		// El resto de las armas est�n desactivadas, ya que no las tenemos
		for(unsigned int i = 1; i < _weaponry.size(); ++i) {
			_weaponry[i].first = false; // Por si acaso habian sido activadas anteriormente
			_weaponry[i].second->stayBusy();
			_weaponry[i].second->resetAmmo();

		}

		// El arma actual tiene que ser el soulReaper, que
		// es la �nica que tenemos de primeras
		_currentWeapon=WeaponType::eSOUL_REAPER;
		// Por defecto la primera arma est� activada y equipadda
		_weaponry[WeaponType::eSOUL_REAPER].first = true;
		_weaponry[WeaponType::eSOUL_REAPER].second->stayAvailable();

		_currentScene->createCompositor("AmplifyDamageCompositor");
		_currentScene->setCompositorVisible("AmplifyDamageCompositor", false);
		_currentScene->createCompositor("ColdDownCompositor");
		_currentScene->setCompositorVisible("ColdDownCompositor", false);

	} // onActivate
	//---------------------------------------------------------

	void CWeaponsManagerClient::onDeactivate(){
		/*
		//reset de armas y balas
		for(unsigned int i = 0; i<_weaponry.size();++i){
			_weaponry[i].first = false;
			_weaponry[i].second->resetAmmo();
			_weaponry[i].second->inUse(false);
		}
		*/
		
	}// onDeactivate
	//---------------------------------------------------------

	bool CWeaponsManagerClient::accept(const std::shared_ptr<CMessage>& message) {
		Logic::TMessageType msgType = message->getMessageType();

		return msgType == Message::CHANGE_WEAPON
			|| msgType == Message::ADD_AMMO
			|| msgType == Message::ADD_WEAPON
			|| msgType == Message::REDUCED_COOLDOWN
			|| msgType == Message::DAMAGE_AMPLIFIER;
	} // accept
	
	//---------------------------------------------------------

	void CWeaponsManagerClient::process(const std::shared_ptr<CMessage>& message) {
		switch( message->getMessageType() ) {
			case Message::CHANGE_WEAPON: {
				std::shared_ptr<CMessageChangeWeapon> changeWeaponMsg = std::static_pointer_cast<CMessageChangeWeapon>(message);
				int iWeapon = changeWeaponMsg->getWeapon();

				//Si iWeapon no es scroll de rueda de rat�n hacia adelante o hacia atr�s, 
				//asignamos directamente el arma con el �ndice recibido
				if ((iWeapon != 100) && (iWeapon != -100)){
					changeWeapon(iWeapon);
				}
				else{
					//Obtenemos el �ndice del arma nueva al que se va a cambiar por el scroll
					iWeapon = selectScrollWeapon(iWeapon);
					if (iWeapon != -1)
						changeWeapon(iWeapon); //Si hemos obtenido arma, se la asignamos
				}
				break;
			}
			case Message::ADD_AMMO: {
				std::shared_ptr<CMessageAddAmmo> addAmmoMsg = std::static_pointer_cast<CMessageAddAmmo>(message);
				unsigned int weaponIndex = addAmmoMsg->getAddWeapon();
				_weaponry[weaponIndex].second->addAmmo(weaponIndex, addAmmoMsg->getAddAmmo(), _weaponry[weaponIndex].first);
				break;
			}
			case Message::ADD_WEAPON: {
				std::shared_ptr<CMessageAddWeapon> addWeaponMsg = std::static_pointer_cast<CMessageAddWeapon>(message);
				addWeapon( addWeaponMsg->getAddAmmo(), addWeaponMsg->getAddWeapon() );
				break;
			}
			case Message::REDUCED_COOLDOWN: {
				std::shared_ptr<CMessageReducedCooldown> reducedCdMsg = std::static_pointer_cast<CMessageReducedCooldown>(message);
				reduceCooldowns( reducedCdMsg->getPercentCooldown() );
				break;
			}
			case Message::DAMAGE_AMPLIFIER: {
				std::shared_ptr<CMessageDamageAmplifier> damageAmplifierMsg = std::static_pointer_cast<CMessageDamageAmplifier>(message);
				amplifyDamage( damageAmplifierMsg->getPercentDamage() );
				break;
			}
		}
	} // process
	
	//---------------------------------------------------------

	void CWeaponsManagerClient::changeWeapon(unsigned char newWeapon){

		//printf("\n%d\t%d",_currentWeapon, newWeapon);
		if(newWeapon >= WeaponType::eSIZE){
			return;
		}
		if( _weaponry[newWeapon].first && (newWeapon != _currentWeapon) && ( _weaponry[newWeapon].second->getAmmo() > 0 || newWeapon == WeaponType::eSOUL_REAPER ) )
		{
			// Indicamos que el arma actual ya no est� equipada
			// Desactivamos el componente Shoot del arma actual
			// e indicamos que ya no est� equipada
			//_weaponry[_currentWeapon].second->inUse(false);
			_weaponry[_currentWeapon].second->stayBusy();

			// Activamos el componente del nuevo arma que vamos
			// a equipar e indicamos que el arma est� equipada
			_weaponry[newWeapon].second->stayAvailable();
			//_weaponry[newWeapon].second->inUse(true);
			
			// Actualizamo el indice de arma
			_currentWeapon = newWeapon;
			
			// Mandamos un mensaje para actualizar el HUD
			std::shared_ptr<CMessageChangeWeaponGraphics> chgWpnGraphicsMsg = std::make_shared<CMessageChangeWeaponGraphics>();
			chgWpnGraphicsMsg->setWeapon(_currentWeapon);
			_entity->emitMessage(chgWpnGraphicsMsg);
		}
		
	}

	//---------------------------------------------------------

	void CWeaponsManagerClient::amplifyDamage(int percentage) {
		// Amplificamos el da�o de todas las armas en base al porcentaje dado
		for(unsigned int i = 0; i < _weaponry.size(); ++i) {
			_weaponry[i].second->amplifyDamage(percentage);
		}

		if(!percentage){
			_currentScene->setCompositorVisible("AmplifyDamageCompositor", false);
		}else{
			_currentScene->setCompositorVisible("AmplifyDamageCompositor", true);
			_amplifydamageTimeStamp = 1500; 
		}
	}

	//---------------------------------------------------------

	void CWeaponsManagerClient::reduceCooldowns(int percentage) {
		// Reducimos el cooldown de todas las armas en base al porcentaje dado
		for(unsigned int i = 0; i < _weaponry.size(); ++i) {
			_weaponry[i].second->reduceCooldown(percentage);
		}

		if(!percentage){
			_currentScene->setCompositorVisible("ColdDownCompositor", false);
		}else{
			_currentScene->setCompositorVisible("ColdDownCompositor", true);
			_coolDownTimeStamp = 1500;
		}
	}

	//---------------------------------------------------------

	void CWeaponsManagerClient::addWeapon(int ammo, int weaponIndex){
		// Si el arma dada no la teniamos, indicamos que ahora la tenemos
		if(weaponIndex < WeaponType::eSIZE && !_weaponry[weaponIndex].first)
			_weaponry[weaponIndex].first = true;

		// Activamos el componente pero indicamos que
		// no es el arma equipada.
		if(_currentWeapon != weaponIndex){
			_weaponry[weaponIndex].second->stayBusy();
//_weaponry[weaponIndex].second->inUse( false );
		}
		/*
		else{
			//_weaponry[weaponIndex].second->stayAvailable();
			//_weaponry[weaponIndex].second->inUse( true );
		}
		*/
		
		// El arma estara en uso si es la actual, si no estara sin uso
		
		_weaponry[weaponIndex].second->addAmmo(weaponIndex, ammo, _weaponry[weaponIndex].first);

		/*
		// Enviamos un mensaje de actualizacion del hud
		std::shared_ptr<CMessageHudAmmo> *m=std::make_shared<CMessageHudAmmo>();
		m->setWeapon(weaponIndex);
		m->setAmmo(ammo);//No es necesario esto, ya que solo actualizare el hud como que puedo coger el arma pero no mostrara sus balas(en este caso concreto)
		_entity->emitMessage(m);
		*/

		if(_currentWeapon == WeaponType::eSOUL_REAPER){
			changeWeapon(weaponIndex);
		}
	}

	//---------------------------------------------------------

	int CWeaponsManagerClient::selectScrollWeapon(int iWeapon) 
	{
		if (iWeapon == 100) //Armas siguientes
		{
			//Recorremos todas las armas del inventario desde la actual hasta el final
			for (int i = _currentWeapon + 1; i < _weaponry.size(); ++i)
			{
				//Comprobamos si en ese �ndice tenemos arma
				//if (_weaponry[i].first && ( _weaponry[i].second->getAmmo() > 0 || i == WeaponType::eSOUL_REAPER ) )
				if( _weaponry[i].first && (i != _currentWeapon) && ( _weaponry[i].second->getAmmo() > 0 || i == WeaponType::eSOUL_REAPER ) )
					return i;
			}
			/*
			//Recorremos todas las armas del inventario desde el principio, para hacerlo circular
			for (int i = WeaponType::eSOUL_REAPER; i <= _currentWeapon; ++i)
			{
				//Comprobamos si en ese �ndice tenemos arma y tiene balas
				if (_weaponry[i].first && ( _weaponry[i].second->getAmmo() > 0 || i == WeaponType::eSOUL_REAPER ) )
					return i;
			}*/
		}
		else //iWeapon == -100 Armas anteriores
		{
			//Recorremos todas las armas del inventario desde la actual hasta el principio
			for (int i = _currentWeapon - 1; i >= 0; --i)
			{
				//Comprobamos si en ese �ndice tenemos arma y tiene balas
				//if (_weaponry[i].first)
				if( _weaponry[i].first && (i != _currentWeapon) && ( _weaponry[i].second->getAmmo() > 0 || i == WeaponType::eSOUL_REAPER ) )
					return i;
			}

			/*for (int i = WeaponType::eSIZE - 1 ; i > _currentWeapon; --i)
			{
				//Comprobamos si en ese �ndice tenemos arma
				if (_weaponry[i].first && ( _weaponry[i].second->getAmmo() > 0 || i == WeaponType::eSOUL_REAPER ))
					return i;
			}*/
		}

		return -1; //No hemos obtenido arma
	}//selectScrollWeapon

	void CWeaponsManagerClient::onTick(unsigned int msecs) {

		if(_coolDownTimeStamp > 0){
			_coolDownTimeStamp -= msecs;
			_currentScene->updateCompositorVariable("AmplifyDamageCompositor", "strength", _coolDownTimeStamp*0.01);
			if(_coolDownTimeStamp <= 0)
				_coolDownTimeStamp = 100;
		}

		if(_amplifydamageTimeStamp > 0){
			_amplifydamageTimeStamp -= msecs;
			_currentScene->updateCompositorVariable("ColdDownCompositor", "strength", _amplifydamageTimeStamp*0.01);
			if(_amplifydamageTimeStamp <= 0)
				_amplifydamageTimeStamp = 100;
		}
		
	}

} // namespace Logic

