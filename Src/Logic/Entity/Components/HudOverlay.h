/***

@see Logic::CHudOverlay
@see Logic::IComponent

@author Antonio Jesus
@date Febrero, 2013
*/
#ifndef __Logic_HudOverlay_H
#define __Logic_HudOverlay_H

#include "Graphics/Server.h"
#include "Graphics/Entity.h"
#include "Logic/Entity/Component.h"
#include "WeaponType.h"


/*
#include <OgreOverlay.h>
#include <OgreOverlayContainer.h>
#include <OgreTextAreaOverlayElement.h>
*/

//declaraci�n de la clase
namespace Logic 
{
/**
	Este componente controla el hud, mediante overlays
    @ingroup logicGroup

	@author Antonio Jesus Narvaez
	@date Febrero, 2013
*/
	class CHudOverlay : public IComponent
	{
		DEC_FACTORY(CHudOverlay);

	public:

		/**
		Constructor por defecto; en la clase base no hace nada.
		*/
		CHudOverlay() : IComponent(), _health(0), _shield(0), _ammo(1), _actualWeapon(0), _numWeapons(0), _panelMira(0), _spawnTime(0), _acumSpawn(0),
			_overlayPlay(0), _overlayDie(0), _panelDie(0), _textAreaDie(0), _overlayDebug(0), _panelDebug(0), _textAreaDebug(0), 
			_overlayLocationImpact(0), _overlayPanelLocationImpact(0), _acumDebug(0), temporal(0)  {
			
			for(int i=0;i<3;++i){
				_panelElementsText[i]=1;
			}
			 _overlayPlay = NULL;
			_overlayDie = NULL;
			_panelDie = NULL;
			_textAreaDie = NULL;
			for(int i=0;i<3;++i){
				_panelElements[i] = NULL;
				_panelElementsTextArea[i] = NULL;
			}
			_panelMira = NULL;
			for(int i = 0; i<7; ++i){

				_panelWeapon[i] = NULL;

				keysPanelWeapon[i] = NULL;

				for(int j=0;j<3;++j){
					_weaponsBox[i][j] = NULL;
				}
			}
			_overlayDebug = NULL;
			_panelDebug = NULL;
			_textAreaDebug = NULL;

			_overlayLocationImpact = NULL;

			_overlayPanelLocationImpact = NULL;

			_dispersionTime = 0;
			_resetMirilla = false;
		}
		

		virtual ~CHudOverlay();
		/**
		Inicializaci�n del componente usando la descripci�n de la entidad que hay en 
		el fichero de mapa.
		*/
		virtual bool spawn(CEntity* entity, CMap *map, const Map::CEntity *entityInfo);

		/**
		Este componente s�lo acepta mensajes de tipo DAMAGED.
		*/
		virtual bool accept(const std::shared_ptr<CMessage>& message);

		/**
		Al recibir un mensaje de tipo DAMAGED la vida de la entidad disminuye.
		*/
		virtual void process(const std::shared_ptr<CMessage>& message);
		
		


		virtual void onDeactivate();

		void changeMaterialActualWeapon(const std::string &materialName);
	protected:

		/**
		M�todo llamado en cada frame que actualiza el estado del componente.
		<p>
		Lo uso para cambiar los valores del debug

		@param msecs Milisegundos transcurridos desde el �ltimo tick.
		*/
		virtual void onTick(unsigned int msecs);

		int _numWeapons;

		
		enum eSkillIndex {PRIMARY_SKILL, SECONDARY_SKILL, NONE };
		enum eOverlayWeaponState {ACTIVE, NO_AMMO, NO_WEAPON };
		enum eOverlayElements {HEALTH, SHIELD, AMMO };


		void hudLife(int health);
		void hudShield(int shield);
		void hudWeapon(int ammo, int weapon);
		void hudAmmo(int ammo, int weapon);
		void hudSpawn(int spawn);
		void hudDispersion();

		void hudDeath();
		void hudRespawn();

		void hudParticle(const std::string &nameParticle);

