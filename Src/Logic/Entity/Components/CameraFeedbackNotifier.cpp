/**
@file CameraFeedbackNotifier.cpp

Contiene la implementaci�n del componente que 
controla los da�os recibidos por el escudo
del Screamer.
 
@see Logic::CCameraFeedbackNotifier
@see Logic::IComponent

@author Jaime Chapinal Cervantes
@date Abril, 2013
*/

#include "CameraFeedbackNotifier.h"
#include "Camera.h"
#include "HudWeapons.h"
#include "Logic/Server.h"
#include "Logic/Entity/Entity.h"
#include "Logic/Maps/Map.h"
#include "Map/MapEntity.h"
#include "Application/BaseApplication.h"
#include "Graphics/Scene.h"
#include "Graphics/Server.h"

// Mensajes
#include "Logic/Messages/MessageDamaged.h"
#include "Logic/Messages/MessageFlash.h"
#include "Logic/Messages/MessageSetReducedDamage.h"
#include "Logic/Messages/MessageCameraOffset.h"
#include "Logic/Messages/MessageControl.h"
#include "Logic/Messages/MessageImpact.h"

namespace Logic {
	
	IMP_FACTORY(CCameraFeedbackNotifier);

	//________________________________________________________________________

	CCameraFeedbackNotifier::CCameraFeedbackNotifier() : _owner(NULL),
														 _playerIsWalking(false),
														 _playerIsLanding(false),
														 _landForce(0),
														 _strafingDir(0),
														 _landRecoverySpeed(0.007f),
														 _currentLandOffset(0),
														 _shockWavePos(0.0f),
														 _shockWaveSpeed(0.001f),
														 //_shockWaveSpeed(0.0001f),
														 _shockWaveLength(0.4f),
														 _shockWaveIsActive(false),
														 _flashVisible(true) {


		_walkAnim.currentHorizontalPos = Math::HALF_PI;
		_walkAnim.horizontalSpeed = 0.0075f;
		_walkAnim.horizontalOffset = 0.055f;

		_walkAnim.currentVerticalPos = Math::HALF_PI;
		_walkAnim.verticalSpeed = _walkAnim.horizontalSpeed * 2;
		_walkAnim.verticalOffset = 0.055f;

		_walkAnim.currentRoll = 0.0f;
		_walkAnim.rollSpeed = 0.00005f;
		_walkAnim.rollOffset = 0.015f;
		_walkAnim.rollCoef = 0.95f;
		_walkAnim.recoveringRoll = false;
		_walkAnim.currentStrafingDir = 0;
	}

	//________________________________________________________________________

	CCameraFeedbackNotifier::~CCameraFeedbackNotifier() {
		// Nada que borrar
	}

	//________________________________________________________________________

	bool CCameraFeedbackNotifier::spawn(CEntity *entity, CMap *map, const Map::CEntity *entityInfo) {
		if( !IComponent::spawn(entity,map,entityInfo) )
			return false;

		assert( entityInfo->hasAttribute("maxVelocity") && "Error: No se ha definido el atributo maxVelocity en el mapa" );
		_maxVelocity = entityInfo->getFloatAttribute("maxVelocity");
		
		return true;
	} // spawn
	
	//________________________________________________________________________

	bool CCameraFeedbackNotifier::accept(const std::shared_ptr<CMessage>& message) {
		Logic::TMessageType msgType = message->getMessageType();

		return msgType == Message::DAMAGED	||
			   msgType == Message::FLASH;
	} // accept
	
	//________________________________________________________________________

	void CCameraFeedbackNotifier::process(const std::shared_ptr<CMessage>& message) {
		//Ambas hacen lo mismo de momento, pero lo dejo separado por si luego queremos poner 
		//comportamientos distintos en funci�n del da�o
		switch( message->getMessageType() ) {
			case Message::DAMAGED: {
				std::shared_ptr<CMessageDamaged> damageMess = std::static_pointer_cast<CMessageDamaged>(message);
				CEntity* enemy = damageMess->getEnemy();
				if(enemy != NULL)
					damaged( enemy->getPosition() );
				
				break;
			}
			/*case Message::SET_REDUCED_DAMAGE: {
				damaged();
				break;
			}*/
			case Message::FLASH: {
				std::shared_ptr<CMessageFlash> flashMsg = std::static_pointer_cast<CMessageFlash>(message);
				_flashFactor = flashMsg->getFlashFactor();
				_flashVisible = true;
				_scene->setCompositorVisible(_flashEffect, true);
				break;
			}
		}
	} // process

