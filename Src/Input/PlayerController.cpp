/**
@file PlayerController.cpp

Contiene la implementaci�n de la clase CPlayerController. Se encarga de
recibir eventos del teclado y del rat�n y de interpretarlos para
mover al jugador.

@see GUI::CPlayerController

@author David Llans�
@date Agosto, 2010
*/

#include "PlayerController.h"
#include "InputManager.h"

#include "Logic/Entity/Entity.h"
#include "Logic/Messages/Message.h"
#include "Logic/Entity/Components/AvatarController.h"
#include "Logic/Entity/Components/WeaponsManager.h"
#include "Logic/Entity/Components/ArrayGraphics.h"

#include "Logic/Messages/MessageChangeWeapon.h"

#include "Logic/Messages/MessageControl.h"
#include "Logic/Messages/MessageMouse.h"

#include <cassert>

//PRUEBAS,QUITAR LUEGO LOS DOS INCLUDE SIGUIENTES
#include "Logic/Messages/MessageAddLife.h"
#include "Logic/Messages/MessageAddShield.h"

//MENSAJE PARA DEBBUG
#include "Logic/Messages/MessageHudDebug.h"

#define TURN_FACTOR_X 0.001f
#define TURN_FACTOR_Y 0.001f

namespace Input {

	CPlayerController::CPlayerController() : _controlledAvatar(0)
	{

	} // CPlayerController

	//--------------------------------------------------------

	CPlayerController::~CPlayerController()
	{
		deactivate();

	} // ~CPlayerController

	//--------------------------------------------------------

	void CPlayerController::activate()
	{		
		CInputManager::getSingletonPtr()->addKeyListener(this);
		CInputManager::getSingletonPtr()->addMouseListener(this);

	} // activate

	//--------------------------------------------------------

	void CPlayerController::deactivate()
	{
		CInputManager::getSingletonPtr()->removeKeyListener(this);
		CInputManager::getSingletonPtr()->removeMouseListener(this);
	} // deactivate

	//--------------------------------------------------------

	bool CPlayerController::keyPressed(TKey key)
	{
		if(_controlledAvatar)
		{
			int iType = typeOfKey(key);
			switch (iType)
			{
				case 0: //Change weapons
					ChangeWeaponMessage(key);
				break;
				case 1: //Movement
					MovementMessage(key);
				case 2: //Habilities
					HabilityMessage(key);
				break;
				case 3: //Other (debug)
					OtherMessages(key);
					return true;
				break;				
			}//switch iType
		}//if _controlledAvatar
		return false;

	} // keyPressed

	//--------------------------------------------------------

	bool CPlayerController::keyReleased(TKey key)
	{
		if(_controlledAvatar)
		{
			Logic::CMessageControl *m=new Logic::CMessageControl();
			switch(key.keyId)
			{
			case Input::Key::W:
				m->setType(Logic::Control::STOP_WALK);
 				break;
			case Input::Key::S:
				m->setType(Logic::Control::STOP_WALKBACK);
				break;
			case Input::Key::A:
				std::cout << "Stop strafe left" << std::endl;
				m->setType(Logic::Control::STOP_STRAFE_LEFT);
				break;
			case Input::Key::D:
				std::cout << "Stop strafe right" << std::endl;
				m->setType(Logic::Control::STOP_STRAFE_RIGHT);
				break;

			case Input::Key::Q:
				m->setType(Logic::Control::STOP_PRIMARY_SKILL);
				break;
			case Input::Key::E:
				m->setType(Logic::Control::STOP_SECONDARY_SKILL);
				break;
			case Input::Key::ESCAPE:// esto debe desaparecer en el futuro
					std::cout << "escape pulsado" << std::endl;
					return false;
			//default:
				//return true;
			}
			_controlledAvatar->emitMessage(m);
			return true;
		}
		return false;

	} // keyReleased

	//--------------------------------------------------------

	bool CPlayerController::mouseMoved(const CMouseState &mouseState)
	{
		if(_controlledAvatar)
		{
			Logic::CMessageMouse *m=new Logic::CMessageMouse();
			m->setType(Logic::Control::MOUSE);
			float mouse[]={-(float)mouseState.movX * TURN_FACTOR_X,-(float)mouseState.movY * TURN_FACTOR_Y};
			m->setMouse(mouse);
			_controlledAvatar->emitMessage(m);
			return true;
		}
		return false;

	} // mouseMoved

