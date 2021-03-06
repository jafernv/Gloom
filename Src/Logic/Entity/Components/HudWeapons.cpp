/**
@file Graphics.cpp

Contiene la implementaci�n del componente que controla la representaci�n
gr�fica de la entidad.
 
@see Logic::CGraphics
@see Logic::IComponent

@author David Llans�
@date Agosto, 2010
*/

#include "HudWeapons.h"
#include "CameraFeedbackNotifier.h"

#include "BaseSubsystems/Euler.h"

#include "Audio/Server.h"

// Logica
#include "Logic/Entity/Entity.h"
#include "Logic/Maps/Map.h"
#include "Logic/Maps/EntityFactory.h"
#include "Map/MapEntity.h"

// Graficos
#include "Graphics/Camera.h"
#include "Graphics/Server.h"
#include "Graphics/Scene.h"
#include "Graphics/Entity.h"
#include "Graphics/StaticEntity.h"
#include "Graphics/Overlay.h"

// Mensajes
#include "Logic/Messages/MessageTransform.h"
#include "Logic/Messages/MessageChangeWeaponGraphics.h"
#include "Logic/Messages/MessageHudDebugData.h"
#include "Logic/Messages/MessageBlockShoot.h"

#include <stdio.h>
#include <windows.h>


using namespace std;

namespace Logic {
	IMP_FACTORY(CHudWeapons);
	
	//________________________________________________________________________
	
	CHudWeapons::CHudWeapons() : _currentWeapon(0), 
								 _graphicsEntities(0),
								 _changingWeapon(false),
								 _playerIsWalking(false),
								 _playerIsLanding(false),
								 _loadingWeapon(false),
								 _continouslyShooting(false),
								 _linking(false),
								 _threePiQuarters(3 * Math::PI / 4.0f),
								 _playerIsFalling(false) {

		// Valores de configuracion de la animacion de linkar
		_linkAnim.sinOffset = 0.05f;
		_linkAnim.x = 0.0f;
		_linkAnim.xSpeed = 0.028f;
		_linkAnim.offset = Vector3::ZERO;

		// Valores de configuracion de la animacion de correr
		_runAnim.currentHorizontalPos = Math::HALF_PI;
		_runAnim.horizontalSpeed = 0.0075f;
		_runAnim.horizontalOffset = 0.055f;

		_runAnim.currentVerticalPos = Math::HALF_PI;
		_runAnim.verticalSpeed = 2 * _runAnim.horizontalSpeed;
		_runAnim.verticalOffset = 0.055f;

		_runAnim.currentStrafingDir = _runAnim.oldStrafingDir = 0;
		
		_runAnim.offset = Vector3::ZERO;

		// Valores de configuracion de la animacion de aterrizar
		_landAnim.force = 0.0f;
		_landAnim.currentOffset = 0.0f;
		_landAnim.recoverySpeed = 0.01f;
		_landAnim.offset = Vector3::ZERO;

		// Valores de configuraci�n de la animaci�n de idle
		_idleAnim.currentVerticalPos = 0.0f;
		_idleAnim.verticalSpeed = 0.003f;
		_idleAnim.verticalOffset = 0.04f;

		_idleAnim.offset = Vector3::ZERO;

		// Valores de configuraci�n de la animaci�n de disparo
		_shootAnim.shootRecoveryCoef = 0.96f;
		_shootAnim.offset = Vector3::ZERO;

		// Valores de configuraci�n de la animaci�n de carga del arma
		_unstableLoadAnim.currentVerticalPos = 0.0f;
		_unstableLoadAnim.currentVerticalSpeed = _unstableLoadAnim.initVerticalSpeed = 0.01f;
		_unstableLoadAnim.maxVerticalSpeed = 0.035f;
		//_unstableLoadAnim.speedInc = 0.000035f;
		
		_unstableLoadAnim.verticalOffset = 0.05f;

		_unstableLoadAnim.currentNoise = _unstableLoadAnim.initNoiseSpeed = 0.0f;
		_unstableLoadAnim.maxNoiseSpeed = 0.1f;

		_unstableLoadAnim.offset = Vector3::ZERO;

		// Valores de configuracion de la animacion de disparo rapido
		_rapidShootAnim.shakeOffset = 0.1f;
		_rapidShootAnim.recoveryCoef = 0.96f;

		_rapidShootAnim.currentVerticalPos = 0.0f;
		_rapidShootAnim.verticalOffset = 0.05f;
		_rapidShootAnim.verticalSpeed = 0.025f;

		_rapidShootAnim.offset = Vector3::ZERO;

		_rapidShootAnim.firingRate = 20;
		_rapidShootAnim.acumFiringTime = 0;

		// Valores de configuracion de la animacion de cambio de arma
		_chgWpnAnim.sineOffset = 0.5f;
		_chgWpnAnim.xSpeed = 0.022f;
		_chgWpnAnim.x = 0.0f;
		_chgWpnAnim.horizontalSpeed = 0.8f;
		_chgWpnAnim.offset = Vector3::ZERO;
		_chgWpnAnim.takingAway = false;

		// Valores de configuraci�n de la animaci�n de salto
		_fallAnim.currentHorizontalPos = 0.0f;
		_fallAnim.horizontalSpeed = 0.0002f;
		_fallAnim.horizontalOffset = 0.2f;
		_fallAnim.movementDir = 0;
		
		_fallAnim.offset = Vector3::ZERO;
		_halfPi= Quaternion(Ogre::Radian(Math::HALF_PI),Vector3::UNIT_Y);
	}