	//________________________________________________________________________

	void CCameraFeedbackNotifier::onStart() {
		CEntity* cameraEntity = Logic::CServer::getSingletonPtr()->getMap()->getEntityByName("Camera");
		assert(cameraEntity != NULL && "Error: No existe una entidad camara");
		_cameraComponent = cameraEntity->getComponent<CCamera>("CCamera");
		assert(_cameraComponent != NULL && "Error: La entidad camara no tiene un componente de camara");
		_avatarc = _entity->getComponent<CAvatarController>("CAvatarController");
		assert(_avatarc != NULL && "Error: no tenemos avatar controller lol");

		// Nos registramos como observadores (no hace falta que nos desregistremos)
		_avatarc->addObserver(this);

		_scene = _entity->getMap()->getScene();
		_effect = "damageCompositor";
		_flashEffect = "Muzzle Flash";
		_strengthEffect = "strength";
		_effectIsActivated = false;
		_scene->createCompositor(_effect);
		_scene->setCompositorVisible(_effect, false);
		// Por ahora esta a hierro, lo suyo es ponerlo por el mapa
		_scene->updateCompositorVariable(_effect, _strengthEffect, 1);
		
		_scene->createCompositor(_flashEffect);
		_scene->setCompositorVisible(_flashEffect, false);
		// Por ahora esta a hierro, lo suyo es ponerlo por el mapa
		_scene->updateCompositorVariable(_flashEffect, "flashLevel", 1.0);

		_scene->createCompositor("shockWaveCompositor");
		_scene->setCompositorVisible("shockWaveCompositor", false);

		//_scene->createCompositor("heatHazeCompositor");
		//_scene->setCompositorVisible("heatHazeCompositor", true);
	}

	//________________________________________________________________________

	void CCameraFeedbackNotifier::onFixedTick(unsigned int msecs) {
		if(_playerIsLanding)
			landEffect(msecs);
		else if(_playerIsWalking)
			walkEffect(msecs);
		else
			offsetRecovery(msecs);

		if(_effectIsActivated){
			_scene->updateCompositorVariable(_effect, _strengthEffect, 1+_timestamp*0.01);
			_timestamp += msecs;
			if(_timestamp > 500){
				_effectIsActivated = false;
				_scene->setCompositorVisible(_effect, false);
			}
		}
		
		//ahora actualizamos el flashazo si procede
		if(_flashFactor > 1.0){
			_flashFactor-=0.5f;
			_scene->updateCompositorVariable(_flashEffect,"flashLevel",_flashFactor);
			//std::cout << "estoy flasheado" << _flashFactor << std::endl;
		}else if(_flashVisible){
			_scene->setCompositorVisible(_flashEffect, false);
			_flashVisible = false;
		}

		if(_shockWaveIsActive) {
			_shockWavePos += _shockWaveSpeed * msecs;
			if(_shockWavePos > _shockWaveLength) {
				_shockWaveIsActive = false;
				_shockWavePos = 0.0f;
				_scene->setCompositorVisible("shockWaveCompositor", false);
			}
			else {
				_scene->updateCompositorVariable("shockWaveCompositor", "wavePos", _shockWavePos);
			}
		}
	}

	//________________________________________________________________________

	void CCameraFeedbackNotifier::weaponShockWave(const Vector2& waveCenter, float waveLength, float waveSpeed, 
												  float waveSize, float waveCurvature, float waveIntensity) {

		// Configuramos los valores del shader
		_scene->setCompositorVisible("shockWaveCompositor", true);
		_scene->updateCompositorVariable("shockWaveCompositor", "waveLength", waveLength);
		_scene->updateCompositorVariable("shockWaveCompositor", "waveU", waveCenter.x);
		_scene->updateCompositorVariable("shockWaveCompositor", "waveV", waveCenter.y);
		_scene->updateCompositorVariable("shockWaveCompositor", "waveSize", waveSize);
		_scene->updateCompositorVariable("shockWaveCompositor", "waveCurvature", waveCurvature);
		_scene->updateCompositorVariable("shockWaveCompositor", "waveIntensity", waveIntensity);
		_scene->updateCompositorVariable("shockWaveCompositor", "wavePos", 0.0f);
		
		_shockWaveLength = waveLength;
		_shockWaveSpeed = waveSpeed;
		_shockWaveIsActive = true;
	}