		void hudDebug();

		void hudDebugData(const std::string &key, const std::string &value);

		/**
		Se usa para mostrar el localizador de impacto y setearle una posicion.

		@param radianAngle, angulos en radianes que se girara, sentido anti horario
		*/
		void hudDirectionImpact(float radianAngle);

		/**
		Establece el tama�o de la mirila, para realizar la secuancia de disparo y su feedback de perdida de punteria.

		@param width, establece el ancho de la mirilla en pantalla.
		@param height, establece el alto de la mirilla en pantalla.
		*/
		void hudSizeCrossfire(float width, float height);

		std::string toText(eSkillIndex skill);
		std::string toText(eOverlayWeaponState state);
		std::string toText(eOverlayElements element);


		int _health;
		int _shield;
		int _ammo;
		int _actualWeapon;

		Graphics::CServer* _server;
		
		/** 
		overlay principal, albergar� los paneles de armas, y el inidicador de vida, municion y armadura
		*/
		Graphics::COverlay *_overlayPlay;

		/** 
		Overlay que se mostrara cuando se muere el usuario
		*/
		Graphics::COverlay *_overlayDie;
		
		/** 
		Overlay con el panel
		*/
		Graphics::COverlay* _panelDie;
		/**
		Texto que se muestra cuando el jugador se muere
		*/
		Graphics::COverlay *_textAreaDie;

		/**
		Overlays de los siguiente elementos, HEALTH, SHIELD, AMMO
		*/
		Graphics::COverlay *_panelElements[3];
		/** 
		OverlaysText  de los siguiente elementos, HEALTH, SHIELD, AMMO
		*/
		Graphics::COverlay *_panelElementsTextArea[3];
		/**
		Textos que se muestran por pantalla en los overlaysElements
		*/
		int _panelElementsText[3];

		/**
		Maneja el overlay de la mira.
		*/
		Graphics::COverlay *_panelMira;

		/**
		Maneja el overlay de la mira que se mueve, es decir, que aumenta cuanto mas dispara.
		*/
		Graphics::COverlay *_panelMiraMovible;

		/** 
		Gestiona los paneles de armas basicas
		*/
		Graphics::COverlay *_panelWeapon[7];

		/** 
		Panel de las teclas de los paneles
		*/
		Graphics::COverlay *_panelWeaponKey[7];

		/** 
		tiene las teclas que)
		*/
		char keysPanelWeapon[7];

		/**
		Gestion los paneles de las armas (6) en este caso, en cada uno de sus estados: ACTIVE, NO_AMMO, NO_WEAPON
		*/
		Graphics::COverlay *_weaponsBox[7][3];
		
		/**
		Aqui se guardara el panel de localizador de impacto para el feedback de las colisiones
		*/
		Graphics::COverlay *_overlayLocationImpact;
		/**
		Aqui se guardara el panel de localizador de impacto para el feedback de las colisiones
		*/
		Graphics::COverlay *_overlayPanelLocationImpact;

		/**
		objetos para debug
		*/
		Graphics::COverlay *_overlayDebug;
		Graphics::COverlay *_panelDebug;
		Graphics::COverlay *_textAreaDebug;
		std::map<std::string, std::string> _textDebug;
		std::stringstream _sDebug;
		int _acumDebug;

		int _spawnTime;
		int _acumSpawn;

		int _contadorLocalizadorImpacto;

		float _sizeCrossFireX;
		float _sizeCrossFireY;
		
		/*
		float _dimCrossX;
		float _dimCrossY;
		float _actualDimCrossX;
		float _actualDimCrossY;
		unsigned int _timerMinigunCrossFire;
		bool _activeTimerMinigunCrossFire;
		*/

		int		_dispersionTime;

		float	_dispersionWidth;
		float	_dispersionHeight;

		bool	_resetMirilla;


		float temporal;
	}; // class CHudOverlay

	REG_FACTORY(CHudOverlay);

} // namespace Logic

#endif // __Logic_Life_H