//---------------------------------------------------------------------------
// Weapon.h
//---------------------------------------------------------------------------

/**
@file Weapon.h

Contiene la declaraci�n de la interfaz com�n
a todas las armas.

@author Antonio Jesus Narv�ez Corrales
@author Francisco Aisa Garc�a
@date Mayo, 2013
*/

#ifndef __Logic_Weapon_H
#define __Logic_Weapon_H

#include "Logic/Entity/Component.h"
#include "WeaponType.h"

#include <string>

namespace Logic {

	/**
	@ingroup logicGroup

	Interfaz com�n a todas las armas del juego. El objetivo
	de esta clase es asegurarse de que todas siguen una interfaz
	com�n y sencilla que facilite la implementaci�n y la comprensi�n
	de las armas.

	Debido a que el funcionamiento de cada arma es realmente 
	�nico, hemos preferido evitar generalizar en esta clase.

	IMPORTANTE: Decrementar la munici�n y emitir los sonidos
	de disparo es responsabilidad del cliente. Las animaciones
	de las armas tambien son responsabilidad del cliente.

	@author Antonio Jesus Narv�ez Corrales
	@author Francisco Aisa Garc�a
	@date Mayo, 2013
	*/

	class IWeapon : public IComponent {
	public:

		
		// =======================================================================
		//                      CONSTRUCTORES Y DESTRUCTOR
		// =======================================================================

		/**
		Constructor parametrizado. Las clases hijas deber�n llamar a este constructor
		con el nombre de su arma.

		@param weaponName Nombre del arma implementada por la clase derivada.
		*/
		IWeapon(const std::string& weaponName);

		//__________________________________________________________________

		/** Destructor. */
		virtual ~IWeapon();


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
		virtual void stopPrimaryFire() { /* Por defecto nada */ }

		//__________________________________________________________________

		/**
		Este m�todo es invocado cuando se deja de pulsar el bot�n de disparo
		secundario.
		*/
		virtual void stopSecondaryFire() { /* Por defecto nada */ }

		//__________________________________________________________________

		/**
		Incrementar la munici�n de un arma.

		@param weapon Identificador del arma.
		@param ammo Munici�n a incrementar.
		@param iAmCath Para el HUD.
		*/
		void addAmmo(int weapon, int ammo, bool iAmCatch);

		//__________________________________________________________________

		/**
		Resetea la munici�n.
		*/
		virtual void resetAmmo();

		//__________________________________________________________________

		/**
		M�todo virtual puro invocado cuando se amplifica el da�o del arma.
		El cliente es responsable de incrementar el da�o de su arma en 
		el porcentaje dado por par�metro.

		@param percentage Tanto por ciento del 1 al 100 en el que se incrementa
		el da�o del arma. Si su valor es 0, significa que debemos resetear el
		da�o del arma a su valor normal.
		*/
		virtual void amplifyDamage(unsigned int percentage) = 0;

		//__________________________________________________________________

		/**
		M�todo virtual invocado cuando se decrementa el tiempo de cooldown del arma.
		El cliente es responsable de decrementar los cooldowns de su arma en 
		el porcentaje dado por par�metro.

		@param percentage Tanto por ciento del 1 al 100 en el que se decrementa
		el cooldown del arma. Si su valor es 0, significa que debemos resetear
		los cooldowns del arma a su valor por defecto.
		*/
		virtual void reduceCooldown(unsigned int percentage) {};

		//__________________________________________________________________

		/**
		Pintar las particulas para el arma
		*/
		void particles();

		//__________________________________________________________________

		/**
		Bloquea el disparo si asi se indica.

		@param unable true si queremos bloquear el disparo.
		*/
		void ableToShoot(bool able) { _ableToShoot = able; }

	protected:


		// =======================================================================
		//                    METODOS HEREDADOS DE ICOMPONENT
		// =======================================================================

		

		/**
		Llamado cuando el arma pasa a ser activa.
		*/
		virtual void onAvailable();


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
		Decrementa la munici�n en base al par�metro dado y actualiza la munici�n
		que aparece en el HUD.

		@param Cantidad de munici�n a reducir. Por defecto es 1.
		*/
		void decrementAmmo(unsigned int ammoSpent = 1);

		//__________________________________________________________________

		/**
		Desde esta funci�n se env�a un mensaje al componente gr�fico de la entidad a la que se ha dado
		(pEntity) para que se pinte el decal ah�. Es decir, esta funci�n se encarga de enviar el mensaje.

		@param pEntity Entidad sobre la que queremos pintar el decal.
		@param vPos Posici�n en la que queremos pintar el decal.
		*/
		void drawDecal(Logic::CEntity* pEntity, Vector3 vPos);

		//Sobrecarga del m�todo apra indicar qu� arma ha hecho el decal
		void drawDecal(Logic::CEntity* pEntity, Vector3 vPos, int iWeapon);



		// =======================================================================
		//                          MIEMBROS PROTEGIDOS
		// =======================================================================




		/** Cuanta munici�n puede llevar este arma como m�ximo. */
		unsigned int _maxAmmo;
		
		/** Cuanta munici�n tenemos actualmente en este arma. */
		unsigned int _currentAmmo;
		
		/** Radio de la c�psula del personaje */
		float _capsuleRadius;
		
		/** Altura desde la que sale el disparo. */
		float _heightShoot;
		
		/** Enumerado que indica cual es el identificador de arma. */
		WeaponType::Enum _weaponID;

		/** Nombre del arma con el formato: weapon + <nombre arma>.*/
		std::string _weaponName;

		/** true si por alguna razon no podemos disparar. */
		bool _ableToShoot;

	}; // class IWeapon

} // namespace Logic

#endif // __Logic_Weapon_H