	//________________________________________________________________________

	void CCameraFeedbackNotifier::onWalk() { 
		_playerIsWalking = true;
	}

	//________________________________________________________________________

	void CCameraFeedbackNotifier::onAir() {
		_playerIsWalking = false;
		_strafingDir = 0;
	}

	//________________________________________________________________________

	void CCameraFeedbackNotifier::onIdle() {
		_playerIsWalking = false;
		_strafingDir = 0;
	}

	//________________________________________________________________________

	void CCameraFeedbackNotifier::offsetRecovery(unsigned int msecs) {
		// Queda mas suave sin correccion
		_cameraComponent->setOffset( _cameraComponent->getOffset() * Vector3(0.85f, 0.95f, 0.85f) );
		if(_walkAnim.currentRoll != 0) {
			_walkAnim.currentRoll *= 0.85f; // Coeficiente un poco m�s fuerte
			_cameraComponent->rollCamera(_walkAnim.currentRoll);
		}
	}

	//________________________________________________________________________

	void CCameraFeedbackNotifier::walkEffect(unsigned int msecs) {
		Vector3 offset = _cameraComponent->getOffset();

		_walkAnim.currentStrafingDir = _strafingDir;
		_strafingDir = _avatarc->getDisplacementDir().x;
		if(_strafingDir == 0) {
			Quaternion yaw( _entity->getYaw() );
			Math::rotate( Vector3::UNIT_Y, Ogre::Radian(Math::HALF_PI), yaw);
			Vector3 horizontal = yaw * Vector3::NEGATIVE_UNIT_Z;

			_walkAnim.currentHorizontalPos += _walkAnim.horizontalSpeed * msecs;
			if(_walkAnim.currentHorizontalPos > ((2 * Math::PI) + Math::HALF_PI)) _walkAnim.currentHorizontalPos = Math::HALF_PI;

			// Multiplicamos el vector horizontal normalizado por el desplazamiento y lo sumamos al offset
			horizontal *= sin(_walkAnim.currentHorizontalPos) * _walkAnim.horizontalOffset;
			offset += horizontal;
		}
		else {
			// Reducimos el offset horizontal
			// Queda mas suave sin correccion
			offset = offset * Vector3(0.95f, 1.0f, 0.95f);

			// Si no estamos recuperando el offset por haber llegado al m�ximo
			// incrementamos el roll
			if(!_walkAnim.recoveringRoll) {
				_walkAnim.currentRoll += _strafingDir * _walkAnim.rollSpeed * msecs;

				if(abs(_walkAnim.currentRoll) >= _walkAnim.rollOffset) {
					_walkAnim.currentRoll = _walkAnim.rollOffset * _walkAnim.rollCoef * _strafingDir;
					_walkAnim.recoveringRoll = true;
				}
			}
			// Si cambiamos de direccion dejamos de recuperar el offset
			else if(_walkAnim.currentStrafingDir != _strafingDir) {
				_walkAnim.recoveringRoll = false;
			}
		}

		// Recuperamos la posicion inicial multiplicando por el coeficiente
		// de recuperacion
		if(_walkAnim.recoveringRoll && _walkAnim.currentRoll != 0) {
			_walkAnim.currentRoll *= _walkAnim.rollCoef;
			if(abs(_walkAnim.currentRoll) < 0.001)
				_walkAnim.currentRoll = 0;
		}

		_cameraComponent->rollCamera(_walkAnim.currentRoll);

		_walkAnim.currentVerticalPos += _walkAnim.verticalSpeed * msecs;
		if(_walkAnim.currentVerticalPos > ((2 * Math::PI) + Math::HALF_PI)) {
			_walkAnim.currentVerticalPos = Math::HALF_PI;
		}

		offset.y += sin(_walkAnim.currentVerticalPos) * _walkAnim.verticalOffset;

		_cameraComponent->setOffset(offset);
	}

