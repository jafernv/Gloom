/**
@file Message.h

Contiene el tipo de datos de un mensaje.

@see Logic::CMessage

@author Jos� Antonio Garc�a Y��ez
@author Francisco Aisa Garc�a
@author Rub�n Mulero Guerrero
@author Antonio Jes�s Narv�ez Corrales
*/

#ifndef __Logic_Message_H
#define __Logic_Message_H

#include "BaseSubsystems/Math.h"
#include "Net/Buffer.h"
#include "Logic/Entity/MessageFactory.h"

// Predeclaraciones
namespace Logic {
	class CEntity;
};

namespace Net {
	typedef unsigned char byte;
};

namespace Logic {
	/**
	Namespace para los tipos de mensajes posibles.
	*/
	namespace Message {
		enum TMessageType {
			UNASSIGNED					= 0xFFFFFFFF,
			SET_TRANSFORM				= 0x00000000,
			SET_ANIMATION				= 0x00000001,
			STOP_ANIMATION				= 0x00000002,
			CONTROL						= 0x00000003,
			FLASH						= 0x00000004,
			KINEMATIC_MOVE				= 0x00000005,
			TOUCHED						= 0x00000006,
			UNTOUCHED					= 0x00000007,
			SWITCH						= 0x00000008,
			DAMAGED						= 0x00000009,
			CHANGE_WEAPON				= 0x0000000A,//10
			CHANGE_WEAPON_GRAPHICS		= 0x0000000B,//11
			COLLISION_DOWN				= 0x0000000C,//12
			SPAWN_IS_LIVE				= 0x0000000D,//13
			HUD_LIFE					= 0x0000000E,//14
			HUD_SHIELD					= 0x0000000F,//15
			HUD_AMMO					= 0x00000010,//16
			HUD_WEAPON					= 0x00000011,//17
			ADD_LIFE					= 0x00000012,//18
			ADD_SHIELD					= 0x00000013,//19
			ADD_AMMO					= 0x00000014,//20
			ADD_WEAPON					= 0x00000015,//21
			PLAYER_DEAD					= 0x00000016,//22
			SET_PHYSIC_POSITION			= 0x00000017,//23
			HUD_SPAWN					= 0x00000018,//24
			CAMERA_TO_ENEMY				= 0x00000019,//25
			SYNC_POSITION				= 0x0000001A,//26
			WAKEUP						= 0x0000001B,//27
			SLEEP						= 0x0000001C,//28
			ACTIVATE					= 0x0000001D,//29
			DECAL						= 0x0000001E,//30
			HOUND_CHARGE				= 0x0000001F,//31
			ADDFORCEPLAYER				= 0x00000020,//32
			SIDE						= 0x00000021,//33
			ELEVATOR_INITIAL			= 0x00000022,//34
			ELEVATOR_FINAL				= 0x00000023,//35
			ADD_FORCE_PHYSICS			= 0x00000024,//36
			CONTACT_ENTER				= 0x00000025,//37
			PLAYER_SPAWN				= 0x00000026,//38
			HUD_DEBUG					= 0x00000027,//39
			CONTACT_EXIT				= 0x00000028,//40
			AUDIO						= 0x00000029,//41
			CHANGE_PLAYER_CLASS			= 0x0000002A,//42
			CHANGE_MATERIAL				= 0x0000002B,//43
			SET_REDUCED_DAMAGE			= 0x0000002C,//44
			REDUCED_COOLDOWN			= 0x0000002D,//45
			CREATE_PARTICLE				= 0x0000002E,//46
			HUD_DEBUG_DATA				= 0x0000002F,//47
			SET_OWNER					= 0x00000030,//48
			HUD							= 0x00000031,//49
			BLOCK_SHOOT					= 0x00000032,//50
			CHANGE_MATERIAL_HUD_WEAPON	= 0x00000033,//51
			CAMERA_OFFSET				= 0x00000034,//52
			CAMERA_ROLL					= 0x00000035,//53
			IMPACT						= 0x00000036,//54
			TRANSFORM_SNAPSHOT			= 0x00000037,//55
			POSITION_SNAPSHOT			= 0x00000038,//56
			HUD_DISPERSION				= 0x00000039,//57
			DAMAGE_AMPLIFIER			= 0x0000003A,//58
			PLAYER_SNAPSHOT				= 0x0000003B,//59
			CHANGE_GRAVITY				= 0x0000003C,//60
			PARTICLE_VISIBILITY			= 0x0000003D,//61
			PRIMARY_SHOOT				= 0x0000003E,//62
			SECONDARY_SHOOT				= 0x0000003F,//63
			PRIMARY_SPELL				= 0x00000040,//64
			SECONDARY_SPELL				= 0x00000041,//65
			ADD_SPELL					= 0x00000042,//66
			SPELL_HUNGRY				= 0x00000043,//67
			KILL_STREAK					= 0X00000044,//68
			PARTICLE_START				= 0x00000045,//69
			PARTICLE_STOP				= 0x00000046 //70
		};
	}