	//--------------------------------------------------------

	bool CPlayerController::mousePressed(const CMouseState &mouseState)
	{
		if(_controlledAvatar)
		{
			Logic::CMessageControl *m=new Logic::CMessageControl();
			switch(mouseState.button)
			{
			case Input::Button::LEFT:
				m->setType(Logic::Control::LEFT_CLICK);
				break;
			case Input::Button::RIGHT:
				//m->setType(Logic::Control::RIGHT_CLICK);
				m->setType(Logic::Control::USE_PRIMARY_SKILL);
				break;
			case Input::Button::MIDDLE:
				//m->setType(Logic::Control::MIDDLE_CLICK);
				m->setType(Logic::Control::USE_SECONDARY_SKILL);
				break;
			case Input::Button::BUTTON_3:
				m->setType(Logic::Control::BUTTON3_CLICK);
				break;
			default:
				return false;
			}
			_controlledAvatar->emitMessage(m);
			return true;
		}
		return false;

	} // mousePressed

	//--------------------------------------------------------

	bool CPlayerController::mouseReleased(const CMouseState &mouseState)
	{
		if(_controlledAvatar)
		{
			int a = mouseState.scrool;
			Logic::CMessageControl *m=new Logic::CMessageControl();
			switch(mouseState.button)
			{
			case Input::Button::LEFT:
				m->setType(Logic::Control::UNLEFT_CLICK);
				break;
			case Input::Button::RIGHT:
				//m->setType(Logic::Control::UNRIGHT_CLICK);
				m->setType(Logic::Control::STOP_PRIMARY_SKILL);
				break;
			case Input::Button::MIDDLE:
				//m->setType(Logic::Control::UNMIDDLE_CLICK);
				m->setType(Logic::Control::STOP_SECONDARY_SKILL);
				break;
			case Input::Button::BUTTON_3:
				m->setType(Logic::Control::UNBUTTON3_CLICK);
				break;

			default:
				return false;
			}
			_controlledAvatar->emitMessage(m);
			return true;
		}
		return false;

	} // mouseReleased

	//--------------------------------------------------------

	int CPlayerController::typeOfKey(TKey key)
	{
		if ( key.keyId == Input::Key::NUMBER1 || key.keyId == Input::Key::NUMBER2 || key.keyId == Input::Key::NUMBER3 || key.keyId == Input::Key::NUMBER4 || 
				key.keyId == Input::Key::NUMBER5 || key.keyId == Input::Key::NUMBER6)
		{		
			return 0;//Change weapon
		}
		else if (key.keyId == Input::Key::W || key.keyId == Input::Key::A || key.keyId == Input::Key::S || key.keyId == Input::Key::D 
			|| key.keyId == Input::Key::Q || key.keyId == Input::Key::E || key.keyId == Input::Key::SPACE )
		{
			return 1; //Movement
		}
		else if (key.keyId == Input::Key::Q || key.keyId == Input::Key::E)
		{
			return 2; //Habilities
		}
		else if (key.keyId == Input::Key::O || key.keyId == Input::Key::NUMBER6 || key.keyId == Input::Key::NUMBER7 
			|| key.keyId == Input::Key::NUMBER8 || key.keyId == Input::Key::ESCAPE)
		{
			return 3; //Other
		}
		return -1;
	} // typeOfKey

	//--------------------------------------------------------

	void CPlayerController::ChangeWeaponMessage(TKey key)
	{
		Logic::CMessageChangeWeapon *message=new Logic::CMessageChangeWeapon();

		switch(key.keyId)
		{
		case Input::Key::NUMBER1:
			message->setWeapon(0);
			break;
		case Input::Key::NUMBER2:
			message->setWeapon(1);
			break;
		case Input::Key::NUMBER3:
			message->setWeapon(2);
			break;
		case Input::Key::NUMBER4:
			message->setWeapon(3);
			break;
		case Input::Key::NUMBER5:
			message->setWeapon(4);
			break;
		case Input::Key::NUMBER6:
			message->setWeapon(5);
			break;
		}

		_controlledAvatar->emitMessage(message);
	}//EmitMessageChangeWeapon