	//________________________________________________________________________

	void CCameraFeedbackNotifier::onLand() {
		float hitForce = _avatarc->getMomentum().y;
		if(hitForce < -0.3f) {
			_playerIsLanding = true;
			_landForce = hitForce;
		}
	}

	//________________________________________________________________________

	void CCameraFeedbackNotifier::landEffect(unsigned int msecs) {
		_currentLandOffset += _landRecoverySpeed * msecs;
		float vOffset = sin(_currentLandOffset) * _landForce;
		if(vOffset >= 0.0f) {
			_currentLandOffset =  _landForce = 0;
			_cameraComponent->setVerticalOffset(0);
			_playerIsLanding = false;
		}
		else {
			_cameraComponent->setVerticalOffset(vOffset);
		}
	}

	//________________________________________________________________________

	void CCameraFeedbackNotifier::damaged(Vector3 vEnemyPosition) {
		std::shared_ptr<Logic::CMessageCameraOffset> m3 = std::make_shared<Logic::CMessageCameraOffset>();
		m3->setOffsetTimer(100.0f);//Timer								 
		Logic::CEntity * camera = Logic::CServer::getSingletonPtr()->getMap()->getEntityByName("Camera");
		assert(camera != NULL && "Error: Esto no se puede hacer asi que sois unos lamers, ahora el servidor que hace?");
		camera->emitMessage(m3);

		_scene->setCompositorVisible(_effect, true);
		_effectIsActivated = true;
		_timestamp = 0;

		calculateEnemyPosition(vEnemyPosition);
	}// damaged

	//________________________________________________________________________

	void CCameraFeedbackNotifier::setOwner(Logic::CEntity* owner) { 
		this->_owner = owner; 
	}

	//________________________________________________________________________

	void CCameraFeedbackNotifier::calculateEnemyPosition(Vector3 vPosEnemy) { 
		//Obtengo la posici�n del enemigo
		Ogre::Vector3 vEnemyPos = vPosEnemy;
		//Obtengo mi posici�n (entidad a la que han da�ado)
		Ogre::Vector3 vMyPos = this->_entity->getPosition();

		//Obtengo el vector en el que estoy mirando, y me quedo s�lo en el plano horizontal (quitando la altura)
		Vector3 vMyDirVision = _entity->getOrientation()*Vector3::NEGATIVE_UNIT_Z;
		vMyDirVision = Vector3(vMyDirVision.x,0,vMyDirVision.z);
		//Obtengo el vector desde el enemigo a mi posici�n; y me quedo s�lo con el plano horizontal (quitando la altura)
		Vector3 vEnemyDirVision = vPosEnemy - vMyPos;
		vEnemyDirVision = Vector3(vEnemyDirVision.x, 0, vEnemyDirVision.z);

		//�ngulo entre ambos vectores
		Ogre::Radian rad = vMyDirVision.angleBetween(vEnemyDirVision);
		//Convierto los radianes a float porque en el mensaje mando float
		float fRadianes = (float)rad.valueRadians();

		//Cambio de sistema de coordenadas para tener la posici�n del enemigo respecto 
		//al jugador. Antonio el crack matem�tico! ^^
		Matrix4 mat;
		mat.makeTransform(_entity->getPosition(),Vector3::UNIT_SCALE,_entity->getOrientation());
		mat.inverse();
		Vector3 vec = mat * vPosEnemy; //este vector es la posicion del enemigo respecto a mi
		if (vec.x > 0) 
		{
			//El enemigo est� a la derecha, as� que tengo que multiplicar
			//por -1 para que se oriente bien la flecha de da�o
			fRadianes *= -1.0f;
		}

		//Mando el mensaje
		std::shared_ptr<Logic::CMessageImpact> impact = std::make_shared<Logic::CMessageImpact>();
		impact->setDirectionImpact(fRadianes);							 
		_entity->emitMessage(impact);
	}

} // namespace Logic