	//________________________________________________________________________

	CHudWeapons::~CHudWeapons() {
		if(_graphicsEntities) {
			for(int i = 0; i < WeaponType::eSIZE; ++i) {
				//_scene->removeChild(_graphicsEntities[i]._graphicsEntity);
				delete _graphicsEntities[i].graphicsEntity;
				_graphicsEntities[i].graphicsEntity = NULL;

				//delete _overlayWeapon3D[i];
			}

			delete _graphicsEntities;
		}

	} // ~CGraphics

	//________________________________________________________________________
	
	void CHudWeapons::onActivate() {
		//Cuando activamos el componente solo tendremos visible el arma 0( arma melee)
		_currentWeapon = WeaponType::eSOUL_REAPER;
		_graphicsEntities[_currentWeapon].graphicsEntity->setVisible(true);
		//_overlayWeapon3D[_currentWeapon]->setVisible(true);
	} // activate

	//________________________________________________________________________

	void CHudWeapons::onDeactivate() {
		//Cuando desactivamos el componente, desactivaremos el arma actual
		//_overlayWeapon3D[_currentWeapon]->setVisible(false);
		_graphicsEntities[_currentWeapon].graphicsEntity->setVisible(false);

		_fallAnim.movementDir = _runAnim.currentStrafingDir = 0;
		_playerIsWalking = _playerIsLanding = _playerIsFalling = false;
	} // deactivate

	//________________________________________________________________________

	Graphics::CEntity* CHudWeapons::getCurrentWeapon() {
		return _graphicsEntities[_currentWeapon].graphicsEntity;
	}

	//________________________________________________________________________