	//--------------------------------------------------------

	void CPlayerController::MovementMessage(TKey key)
	{
		Logic::CMessageControl *m=new Logic::CMessageControl();
		unsigned int iDiffTime = clock() - m_iLastTime;
		switch(key.keyId)
		{
			case Input::Key::W:
				if ((iDiffTime < MAX_TIME_DOBULE_PUSH)	&& (m_eLastMove == WALK))	
				{
					std::cout << "Salto adelante!!!" << iDiffTime << std::endl;
					m->setType(Logic::Control::DODGE_FORWARD);
				}
				else
				{
					std::cout << "Walk" << iDiffTime  << std::endl;
					m->setType(Logic::Control::WALK);
				}
				m_iLastTime=clock();
				m_eLastMove = WALK;
				break;
			case Input::Key::S:
				if ((iDiffTime < MAX_TIME_DOBULE_PUSH)	&& (m_eLastMove == WALKBACK))	
				{
					std::cout << "Salto atr�s!!!" << iDiffTime << std::endl;
					m->setType(Logic::Control::DODGE_BACKWARDS);
				}
				else
				{
					std::cout << "Walkback " << iDiffTime  << std::endl;
					m->setType(Logic::Control::WALKBACK);				
				}
				m_iLastTime=clock();
				m_eLastMove = WALKBACK;
				break;
			case Input::Key::A:
				if ((iDiffTime < MAX_TIME_DOBULE_PUSH)	&& (m_eLastMove == LEFT))	
				{
					std::cout << "SALTO IZQUIERDA!!!" << iDiffTime << std::endl;
					m->setType(Logic::Control::SIDEJUMP_LEFT);
				}
				else
				{
					std::cout << "Stafe left - Time" << iDiffTime  << std::endl;
					m->setType(Logic::Control::STRAFE_LEFT);
				}
				m_eLastMove = LEFT;
				m_iLastTime=clock();
				break;
			case Input::Key::D:
				if ((iDiffTime < MAX_TIME_DOBULE_PUSH) && (m_eLastMove == RIGHT))
				{
					std::cout << "SALTO DERECHA!!!" << iDiffTime  << std::endl;
					m->setType(Logic::Control::SIDEJUMP_RIGHT);
				}
				else
				{
					std::cout << "Stafe right - Time" <<  iDiffTime << std::endl;
					m->setType(Logic::Control::STRAFE_RIGHT);
				}
				m_iLastTime=clock();
				m_eLastMove = RIGHT;
				break;
			case Input::Key::SPACE:
				m->setType(Logic::Control::JUMP);
				break;
			break;
		}

		_controlledAvatar->emitMessage(m);
	}//EmitMessageMovement

	//--------------------------------------------------------

	void CPlayerController::HabilityMessage(TKey key)
	{
		Logic::CMessageControl *m=new Logic::CMessageControl();
		switch (key.keyId)
		{
			case Input::Key::Q:
				m->setType(Logic::Control::USE_PRIMARY_SKILL);
			break;
			case Input	::Key::E:
				m->setType(Logic::Control::USE_SECONDARY_SKILL);
			break;
		}
		_controlledAvatar->emitMessage(m);
	}//HabilityMessage

	//--------------------------------------------------------

	void CPlayerController::OtherMessages(TKey key)
	{
		switch (key.keyId)
		{					
			case Input::Key::NUMBER7:	
			{
				Logic::CMessageAddLife *message2=new Logic::CMessageAddLife();
				message2->setAddLife(20);
				_controlledAvatar->emitMessage(message2);
			}
			break;
			case Input::Key::NUMBER8:
			{
				Logic::CMessageAddShield *message3=new Logic::CMessageAddShield();
				message3->setAddShield(20);
				_controlledAvatar->emitMessage(message3);
			}
			break;
			case Input::Key::O:
			{
				Logic::CMessageHudDebug *m2=new Logic::CMessageHudDebug();
				_controlledAvatar->emitMessage(m2);
			}
			break;
			case Input::Key::ESCAPE:// esto debe desaparecer en el futuro
				std::cout << "escape pulsado" << std::endl;
				//return false;
			break;
		}

	}//EmitOtherMessages
} // namespace Input