//---------------------------------------------------------------------------
// AvatarController.cpp
//---------------------------------------------------------------------------

/**
@file AvatarController.cpp

Contiene la implementaci�n del componente que controla el movimiento 
de la entidad.
 
@see Logic::CAvatarController
@see Logic::IComponent

@author Rub�n Mulero Guerrero
@author Francisco Aisa Garc�a
@date Abril, 2013
*/

#include "AvatarController.h"
#include "CameraFeedbackNotifier.h"
#include "Logic/Entity/Entity.h"
#include "Map/MapEntity.h"
#include "Logic/Entity/Components/PhysicController.h"
#include "Logic/Entity/Components/Camera.h"
#include "Logic/GameNetMsgManager.h"

#include "Logic/Server.h"
#include "Logic/Maps/Map.h"
#include "Logic/Messages/MessageHudDebugData.h"

#include "Logic/Messages/MessageControl.h"
#include "Logic/Messages/MessageMouse.h"
#include "Logic/Messages/MessageAddForcePlayer.h"
#include "Logic/Messages/MessageChangeGravity.h"
#include "Logic/Messages/MessageAudio.h"
#include "Logic/Messages/MessageHoundCharge.h"

#include "Graphics/Scene.h"
#include "Graphics/Camera.h"

#include "Physics/Server.h"

#include <math.h>

namespace Logic {

	IMP_FACTORY(CAvatarController);

	//________________________________________________________________________

	CAvatarController::CAvatarController() : _gravity(Vector3::ZERO),
											 _touchingGround(false),
											 _sideColliding(false),
											 _collisionOnTop(false),
											 _walking(false),
											 _physicController(0),
											 _momentum(Vector3::ZERO),
											 _displacementDir(Vector3::ZERO),
											 _dodgeForce(Vector3::ZERO)
	{
		
		//anti release
		for(int i = 0; i < 18 ; ++i)
			_movementCommands[i] = Vector3::ZERO;

		// Inicializamos el array que contiene los vectores
		// de cada tecla de movimiento
		initMovementCommands();
	}

	//________________________________________________________________________

	CAvatarController::~CAvatarController() {
		// Nada que hacer
	}

	//________________________________________________________________________

	bool CAvatarController::spawn(CEntity *entity, CMap *map, const Map::CEntity *entityInfo) {
		if( !IComponent::spawn(entity,map,entityInfo) ) return false;

		assert( entityInfo->hasAttribute("acceleration") && "Error: No se ha definido el atributo acceleration en el mapa" );
		_acceleration = entityInfo->getFloatAttribute("acceleration");
	
		assert( entityInfo->hasAttribute("maxVelocity") && "Error: No se ha definido el atributo maxVelocity en el mapa" );
		_maxVelocity = entityInfo->getFloatAttribute("maxVelocity");
		_maxGravVelocity = _maxVelocity * 6;

		assert( entityInfo->hasAttribute("frictionCoef") && "Error: No se ha definido el atributo frictionCoef en el mapa" );
		_frictionCoef = entityInfo->getFloatAttribute("frictionCoef");
		
		assert( entityInfo->hasAttribute("airFrictionCoef") && "Error: No se ha definido el atributo airFrictionCoef en el mapa" );
		_airFrictionCoef = entityInfo->getFloatAttribute("airFrictionCoef");

		assert( entityInfo->hasAttribute("airSpeedCoef") && "Error: No se ha definido el atributo airSpeedCoef en el mapa" );
		_airSpeedCoef = entityInfo->getFloatAttribute("airSpeedCoef");
		
		assert( entityInfo->hasAttribute("gravity") && "Error: No se ha definido el atributo gravity en el mapa" );
		_gravity.y = entityInfo->getFloatAttribute("gravity");

		assert( entityInfo->hasAttribute("jumpForce") && "Error: No se ha definido el atributo jumpForce en el mapa" );
		_jumpForce = entityInfo->getFloatAttribute("jumpForce");

		assert( entityInfo->hasAttribute("dodgeForce") && "Error: No se ha definido el atributo dodgeForce en el mapa" );
		_dodgeForce = entityInfo->getVector3Attribute("dodgeForce");

		return true;
	} // spawn

