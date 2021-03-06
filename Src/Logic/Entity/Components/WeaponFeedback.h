//---------------------------------------------------------------------------
// WeaponFeedback.h
//---------------------------------------------------------------------------

/**
@file WeaponFeedback.h

@author Antonio Jesus Narv�ez Corrales
@author Francisco Aisa Garc�a
@date Mayo, 2013
*/

#ifndef __Logic_WeaponFeedback_H
#define __Logic_WeaponFeedback_H

#include "Logic/Entity/Component.h"
#include "WeaponType.h"

#include <string>

namespace Logic {

	// Forward declarations
	class CHudWeapons;

	/**
	@ingroup logicGroup

	@author Antonio Jesus Narv�ez Corrales
	@author Francisco Aisa Garc�a
	@date Mayo, 2013
	*/

	class IWeaponFeedback : public IComponent {
	public:

		
		// =======================================================================
		//                      CONSTRUCTORES Y DESTRUCTOR
		// =======================================================================

		/**
		Constructor parametrizado. Las clases hijas deber�n llamar a este constructor
		con el nombre de su arma.

		@param weaponName Nombre del arma implementada por la clase derivada.
		*/
		IWeaponFeedback(const std::string& weaponName);

		//__________________________________________________________________

		/** Destructor. */
		virtual ~IWeaponFeedback();


		// =======================================================================
		//                    METODOS HEREDADOS DE ICOMPONENT
		// =======================================================================

		/**
		Inicializaci�n del componente a partir de la informaci�n extraida de la entidad
		leida del mapa:
		<ul>
			<li><strong>MaxAmmo:</strong> Munici�n m�xima que puede llevar el arma. </li>
			<li><strong>ID:</strong> Identificador de arma. </li>
			<li><strong>physic_radius:</strong> Radio de la c�psula f�sica del personaje. </li>
			<li><strong>heightShoot:</strong> Altura desde donde sale el disparo. </li>
		</ul>

		@param entity Entidad a la que pertenece el componente.
		@param map Mapa L�gico en el que se registrar� el objeto.
		@param entityInfo Informaci�n de construcci�n del objeto le�do del fichero de disco.
		@return Cierto si la inicializaci�n ha sido satisfactoria.
		*/
		virtual bool spawn(CEntity* entity, CMap *map, const Map::CEntity *entityInfo);

		//__________________________________________________________________

		/** 
		Este componente acepta los siguientes mensajes:

		<ul>
			<li>CONTROL</li>
		</ul>
		
		@param message Mensaje a chequear.
		@return true si el mensaje es aceptado.
		*/
		virtual bool accept(const std::shared_ptr<CMessage>& message);

		//__________________________________________________________________

		/**
		M�todo virtual que procesa un mensaje.

		@param message Mensaje a procesar.
		*/
		virtual void process(const std::shared_ptr<CMessage>& message);

		/**
		Metodo virtual invocado cuando se desactiva la entidad
		*/
		virtual void onDeactivate();

		// =======================================================================
		//                          METODOS PROPIOS
		// =======================================================================


		/**
		M�todo virtual puro que debe ser implementado por las clases derivadas para
		especificar que ocurre al usar el disparo primario.
		*/
		virtual void primaryFire() = 0;

		//__________________________________________________________________
		
		/**
		M�todo virtual puro que debe ser implementado por las clases derivadas para
		especificar que ocurre al usar el disparo secundario.
		*/
		virtual void secondaryFire() = 0;

		//__________________________________________________________________

		/**
		Este m�todo es invocado cuando se deja de pulsar el bot�n de disparo
		primario.
		*/
		virtual void stopPrimaryFire() { };

		//__________________________________________________________________

		/**
		Este m�todo es invocado cuando se deja de pulsar el bot�n de disparo
		secundario.
		*/
		virtual void stopSecondaryFire() { };


		virtual void resetAmmo() {};
	protected:


		// =======================================================================
		//                          METODOS PROTEGIDOS
		// =======================================================================


		/**
		Emite el sonido pasado por par�metro.

		@param soundName Nombre del archivo de audio que queremos reproducir.
		@param loopSound true queremos reproducir el sonido en loop.
		@param play3d true si queremos que el sonido se reproduzca en 3d.
		@param streamSound true si queremos que el sonido se reproduzca en streaming. Util
		para ficheros muy grandes como por ejemplo la m�sica.
		@param stopSound true si queremos parar la reproducci�n del sonido.
		*/
		void emitSound(const std::string &soundName, bool loopSound, bool play3d, bool streamSound, bool stopSound);

		//__________________________________________________________________

		/**
		Pinta un decal dada una entidad y una posicion

		@deprecated Temporalmente est� implementada como un helper, pero
		en el futuro est� funci�n deber�a formar parte del componente gr�fico.

		@param pEntity Entidad sobre la que queremos pintar el decal.
		@param vPos Posici�n en la que queremos pintar el decal.
		*/
		void drawDecal(Logic::CEntity* pEntity, Vector3 vPos);

		/**
		Metodo que emite el mensaje que pintara la particula
		*/
		void emitParticle(bool primaryShoot = true);
		void emitParticle2(bool primaryShoot = true);

		// =======================================================================
		//                          MIEMBROS PROTEGIDOS
		// =======================================================================

		
		/** Enumerado que indica cual es el identificador de arma. */
		WeaponType::Enum _weaponID;

		/** Ruta del sonido del arma. */
		std::string _weaponSound;

		/** Nombre del arma con el formato: weapon + <nombre arma>.*/
		std::string _weaponName;

		CHudWeapons* _hudWeapon;

		bool _primaryFireIsActive;
		bool _secondaryFireIsActive;

		float _heightShoot;

		bool _ableToShoot;
		
		//@deprecated con el sistema actual de particulas
		std::string _primaryFireParticle;
		std::string _secondaryFireParticle;
		
		Vector3 _particlePosition;
		
		CEntity *_currentParticle;

	}; // class IWeaponFeedback

} // namespace Logic

#endif // __Logic_WeaponFeedback_H