	/**
	Namespace para los tipos de mensajes de control posibles.
	*/
	namespace Control {
		enum ControlType {
			UNASSIGNED = 0xFFFFFFFF,
			
			// Movimiento
			WALK,
			WALKBACK,
			STOP_WALK,
			STOP_WALKBACK,
			STRAFE_LEFT,
			STRAFE_RIGHT,
			STOP_STRAFE_LEFT,
			STOP_STRAFE_RIGHT,
			CROUCH,
			STOP_CROUCH,

			// Salto
			STOP_JUMP,
			JUMP,
			SIDEJUMP_LEFT,
			SIDEJUMP_RIGHT,
			DODGE_FORWARD,
			DODGE_BACKWARDS,
			DODGE_RIGHT,
			DODGE_LEFT,

			// Raton
			MOUSE,
			LEFT_CLICK,
			RIGHT_CLICK,
			MIDDLE_CLICK,
			BUTTON3_CLICK,
			UNLEFT_CLICK,
			UNRIGHT_CLICK,
			UNMIDDLE_CLICK,
			UNBUTTON3_CLICK,
			
			// Habilidades
			USE_PRIMARY_SKILL,
			USE_SECONDARY_SKILL,
			STOP_PRIMARY_SKILL,
			STOP_SECONDARY_SKILL,

			// Hechizos
			USE_PRIMARY_SPELL,
			USE_SECONDARY_SPELL,
			STOP_PRIMARY_SPELL,
			STOP_SECONDARY_SPELL,
		};
	}

	/**
	Tipo copia para los mensajes. Por simplicidad.
	*/
	typedef Message::TMessageType TMessageType;

	/**
	Tipo copia para los mensajes de control. Por simplicidad.
	*/
	typedef Control::ControlType ControlType;


	/**
	Contiene la jerarquia de mensajes que implementaremos.
	@ingroup logicGroup
	@ingroup entityGroup

	@author Jose Antonio Garc�a Y��ez
	@author Francisco Aisa Garc�a
	@author Ruben Mulero Guerrero
	@date Enero, 2013
	@ingroup grupoEntidad
	*/
	class CMessage { // Abstracta
	public:
		inline TMessageType getMessageType() { return _type; }

		// Inicializa los mensajes a los valores por defecto
		CMessage(TMessageType t);
		virtual ~CMessage();

		/**
		M�todo virtual puro que debe ser implementado por los mensajes hijos para
		que se serialicen correctamente al ser enviados por la red. Si los mensajes
		heredan de una jerarqu�a indirecta deben llamar a los serializes de los padres.

		Nota: El buffer devuelto implementa copy-on-write as� que no os preocupeis por
		hacer m�s de un return.

		@return Un buffer con los datos del mensaje serializados.
		*/
		virtual Net::CBuffer serialize() = 0;

		/**
		M�todo virtual puro que debe ser implementado por los mensajes hijos para
		que se deserialicen correctamente al ser enviados por la red. Al igual que
		el serialize, si el mensaje hereda de una amplia jerarqu�a deber� llamar 
		a los deserializes de los padres para deserializar toda la informaci�n correctamente.
		*/
		virtual void deserialize(Net::CBuffer& buffer) = 0;

	protected:
		TMessageType _type;
	};

	/////////////////////////////////////////////////////////////
	// Macros para la adici�n de los componentes a la factor�a //
	// de componentes. //
	/////////////////////////////////////////////////////////////

	/**
	Macro para la declaraci�n de los m�todos necesarios para que
	la factoria cree nuevas instancias del tipo de componentes y
	para que el componente se registre en la factor�a.
	*/
	#define DEC_FACTORYMESSAGE(Class) \
	public: \
		/** \
		Crea un componente de la clase en la que se declara. \
		*/ \
		static CMessage* create(); \
		/** \
		Registra el componente de la clase en la factor�a. \
		*/ \
		static bool regist(); \

	/**
	Macro para la implementaci�n de los m�todos necesarios para que
	la factoria cree nuevas instancias del tipo de componentes y
	para que el componente se registre en la factor�a.
	*/
	#define IMP_FACTORYMESSAGE(Class) \
	CMessage* Class::create() \
	{ \
		CMessage* res = new Class(); \
		return res; \
	} \
	bool Class::regist() \
	{ \
		if (!CMessageFactory::getSingletonPtr()->has(#Class)) \
		{ \
			CMessageFactory::getSingletonPtr()->add(Class::create, #Class); \
		} \
		return true; \
	}

	/**
	Macro que invoca al m�todo que registra la clase en la factor�a.
	*/
	#define REG_FACTORYMESSAGE(Class) \
	static bool RegisteredFACTORYMESSAGE_##Class = Class::regist();

} // namespace Logic

#endif // __Logic_Message_H