	//________________________________________________________________________

	void CAvatarController::onActivate() {
		_displacementDir = _momentum = Vector3::ZERO;
	} // activate

	//________________________________________________________________________

	void CAvatarController::onWake() {
		_momentum = Vector3::ZERO;
	}

	//________________________________________________________________________

	bool CAvatarController::accept(const std::shared_ptr<CMessage>& message) {
		TMessageType msgType = message->getMessageType();

		if( this->isInDeepSleep() ) {
			if(msgType == Message::CONTROL) {
				ControlType ctrlType = std::static_pointer_cast<CMessageControl>(message)->getType();

				if(ctrlType == Control::MOUSE)
					return false;
			}
			else {
				return false;
			}
		}

		return msgType == Message::CONTROL			||
			   msgType == Message::ADDFORCEPLAYER	||
			   msgType == Message::HOUND_CHARGE		||
			   msgType == Message::CHANGE_GRAVITY;
	} // accept

	//________________________________________________________________________

	void CAvatarController::process(const std::shared_ptr<CMessage>& message) {
		switch( message->getMessageType() ) {
			case Message::CONTROL: {
				ControlType commandType = std::static_pointer_cast<CMessageControl>(message)->getType();

				// Comando de movimiento
				if(commandType >=0 && commandType < MAX_MOVEMENT_COMMANDS) {
					executeMovementCommand(commandType);
				}
				// Comando de salto
				else if(commandType == Control::JUMP) {
					executeJump();
				}
				// Comando de esquiva
				else if(commandType > Control::JUMP && commandType < Control::MOUSE) {
					executeDodge(commandType);
				}
				// Comando de raton
				else if(commandType == Control::MOUSE) {
					std::shared_ptr<CMessageMouse> mouseMsg = std::static_pointer_cast<CMessageMouse>(message);
					rotationXY( mouseMsg->getMouse() );
				}

				break;
			}
			case Message::ADDFORCEPLAYER:{
				//Fuerza aplicada al jugador
				std::shared_ptr<CMessageAddForcePlayer> addForceMsg = std::static_pointer_cast<CMessageAddForcePlayer>(message);
				addForce( addForceMsg->getForce() );
				break;
			}
			case Message::CHANGE_GRAVITY:{
				_gravity.y = std::static_pointer_cast<CMessageChangeGravity>(message)->getGravity();
			}
			case Message::HOUND_CHARGE: {
				std::shared_ptr<CMessageHoundCharge> houndChargeMsg = std::static_pointer_cast<CMessageHoundCharge>(message);
				
				if( houndChargeMsg->isActive() ) {
					Vector3 chargeVector = -1.0f * ( _entity->getOrientation().zAxis() );
					chargeVector.normalise();
					// Si estamos en el aire nos desplazamos un 10% de lo que en realidad nos desplazariamos en el suelo
					chargeVector *= (_touchingGround || _sideColliding) ? houndChargeMsg->getForce() : houndChargeMsg->getForce() * 0.4f;

					_filterMask = houndChargeMsg->getFilterMask();
					addForce(chargeVector);
				}
				else {
					_filterMask = _physicController->getDefaultFilterMask();
				}
			}
		}

	} // process	

	//________________________________________________________________________

	void CAvatarController::rotationXY(float XYturn[]) {
		//Aplicamos las rotaciones pertinentes
		 Ogre::Real pitchAngle;
		 Ogre::Real pitchAngleSign;
 
		 //Rotamos el Yaw de la entidad de acuerdo a los grados en radianes pasados como par�metro.
		 _entity->rotate(Orientation::eYAW,Ogre::Radian(XYturn[0]));
 
		 //Rotamos el Pitch de la entidad de acuerdo a los grados en radianes pasados como par�metro.
		 _entity->rotate(Orientation::ePITCH,Ogre::Radian(XYturn[1]));
		
		 //Obtenemos el pitch actual
		 Quaternion actualPitch=_entity->getPitch();

		 // �ngulo de rotaci�n sobre el eje X.
		 pitchAngle = (2 * Ogre::Degree(Ogre::Math::ACos(actualPitch.w)).valueDegrees());
 
		 // Para saber el sentido.
		 pitchAngleSign = actualPitch.x;
 
		 // Limitamos el angulo un poco menos de +90/-90 para evitar perder la direccion (Tomando como referencia a Quake3)
		 if (pitchAngle > 88.0f)
		 {
			 if (pitchAngleSign > 0)
				 //Fijando a +88. Formo el quaternion previamente calculado, ahorrando procesamiento
				 _entity->setPitch(Ogre::Quaternion(0.71934,0.694658, 0, 0));
			 else if (pitchAngleSign < 0)
				 //Fijando a -88. Formo el quaternion previamente calculado, ahorrando procesamiento
				 _entity->setPitch(Ogre::Quaternion(0.71934,-0.694658, 0, 0));
		 }
	
	}//rotationXY
	//________________________________________________________________________