	bool CHudWeapons::spawn(CEntity *entity, CMap *map, const Map::CEntity *entityInfo) {
		if(!IComponent::spawn(entity,map,entityInfo))
			return false;
		
		_scene = _entity->getMap()->getScene();
			
		_graphicsEntities = new TGraphicsWeapon[WeaponType::eSIZE];
		
		// Por ahora leo a mano cada una de las armas que tiene el usuario
	
		for(int i = WeaponType::eSOUL_REAPER; i < WeaponType::eSIZE; ++i){
				
			WeaponType::Enum current = (WeaponType::Enum)i;
			std::string strWeapon = WeaponType::toString(current);

			Map::CEntity* weapon = CEntityFactory::getSingletonPtr()->getInfo(strWeapon);
				
			//_graphicsEntities[i]._graphicsEntity = createGraphicsEntity(weapon, entityInfo->getStringAttribute(weapon+"Model"));
			_graphicsEntities[current].defaultYaw = _graphicsEntities[current].defaultPitch = _graphicsEntities[current].defaultRoll = 0;
			if(weapon->hasAttribute("ModelYaw"))
				_graphicsEntities[current].defaultYaw = weapon->getFloatAttribute("ModelYaw");
			if(weapon->hasAttribute("ModelPitch"))
				_graphicsEntities[current].defaultPitch = weapon->getFloatAttribute("ModelPitch");
			if(weapon->hasAttribute("ModelRoll"))
				_graphicsEntities[current].defaultRoll = weapon->getFloatAttribute("ModelRoll");
				
			//Esto puede petar si no esta, pero creo q es obligatorio
			if(!weapon->hasAttribute("Offset"))
				assert("seguro que no tiene offset?");

			_graphicsEntities[current].offset = weapon->getVector3Attribute("Offset");
				
			/*
			// Ahora voy a crear los overlays por cada arma en 3D

			Graphics::CServer *server = Graphics::CServer::getSingletonPtr();

			_overlayWeapon3D[current] = server->createOverlay( "_overlay3D"+strWeapon, _scene );
			std::string modelWeapon = weapon->getStringAttribute("Model");	
			
			_graphicsEntities[current].graphicsEntity = _overlayWeapon3D[current]->add3D(strWeapon, modelWeapon,_graphicsEntities[current].offset);
			assert(_graphicsEntities[current].graphicsEntity != 0 && "error al cargar la entidad grafica");
			//_weaponsEntities[current] = _overlayWeapon3D[current]->add3D(currentOnText, modelWeapon, &offsetPositionWeapon);

			_graphicsEntities[current].graphicsEntity->setOrientation(Math::setQuaternion(_graphicsEntities[current].defaultYaw, _graphicsEntities[current].defaultPitch, _graphicsEntities[current].defaultRoll));
	

			_overlayWeapon3D[current]->setVisible(false);
			_overlayWeapon3D[current]->setZBuffer(15);
			/*/

			std::string modelWeapon = weapon->getStringAttribute("Model");	
			_graphicsEntities[current].graphicsEntity = _scene->getCamera()->addEntityChild(strWeapon, modelWeapon, _graphicsEntities[current].offset);
			// Este render queue se usa para simular los overlays. Poner en esta cola solo lo que queramos
			// que se renderice como un overlay!!
			_graphicsEntities[current].graphicsEntity->setRenderQueue(95);
			// Esta mascara se utiliza para excluir el elemento del efecto de motion blur
			_graphicsEntities[current].graphicsEntity->setVisibilityMask(1 << 0);

			_graphicsEntities[current].graphicsEntity->setOrientation(Math::setQuaternion(_graphicsEntities[current].defaultYaw, _graphicsEntities[current].defaultPitch, _graphicsEntities[current].defaultRoll));
			_graphicsEntities[current].graphicsEntity->setVisible(false);
			/* */
			
		}

		//_overlayWeapon3D[WeaponType::eSOUL_REAPER]->setVisible(true);
		_graphicsEntities[WeaponType::eSOUL_REAPER].graphicsEntity->setVisible(true);

		if(!_graphicsEntities)
			return false;
		
		// Usamos un peque�o truco para calcular a la velocidad a la que tiene que incrementar
		// el ruido de carga
		// Primero obtenemos el tiempo m�ximo de carga del Iron Hell Goat
		Map::CEntity* info = CEntityFactory::getSingletonPtr()->getInfo("ironHellGoat");
		assert( info->hasAttribute("PrimaryFireLoadTime") );

		// Una vez conocido el tiempo de carga, como sabemos que vamos a utilizar fixed ticks
		// de 16 msecs, calculamos cuantos ticks van a pasar (aproximadamente) hasta que se
		// tiene el arma cargada.
		unsigned int nbTicks = (info->getIntAttribute("PrimaryFireLoadTime") * 1000) / 16;

		// Calculamos el incremento de la velocidad distribuyendola uniformemente entre los
		// ticks de carga
		_unstableLoadAnim.speedInc = (_unstableLoadAnim.maxVerticalSpeed - _unstableLoadAnim.initVerticalSpeed) /  (float)nbTicks;
		_unstableLoadAnim.noiseInc = (_unstableLoadAnim.maxNoiseSpeed - _unstableLoadAnim.initNoiseSpeed) / (float)nbTicks;

		return true;

	} // spawn