	void CAvatarController::onStart() {
		// Para evitar overhead de mensajes nos quedamos con el puntero al 
		// componente CPhysicController que es el que realmente se encargar� 
		// de desplazar al controlador del jugador.
		_physicController = _entity->getComponent<CPhysicController>("CPhysicController");
		assert(_physicController && "Error: El player no tiene un controlador fisico");

		// Pasamos la mascara de movimiento al modo normal
		_filterMask = _physicController->getDefaultFilterMask();
	}

	//________________________________________________________________________

	void CAvatarController::onFixedTick(unsigned int msecs) {
		// Calculamos el vector de desplazamiento teniendo en cuenta
		// si estamos en el aire o en el suelo
		Vector3 displacement = _touchingGround ? estimateGroundMotion(msecs) : estimateAirMotion(msecs);

		// Seteamos el efecto de camara
		setCameraEffect();
		
		// Tratamos de mover el controlador fisico con el desplazamiento estimado.
		// En caso de colision, el controlador fisico nos informa.
		// Debido al reposicionamiento de la c�psula que hace PhysX, le seteamos un offset fijo
		// al movernos para asegurarnos de que hay colision
		Vector3 oldPosition = _entity->getPosition();
		manageCollisions( _physicController->move(displacement-Vector3(0.0f, 0.15f, 0.0f), _filterMask, msecs), oldPosition );
	} // tick

	//________________________________________________________________________

	void CAvatarController::setCameraEffect() {
		if(_touchingGround && !_walking && _displacementDir != Vector3::ZERO ) {
			for(auto it = _observers.begin(); it != _observers.end(); ++it) {
				(*it)->onWalk();
			}

			_walking = true;
		}
		else if(_touchingGround && _walking && _displacementDir == Vector3::ZERO) {
			for(auto it = _observers.begin(); it != _observers.end(); ++it) {
				(*it)->onIdle();
			}

			_walking = false;
		}
		else if(!_touchingGround) {
			_walking = false;
		}
	}

	//________________________________________________________________________

	void CAvatarController::manageCollisions(unsigned collisionFlags, Vector3 oldPosition) {
		// Comprobamos si estamos tocando suelo o caemos
		if( !_touchingGround && (collisionFlags & Physics::eCOLLISION_DOWN) ) {
			_touchingGround = true;
			for(auto it = _observers.begin(); it != _observers.end(); ++it) {
				(*it)->onLand();
				(*it)->downCollision(_touchingGround);
			}
		}
		else if( _touchingGround && !(collisionFlags & Physics::eCOLLISION_DOWN) ) {
			_touchingGround = false;
			for(auto it = _observers.begin(); it != _observers.end(); ++it) {
				(*it)->onAir();
				(*it)->downCollision(_touchingGround);
			}
		}

		// Comprobamos si estamos colisionando a los los lados
		if( !_sideColliding && (collisionFlags & Physics::eCOLLISION_SIDES) ) {
			// Empiezo a chocar por los lados
			_sideColliding = true;
			for(auto it = _observers.begin(); it != _observers.end(); ++it) {
				(*it)->sideCollision(_sideColliding);
			}
		}
		else if(_sideColliding && !(collisionFlags & Physics::eCOLLISION_SIDES) ) {
			// Dejo de chocar por los lados
			_sideColliding = false;
			for(auto it = _observers.begin(); it != _observers.end(); ++it) {
				(*it)->sideCollision(_sideColliding);
			}
		}

		// Comprobamos si estamos chocando con la cabezota
		if( !_collisionOnTop && (collisionFlags & Physics::eCOLLISION_UP) ) {
			// Empiezo a chocar con el cabezon
			_collisionOnTop = true;
			for(auto it = _observers.begin(); it != _observers.end(); ++it) {
				(*it)->topCollision(_collisionOnTop);
			}

			// Frenamos de golpe la velocidad a la que estuvieramos ascendiendo
			_momentum.y = 0;
		}
		else if( _collisionOnTop && !(collisionFlags & Physics::eCOLLISION_UP) ) {
			// Dejo de chocar con la chola
			_collisionOnTop = false;
			for(auto it = _observers.begin(); it != _observers.end(); ++it) {
				(*it)->topCollision(_collisionOnTop);
			}
		}
		
		if(collisionFlags & Physics::eCOLLISION_SIDES){
			// Necesitamos la posicion anterior del personaje, para ver la direcci�n 
			// y la velocidad a la que nos hemos movido, y calculamos el momentum
			// resultado de habernos movido y haber chocado contra las paredes
			Vector3 newMom = _entity->getPosition() - oldPosition;
			_momentum.x = newMom.x;
			_momentum.z = newMom.z;
		}
	}

	//________________________________________________________________________

	Vector3 CAvatarController::estimateMotionDirection(const Vector3& direction) {
		// Si nuestro movimiento es nulo no hacemos nada
		if(direction == Vector3::ZERO) return Vector3::ZERO;

		// Mediante trigonometria basica sacamos el angulo que forma el desplazamiento
		// que queremos llevar a cabo
		float displacementYaw = asin(direction.normalisedCopy().x);
		// Obtenemos una copia de la orientacion del personaje
		Quaternion characterQuat = _entity->getOrientation();
		// Si estamos andando hacia atras, invertimos el giro
		if(direction.z < 0) displacementYaw *= -1;

		// Rotamos tantos grados como diga el vector desplazamiento calculado de pulsar las teclas
		Math::rotate( Vector3::UNIT_Y,Ogre::Radian(displacementYaw), characterQuat);
		//Obtenemos la direccion
		Vector3 directionQuat = characterQuat * Vector3::NEGATIVE_UNIT_Z;
		//Formamos el vector normalizado de la direccion
		Vector3 motionDirection(directionQuat.x, 0, directionQuat.z);
		motionDirection.normalise();

		// Invertimos el vector resultante si nos estamos desplazando hacia atras
		// porque el yaw se calcula de forma contraria al andar hacia atras
		if(direction.z < 0) motionDirection *= -1;

		// Devolvemos la direcci�n del movimiento estimado
		return motionDirection;
	}

	//________________________________________________________________________

	Vector3 CAvatarController::estimateGroundMotion(unsigned int msecs) {
		// Si no nos queremos desplazar en ninguna direcci�n aplicamos el coeficiente
		// de rozamiento
		float coef = (_displacementDir == Vector3::ZERO) ? 0.8f : _maxVelocity / (_maxVelocity + (0.5 * _acceleration * msecs) );

		_momentum += Vector3(1,0,1) * estimateMotionDirection(_displacementDir) * _acceleration * msecs * 0.5f;
		// Multiplicamos la inercia por el coeficiente de rozamiento (para decelerarla)
		_momentum *= coef;
		// Seteamos una gravedad fija para que la c�psula colisione contra el suelo
		_momentum.y = _gravity.y * msecs;

		return _momentum;
	}

	//________________________________________________________________________

	Vector3 CAvatarController::estimateAirMotion(unsigned int msecs) {
		// Aplicamos una fuerza de rozamiento al desplazamiento aereo para frenarnos
		// un poco al estar volando
		_momentum *= Vector3(_airFrictionCoef, 1, _airFrictionCoef);
		// Calculamos el coeficiente de movimiento aereo (que deber�a ser m�s reducido
		// que el terrestre) teniendo en cuenta los milisegundos transcurridos
		float speedCoef = _airSpeedCoef / (double)msecs;

		// Aumentamos el desplazamiento en la direcci�n dada teniendo en cuenta
		// que nos movemos m�s lento en el aire -> -> s = u � t + 1/2 � a � t^2
		_momentum += Vector3(speedCoef, 0, speedCoef) * estimateMotionDirection(_displacementDir) * _acceleration * msecs * 0.5f;
		// Incrementamos el empuje de la gravedad
		_momentum.y += _gravity.y * msecs * 0.5f;

		// Fijamos la velocidad de caida a la velocidad m�xima de caida si es necesario
		normalizeGravity();

		return _momentum;
	}