	//________________________________________________________________________

	void CHudWeapons::onStart() {
		for(int i = 0; i < WeaponType::eSIZE; ++i) {
			_graphicsEntities[i].defaultPos = _graphicsEntities[i].graphicsEntity->getPosition();
			_graphicsEntities[i].graphicsEntity->setVisible(false);
		}
		_graphicsEntities[WeaponType::eSOUL_REAPER].graphicsEntity->setVisible(true);

		_avatarController = _entity->getComponent<CAvatarController>("CAvatarController");
		assert(_avatarController != NULL && "Error: Se necesita el componente avatar controller");

		_avatarController->addObserver(this);

		float cameraLandTime = Math::PI / _entity->getComponent<CCameraFeedbackNotifier>("CCameraFeedbackNotifier")->getLandRecoverySpeed();
		_landAnim.recoverySpeed = Math::PI / cameraLandTime;
	}
	
	//________________________________________________________________________

	bool CHudWeapons::accept(const std::shared_ptr<CMessage>& message) {
		Logic::TMessageType msgType = message->getMessageType();

		return msgType == Message::SET_TRANSFORM				||
			   msgType == Message::CHANGE_WEAPON_GRAPHICS;
	} // accept
	
	//________________________________________________________________________

	void CHudWeapons::process(const std::shared_ptr<CMessage>& message) {
		switch( message->getMessageType() ) {
			case Message::CHANGE_WEAPON_GRAPHICS: {
				std::shared_ptr<CMessageChangeWeaponGraphics> chgWpnMsg = static_pointer_cast<CMessageChangeWeaponGraphics>(message);
				changeWeapon( chgWpnMsg->getWeapon() );
				break;
			}
		}
	} // process

	//________________________________________________________________________

	void CHudWeapons::onLand() {
		float hitForce = _avatarController->getMomentum().y;
		if(hitForce < -0.3f) {
			_playerIsLanding = true;
			_landAnim.force = hitForce * 0.2f;
		}

		_fallAnim.movementDir = 0;
		_playerIsFalling = false;
	}

	//________________________________________________________________________

	void CHudWeapons::onWalk() {
		_playerIsWalking = true;
	}

	//________________________________________________________________________

	void CHudWeapons::onIdle() {
		_playerIsWalking = false;
		_runAnim.currentStrafingDir = 0;
	}

	//________________________________________________________________________

	void CHudWeapons::onAir() {
		_playerIsWalking = false;
		_playerIsFalling = true;
		_runAnim.currentStrafingDir = 0;
		_fallAnim.movementDir = _avatarController->getDisplacementDir().x;
	}
	
	//________________________________________________________________________

	void CHudWeapons::changeWeapon(int newWeapon) {
		_linking = _continouslyShooting = _loadingWeapon = false;

		shared_ptr<CMessageBlockShoot> lockWeaponsMsg = make_shared<CMessageBlockShoot>();
		lockWeaponsMsg->canShoot(false);
		_entity->emitMessage(lockWeaponsMsg);

		_chgWpnAnim.nextWeapon = newWeapon;

		_changingWeapon = true;
		_chgWpnAnim.x = 0.0f;
		_chgWpnAnim.takingAway = true;

		Audio::CServer::getSingletonPtr()->playSound("weapons/change.wav");
	}

	//________________________________________________________________________

	void CHudWeapons::onFixedTick(unsigned int msecs) {
		//_graphicsEntities[_currentWeapon].graphicsEntity->setVisible(true);
		if(_changingWeapon) {
			changeWeaponAnim(msecs);
		}
		else if(_playerIsLanding)
			landAnim(msecs);
		else if(_linking)
			linkAnim(msecs);
		else if(_playerIsWalking)
			walkAnim(msecs);
		else if(_playerIsFalling) {
			fallAnim(msecs);
		}
		else
			idleAnim(msecs);

		if(_loadingWeapon)
			loadWeaponAnim(msecs);
		else {
			_unstableLoadAnim.currentVerticalPos *= 0.95f;
			_unstableLoadAnim.currentNoise *= 0.95f;
			_unstableLoadAnim.offset *= 0.95f;
		}

		if(_continouslyShooting) {
			rapidShootAnim(msecs);
		}
		else {
			_rapidShootAnim.offset *= _rapidShootAnim.recoveryCoef;
			_rapidShootAnim.currentVerticalPos *= _rapidShootAnim.recoveryCoef;
		}

		_graphicsEntities[_currentWeapon].graphicsEntity->setPosition( _graphicsEntities[_currentWeapon].defaultPos + 
																	   _linkAnim.offset +
																	   _chgWpnAnim.offset +
																	   _runAnim.offset + 
																	   _landAnim.offset +
																	   _idleAnim.offset +
																	   _shootAnim.offset +
																	   _unstableLoadAnim.offset +
																	   _rapidShootAnim.offset +
																	   _fallAnim.offset );
		_shootAnim.offset *= _shootAnim.shootRecoveryCoef;

		if(!_playerIsFalling)
			_fallAnim.offset *= 0.96f;
		if(!_linking) {
			_linkAnim.offset *= 0.95f;
			_linkAnim.x *= 0.95f;
		}
	}

	//________________________________________________________________________

	void CHudWeapons::fallAnim(unsigned int msecs) {
		Vector3 horizontal = (_graphicsEntities[_currentWeapon].graphicsEntity->getOrientation() * _halfPi) * Vector3::NEGATIVE_UNIT_Z;
		horizontal.normalise();
		Vector3 maxOffset = _fallAnim.horizontalOffset * _fallAnim.movementDir * horizontal;

		horizontal *= _fallAnim.movementDir * _fallAnim.horizontalSpeed * msecs * Vector3(1.0f, 0.0f, 1.0f);
		_fallAnim.offset += horizontal;
		if(_fallAnim.offset.length() > _fallAnim.horizontalOffset) {
			_fallAnim.offset = maxOffset;
		}
	}

	//________________________________________________________________________

	void CHudWeapons::loadingWeapon(bool state) {
		_loadingWeapon = state;
		if(!_loadingWeapon) {
			_unstableLoadAnim.currentVerticalSpeed = _unstableLoadAnim.initVerticalSpeed;
		}
	}

	//________________________________________________________________________

	void CHudWeapons::loadWeaponAnim(unsigned int msecs) {
		// Calculamos el ruido horizontal
		_unstableLoadAnim.offset = (_graphicsEntities[_currentWeapon].graphicsEntity->getOrientation() * _halfPi) * Vector3::NEGATIVE_UNIT_Z;
		_unstableLoadAnim.offset.normalise();

		_unstableLoadAnim.offset *= Math::unifRand(0.0f, _unstableLoadAnim.currentNoise) * Vector3(1.0f, 0.0f, 1.0f);

		if(_unstableLoadAnim.currentNoise != _unstableLoadAnim.maxNoiseSpeed) {
			_unstableLoadAnim.currentNoise += _unstableLoadAnim.noiseInc;
			
			if(_unstableLoadAnim.currentNoise > _unstableLoadAnim.maxNoiseSpeed) {
				_unstableLoadAnim.currentNoise = _unstableLoadAnim.maxNoiseSpeed;
			}
		}
		
		// Calculamos la velocidad de movimiento vertical
		_unstableLoadAnim.offset.y = sineStep(msecs, _unstableLoadAnim.currentVerticalPos, 
			_unstableLoadAnim.verticalOffset, _unstableLoadAnim.currentVerticalSpeed);
		
		if(_unstableLoadAnim.currentVerticalSpeed != _unstableLoadAnim.maxVerticalSpeed) {
			_unstableLoadAnim.currentVerticalSpeed += _unstableLoadAnim.speedInc;
			
			if(_unstableLoadAnim.currentVerticalSpeed > _unstableLoadAnim.maxVerticalSpeed) {
				_unstableLoadAnim.currentVerticalSpeed = _unstableLoadAnim.maxVerticalSpeed;
			}
		}
	}