	//________________________________________________________________________

	void CAvatarController::normalizeGravity() {
		// Normalizamos la velocidad vertical
		float gVelocity = ( _momentum*Vector3(0, 1, 0) ).length();
		if(gVelocity >_maxGravVelocity) {
			double coef = _maxGravVelocity/gVelocity;
			_momentum *= Vector3(1, coef, 1);
		}
	}

	//________________________________________________________________________

	void CAvatarController::executeMovementCommand(ControlType commandType) {
		Vector3 dir = _movementCommands[commandType];

		_displacementDir += dir;
	}

	//________________________________________________________________________

	void CAvatarController::initMovementCommands() {
		// Comandos de movimiento
		_movementCommands[Control::WALK]				= Vector3(0,0,1);
		_movementCommands[Control::WALKBACK]			= Vector3(0,0,-1);
		_movementCommands[Control::STRAFE_LEFT]			= Vector3(1,0,0);
		_movementCommands[Control::STRAFE_RIGHT]		= Vector3(-1,0,0);
		_movementCommands[Control::STOP_WALK]			= Vector3(0,0,-1);
		_movementCommands[Control::STOP_WALKBACK]		= Vector3(0,0,1);
		_movementCommands[Control::STOP_STRAFE_LEFT]	= Vector3(-1,0,0);
		_movementCommands[Control::STOP_STRAFE_RIGHT]	= Vector3(1,0,0);
		
		// Comandos de salto
		_movementCommands[Control::JUMP]				= Vector3(0,1,0);
		_movementCommands[Control::SIDEJUMP_LEFT]		= Vector3(1,0,0);
		_movementCommands[Control::SIDEJUMP_RIGHT]		= Vector3(-1,0,0);
		_movementCommands[Control::DODGE_BACKWARDS]		= Vector3(0,0,-1);
		_movementCommands[Control::DODGE_FORWARD]		= Vector3(0,0,1);

	}

	//________________________________________________________________________

	void CAvatarController::executeJump() {
		if(_touchingGround) {
			_momentum.y = _jumpForce;
			_touchingGround = false;

			for(auto it = _observers.begin(); it != _observers.end(); ++it) {
				(*it)->onJump();
				(*it)->onAir();
				(*it)->downCollision(_touchingGround);
			}
		}
	}

	//________________________________________________________________________

	void CAvatarController::addForce(const Vector3 &force){
		_momentum += force;
		if(_momentum.y > 0 && _touchingGround) {
			_touchingGround = false;
			for(auto it = _observers.begin(); it != _observers.end(); ++it) {
				(*it)->onAir();
				(*it)->downCollision(_touchingGround);
			}
		}
	}

	//________________________________________________________________________

	void CAvatarController::addObserver(IObserver* observer) {
		_observers.push_back(observer);
	}

	//________________________________________________________________________

	void CAvatarController::removeObserver(IObserver* observer) {
		for(auto it = _observers.begin(); it != _observers.end(); ++it) {
			if(*it == observer) {
				_observers.erase(it);
				break;
			}
		}
	}

	//________________________________________________________________________

	void CAvatarController::executeDodge(ControlType commandType){
		_displacementDir += _movementCommands[commandType];
		if(!_touchingGround)
			return;
		
		Vector3 dir = estimateMotionDirection(_movementCommands[commandType]) + Vector3(0, 1, 0);
		Vector3 force = dir.normalisedCopy() * _dodgeForce;

		_momentum += force;
		if(_momentum.y > 0 && _touchingGround) {
			_touchingGround = false;
			for(auto it = _observers.begin(); it != _observers.end(); ++it) {
				(*it)->onDodge();
				(*it)->onAir();
				(*it)->downCollision(_touchingGround);
			}
		}
	}

} // namespace Logic