	//________________________________________________________________________

	void CHudWeapons::shootAnim(float force) {
		Vector3 weaponDir =  _graphicsEntities[_currentWeapon].graphicsEntity->getOrientation() * Vector3::NEGATIVE_UNIT_Z;
		weaponDir.normalise();
		_shootAnim.offset = weaponDir * force * Vector3(1.0f, 0.0f, 1.0f);
	}

	//________________________________________________________________________

	void CHudWeapons::continouosShooting(bool state) {
		_continouslyShooting = state;
		if(!_continouslyShooting)
			_rapidShootAnim.acumFiringTime = 0;
	}

	//________________________________________________________________________

	void CHudWeapons::rapidShootAnim(unsigned int msecs) {
		Vector3 weaponDir = _graphicsEntities[_currentWeapon].graphicsEntity->getOrientation() * Vector3::NEGATIVE_UNIT_Z;
		weaponDir.normalise();

		_rapidShootAnim.acumFiringTime += msecs;
		if(_rapidShootAnim.acumFiringTime > _rapidShootAnim.firingRate) {
			_rapidShootAnim.offset = weaponDir * _rapidShootAnim.shakeOffset * Vector3(1.0f, 0.0f, 1.0f);
			_rapidShootAnim.shakeOffset *= -1.0f;

			_rapidShootAnim.acumFiringTime = 0;
		}

		_rapidShootAnim.offset.y = sineStep(msecs, _rapidShootAnim.currentVerticalPos, 
			_rapidShootAnim.verticalOffset, _rapidShootAnim.verticalSpeed);
	}

	//________________________________________________________________________

	void CHudWeapons::changeWeaponAnim(unsigned int msecs) {
		// Rotamos la orientacion del arma ligeramente para que parezca que la 
		// enfundamos
		Euler euler( _graphicsEntities[_currentWeapon].graphicsEntity->getOrientation() );
		euler.yaw( Ogre::Radian( Math::PI / 8.0f) );

		// Calculamos el desplazamiento sobre el eje x para la ecuacion lineal
		// construida para el seno
		if(_chgWpnAnim.takingAway)
			_chgWpnAnim.x += _chgWpnAnim.xSpeed * msecs;
		else
			_chgWpnAnim.x -= _chgWpnAnim.xSpeed * msecs;

		// Desplazamos sobre la horizontal el arma
		_chgWpnAnim.offset = euler.toQuaternion() * Vector3::NEGATIVE_UNIT_Z;
		_chgWpnAnim.offset.normalise();
		_chgWpnAnim.offset *= -_chgWpnAnim.x * _chgWpnAnim.horizontalSpeed * Vector3(1.0f, 0.0f, 1.0f);

		// Desplazamos sobre la vertical el arma en base a la ecuacion precalculada
        if (_chgWpnAnim.x < _threePiQuarters) {
			// Si aun no hemos acabado de dibujar la curva del seno, calcular la
			// proxima posicion para la y dada esta x
			_chgWpnAnim.offset.y = sin(_chgWpnAnim.x) * _chgWpnAnim.sineOffset;
        }
        else {
			// Esta es la ecuacion que describe una recta que pasa por el punto
			// Seno(3PI/4) * k (con K el offset del seno) y el punto Seno(PI).

            // Ecuacion de la recta pre-calculada
			_chgWpnAnim.offset.y = (-0.9003163162f * _chgWpnAnim.sineOffset) * (_chgWpnAnim.x - Math::PI);
        }

		if(_chgWpnAnim.takingAway && _chgWpnAnim.x > 6.0f) {
			_chgWpnAnim.takingAway = false;
			
			_graphicsEntities[_currentWeapon].graphicsEntity->setVisible(false);
			_graphicsEntities[_chgWpnAnim.nextWeapon].graphicsEntity->setVisible(true);

			//_overlayWeapon3D[_currentWeapon]->setVisible(false);
			//_overlayWeapon3D[_chgWpnAnim.nextWeapon]->setVisible(true);
			_currentWeapon = _chgWpnAnim.nextWeapon;

			Audio::CServer::getSingletonPtr()->playSound("weapons/change.wav");
		}
		else if(_chgWpnAnim.x < 0.0f) {
			_changingWeapon = false;
			_chgWpnAnim.offset = Vector3::ZERO;

			shared_ptr<CMessageBlockShoot> lockWeaponsMsg = make_shared<CMessageBlockShoot>();
			lockWeaponsMsg->canShoot(true);
			_entity->emitMessage(lockWeaponsMsg);
		}
	}

	//________________________________________________________________________

	float CHudWeapons::sineStep(unsigned int msecs, float& currentSinePosition, float offset, float speed, float loBound, float hiBound) {
		currentSinePosition += speed * msecs;
		if(currentSinePosition > hiBound) currentSinePosition = loBound;

		return sin(currentSinePosition) * offset;
	}

	//________________________________________________________________________

	void CHudWeapons::idleAnim(unsigned int msecs) {
		_idleAnim.offset.y = sineStep(msecs, _idleAnim.currentVerticalPos, _idleAnim.verticalOffset, _idleAnim.verticalSpeed);
	}

	//________________________________________________________________________

	void CHudWeapons::linkAnim(unsigned int msecs) {
		_linkAnim.offset.y = sineStep(msecs, _linkAnim.x, _linkAnim.sinOffset, _linkAnim.xSpeed);
	}

	//________________________________________________________________________

	void CHudWeapons::landAnim(unsigned int msecs) {
		_landAnim.currentOffset += _landAnim.recoverySpeed * msecs;
		_landAnim.offset.y = sin(_landAnim.currentOffset) * _landAnim.force;
		if(_landAnim.offset.y >= 0.0f) {
			_landAnim.offset.y = _landAnim.currentOffset = _landAnim.force = 0;
			_playerIsLanding = false;
		}
	}

	//________________________________________________________________________

	void CHudWeapons::offsetRecovery(unsigned int msecs) {
		_runAnim.offset *= Vector3(0.85f, 0.85f, 0.85f);
		_landAnim.offset *= Vector3(0.85f, 0.85f, 0.85f);
		_runAnim.currentHorizontalPos *= 0.85f;
		_runAnim.currentVerticalPos *= 2 * 0.85f;
	}

	//________________________________________________________________________

	void CHudWeapons::playerIsFalling(bool falling, int direction) {
		_fallAnim.movementDir = (_playerIsFalling = falling) ? direction : 0;
	}

	//________________________________________________________________________

	void CHudWeapons::walkAnim(unsigned int msecs) {
		_runAnim.oldStrafingDir = _runAnim.currentStrafingDir;
		_runAnim.currentStrafingDir = _avatarController->getDisplacementDir().x;

		_runAnim.offset = (_graphicsEntities[_currentWeapon].graphicsEntity->getOrientation() * _halfPi) * Vector3::NEGATIVE_UNIT_Z;
		_runAnim.offset.normalise();

		_runAnim.offset *= sineStep(msecs, _runAnim.currentHorizontalPos, _runAnim.horizontalOffset, 
									_runAnim.horizontalSpeed, Math::HALF_PI, (2 * Math::PI) + Math::HALF_PI)
							* Vector3(1.0f, 0.0f, 1.0f);
		
		_runAnim.offset.y = sineStep(msecs, _runAnim.currentVerticalPos, _runAnim.verticalOffset, 
			_runAnim.verticalSpeed, Math::HALF_PI, (2 * Math::PI) + Math::HALF_PI);
	}

} // namespace Logic

