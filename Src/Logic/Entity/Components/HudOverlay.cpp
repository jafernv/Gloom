﻿/**
@file HudOverlay.cpp

Contiene todo lo que respecta al hud del jugador
 
@see Logic::CLife
@see Logic::IComponent

@author Antonio Jesus Narvaez
@date Enero?, 2013
*/

#include "HudOverlay.h"

#include "Logic/Entity/Entity.h"
#include "Logic/Maps/Map.h"
#include "Map/MapEntity.h"
#include "Application/BaseApplication.h"


#include "Graphics/Particle.h"

#include "Graphics/Server.h"
#include "Graphics/Camera.h"
#include "Graphics/Scene.h"
#include "Graphics/Entity.h"
#include "Graphics/Scene.h"
#include "Graphics/Overlay.h"
#include "Graphics/StaticEntity.h"

/*
#include <OgreOverlayManager.h>
#include <OgreSceneManager.h>
#include <OgreOverlayContainer.h>
#include <OgreColourValue.h>
#include <OgreOverlay.h>
#include <OgreTextAreaOverlayElement.h>
#include <OgreEntity.h>
*/

#include "Logic/Messages/Message.h"
#include "Logic/Messages/MessageHudLife.h"
#include "Logic/Messages/MessageHudShield.h"
#include "Logic/Messages/MessageHudAmmo.h"
#include "Logic/Messages/MessageHudWeapon.h"
#include "Logic/Messages/MessageHudSpawn.h"
#include "Logic/Messages/MessageChangeMaterialHudWeapon.h"
#include "Logic/Messages/MessageChangeWeaponGraphics.h"
#include "Logic/Server.h"
#include "Logic/Messages/MessageHudDispersion.h"

#include "Logic/Messages/MessageHudDebug.h"
#include "Logic/Messages/MessageHudDebugData.h"

#include "Logic/Messages/MessageImpact.h"

namespace Logic 
{
	IMP_FACTORY(CHudOverlay);
	
	CHudOverlay::~CHudOverlay(){

		if(_overlayPlay)
			delete _overlayPlay;
		if(_overlayDie)
			delete _overlayDie;
		if(_textAreaDie)
			delete _textAreaDie;
		for(int i=0;i<3;++i){
			if(_panelElements[i])
				delete _panelElements[i];
			if(_panelElementsTextArea)
				delete _panelElementsTextArea[i];
		}
		if(_panelMira)
			delete _panelMira;
		for(int i=0;i<7;++i){
			if(_panelWeapon[i])
				delete _panelWeapon[i];	
			if(_panelWeaponKey[i]){
				delete _panelWeaponKey[i];	
			}
			for(int j=0;j<3;++j){
				if(_weaponsBox[i][j])
					delete _weaponsBox[i][j];
			}
		}
		if(_overlayDebug)
			delete _overlayDebug;
		if(_panelDebug)
			delete _panelDebug;
		if(_textAreaDebug)
			delete _textAreaDebug;
		if(_overlayLocationImpact)
			delete _overlayLocationImpact;
		if (_overlayPanelLocationImpact)
			delete _overlayPanelLocationImpact;
	}

	//---------------------------------------------------------

	bool CHudOverlay::spawn(CEntity *entity, CMap *map, const Map::CEntity *entityInfo)
	{
		if(!IComponent::spawn(entity,map,entityInfo))
			return false;

		// Pongo Q Q para que salga esa tecla, todo eto cambiara cuando se tenga el hud definitivo.
		char keysAux[7] = {'1','2','3','4','5','Q','Q'};

		for (int i =0 ; i<7;++i){
			keysPanelWeapon[i] = keysAux[i];
		}
		Graphics::CScene* scene = map->getScene();
		//_numWeapons = entityInfo->getIntAttribute("numWeapons");
	
		_server = Graphics::CServer::getSingletonPtr();

		// Divido la pantalla en cuadriculas, leo la cantidad de estas en pantalla
		int hudScreenWidth = entityInfo->getIntAttribute("hudScreenWidth");
		int hudScreenHeight = entityInfo->getIntAttribute("hudScreenHeight");

		// 
		int hudPanelInitialPositionX = entityInfo->getIntAttribute("hudPanelInitialPositionX");
		int hudPanelInitialPositionY = entityInfo->getIntAttribute("hudPanelInitialPositionY");
		float hudPanelSizeX = entityInfo->getIntAttribute("hudPanelSizeX");
		float hudPanelSizeY = entityInfo->getIntAttribute("hudPanelSizeY");

		// obtengo el tamaño de la pantalla.
		float height = _server->getHeight();
		float width = _server->getWidth();

		float relativeWidth = width/hudScreenWidth;
		float relativeHeight = height/hudScreenHeight;

		///////////////////////////////////Todo esto para la mira
         // Create an overlay
		_overlayPlay = _server->createOverlay( "_overlayPlay", scene );		
         
		// Creo panel de Mira
		_panelMira = _server->createOverlay("Mira", scene, "Panel" );
		_sizeCrossFireX = entityInfo->getFloatAttribute("hudCrossX");
		_sizeCrossFireY = entityInfo->getFloatAttribute("hudCrossY");
		float positionCrossFireX = 0.5f-((_sizeCrossFireX/2)*0.01f) ;
		float positionCrossFireY = 0.5f-((_sizeCrossFireY/2)*0.01f) ;
        _panelMira->setPosition( positionCrossFireX,positionCrossFireY);
		_panelMira->setDimensions( _sizeCrossFireX*0.01, _sizeCrossFireY*0.01);
        _panelMira->setMaterial("hudMira3");

		// overlay para controlar las teclas
			Graphics::COverlay *textAreaDummy = _server->createOverlay("dummy", scene, "TextArea");
			textAreaDummy->setMetricsMode("pixel");
			textAreaDummy->setPosition(200, 200);
			textAreaDummy->setDimensions(0, 0);
			textAreaDummy->setText(" ");
			// hardcodea el tamaño y tipo de la fuente
			textAreaDummy->setTextSize(16);
			textAreaDummy->setFont("fuenteSimple");
			_panelMira->addChild(textAreaDummy);
		_overlayPlay->add2D( _panelMira );

		///////////////////////////////////Esto para la mira dinamica
		_panelMiraMovible = _server->createOverlay("Mira dinamica", scene, "Panel" );
        _panelMiraMovible->setPosition( positionCrossFireX,positionCrossFireY);
		_panelMiraMovible->setDimensions( _sizeCrossFireX*0.01, _sizeCrossFireY*0.01 );
        _panelMiraMovible->setMaterial("hudMira4");
		_overlayPlay->add2D(_panelMiraMovible);


         // Add the panel to the overlay
		int x = hudPanelInitialPositionX;
		int y = hudPanelInitialPositionY;

		// Aqui me creo los cuadros para cada arma
		for(int i=WeaponType::eSOUL_REAPER; i< WeaponType::eSIZE; ++i){

			WeaponType::Enum current = (WeaponType::Enum)i;
			std::string currentOnText = WeaponType::toString(current);
		
			// Primero me creo el panel que estaba debajo
			_panelWeapon[current] = _server->createOverlay( "PanelWeapon" + currentOnText, scene, "Panel");
			_panelWeapon[current]->setMetricsMode("pixel");
			_panelWeapon[current]->setPosition( x*relativeWidth, y*relativeHeight);
			_panelWeapon[current]->setDimensions( relativeWidth*hudPanelSizeX, relativeHeight*hudPanelSizeX );
			_panelWeapon[current]->setMaterial("cuadroArmas");
	
			// overlay para controlar las teclas
			_panelWeaponKey[current] = _server->createOverlay(currentOnText+"_Key", scene, "TextArea");
				
			_panelWeaponKey[current]->setMetricsMode("pixel");
			_panelWeaponKey[current]->setPosition(1*relativeWidth, -1*relativeHeight);
			_panelWeaponKey[current]->setDimensions(relativeWidth, relativeHeight);
				std::stringstream sString;//create a stringstream
				sString << keysPanelWeapon[current];
			_panelWeaponKey[current]->setText(sString.str());

			// hardcodea el tamaño y tipo de la fuente
			_panelWeaponKey[current]->setTextSize(20);
			_panelWeaponKey[current]->setFont("fuenteSimple");

			_panelWeapon[current]->addChild(_panelWeaponKey[current]);
			//_panelWeapon[current]->setZBuffer(0);
				
			_overlayPlay->add2D( _panelWeapon[current]);


			//Aqui me creo el cuadro de ACTIVE, NO_AMMO, NO_WEAPON
			for(int j=0; j<=NO_WEAPON; ++j){
				eOverlayWeaponState currentState = (eOverlayWeaponState)j;
				std::string currentStateOnText = toText(currentState);
				
				_weaponsBox[current][currentState] = _server->createOverlay( "_weaponsBox["+currentOnText+"]["+currentStateOnText+"]", scene, "Panel");
				_weaponsBox[current][currentState]->setMetricsMode("pixel");
				_weaponsBox[current][currentState]->setPosition( x*relativeWidth, y*relativeHeight);
				_weaponsBox[current][currentState]->setDimensions( relativeWidth*hudPanelSizeX, relativeHeight*hudPanelSizeY );
				_weaponsBox[current][currentState]->setMaterial(currentOnText +"_"+ currentStateOnText);
				_weaponsBox[current][currentState]->setVisible(false);

				_overlayPlay->add2D( _weaponsBox[current][currentState] );
			}
			// Ya que todos los he puesto invisibles, el sin arma me lo pongo a visible
			_weaponsBox[current][NO_WEAPON]->setVisible(true);

			// Actualizo la posicion de los paneles
			x += hudPanelSizeX;
			// La posicion y no debe cambiar, ya que todo ira a la misma altura.
			//y += hudPanelSizeY;

		 } // For de gestion de cada arma.


		// Bucle para la gestion de la primary skill.

		x -= hudPanelSizeX*4;
		y -= hudPanelSizeY*1.5;
		
		// Pinto los cuadros de las habilidades PRIMARY_SKILL Y SECONDARY_SKILL
		for(int i=PRIMARY_SKILL + WeaponType::eSIZE; i< NONE + WeaponType::eSIZE; ++i){

			eSkillIndex current = (eSkillIndex)i;
			std::string currentOnText = toText((eSkillIndex)(i - WeaponType::eSIZE));
			// Primero me creo el panel que estaba debajo
			_panelWeapon[current] = _server->createOverlay( "PanelWeapon" + currentOnText, scene, "Panel");
			_panelWeapon[current]->setMetricsMode("pixel");
			_panelWeapon[current]->setPosition( x*relativeWidth, y*relativeHeight);
			_panelWeapon[current]->setDimensions( relativeWidth*hudPanelSizeX, relativeHeight*hudPanelSizeX );
			_panelWeapon[current]->setMaterial("cuadroArmas");

			_overlayPlay->add2D( _panelWeapon[current]);

			// overlay para controlar las teclas
			_panelWeaponKey[current] = _server->createOverlay(currentOnText+"_Key", scene, "TextArea");

			_panelWeaponKey[current]->setMetricsMode("pixel");
			_panelWeaponKey[current]->setPosition(1*relativeWidth, -1*relativeHeight);
			_panelWeaponKey[current]->setDimensions(relativeWidth, relativeHeight);
				std::stringstream sString;//create a stringstream
				sString << keysPanelWeapon[current];
			_panelWeaponKey[current]->setText(sString.str());

			// hardcodea el tamaño y tipo de la fuente
			_panelWeaponKey[current]->setTextSize(24);
			_panelWeaponKey[current]->setFont("fuenteSimple");

			_panelWeapon[current]->addChild(_panelWeaponKey[current]);

			//Aqui me creo el cuadro de ACTIVE, NO_AMMO, NO_WEAPON
			for(int j=0; j<=NO_WEAPON; ++j){
				eOverlayWeaponState currentState = (eOverlayWeaponState)j;
				std::string currentStateOnText = toText(currentState);
				
				_weaponsBox[current][currentState] = _server->createOverlay( "_weaponsBox["+currentOnText+"]["+currentStateOnText+"]", scene, "Panel");
				_weaponsBox[current][currentState]->setMetricsMode("pixel");
				_weaponsBox[current][currentState]->setPosition( x*relativeWidth, y*relativeHeight);
				_weaponsBox[current][currentState]->setDimensions( relativeWidth*hudPanelSizeX, relativeHeight*hudPanelSizeY );
				_weaponsBox[current][currentState]->setMaterial(currentOnText +"_"+ currentStateOnText);
				_weaponsBox[current][currentState]->setVisible(false);

				_overlayPlay->add2D( _weaponsBox[current][currentState] );
			}
			// Ya que todos los he puesto invisibles, el sin arma me lo pongo a visible
			_weaponsBox[current][NO_WEAPON]->setVisible(true);
		}

		// fin de la introduccion de la primary skill
		
	
		
		
		// Ahora me voy a crear otro bucle para los paneles.
		
		float hudPanelpositionX;
		float hudPanelpositionY;
		float hudPanelWidth;
		float hudPanelHeight;
		std::string hudPanelFont;
		int hudPanelFontSize;
		float hudPanelTextPositionXRelative;
		float hudPanelTextPositionYRelative;

		//bucle para pintar el LIFE, SHIELD, AMMO
		for(int i=0; i<3;++i){
		
			eOverlayElements current = (eOverlayElements)i;
			std::string currentOnText = toText(current);
	
			//Recopilo todos los datos necesarios para añadir los paneles
			hudPanelpositionX = entityInfo->getFloatAttribute("hudPanel"+currentOnText+"PositionX");
			hudPanelpositionY = entityInfo->getFloatAttribute("hudPanel"+currentOnText+"PositionY");
			hudPanelWidth = entityInfo->getFloatAttribute("hudPanel"+currentOnText+"Width");
			hudPanelHeight = entityInfo->getFloatAttribute("hudPanel"+currentOnText+"Height"); 
			hudPanelFont = entityInfo->getStringAttribute("hudPanel"+currentOnText+"Font");
			hudPanelFontSize = entityInfo->getIntAttribute("hudPanel"+currentOnText+"FontSize");
			hudPanelTextPositionXRelative = entityInfo->getFloatAttribute("hudPanel"+currentOnText+"TextPositionXRelative");
			hudPanelTextPositionYRelative = entityInfo->getFloatAttribute("hudPanel"+currentOnText+"TextPositionYRelative");
			
			_panelElements[current] = _server->createOverlay(currentOnText, scene,  "Panel" );
			_panelElements[current]->setMetricsMode("pixel");
			_panelElements[current]->setPosition( hudPanelpositionX*relativeWidth, hudPanelpositionY*relativeHeight);
			_panelElements[current]->setDimensions( relativeWidth * hudPanelWidth, relativeHeight * hudPanelHeight );
			_panelElements[current]->setMaterial("hud_"+currentOnText);

			_panelElementsTextArea[current] = _server->createOverlay(currentOnText+"_TextArea", scene, "TextArea");
				
				_panelElementsTextArea[current]->setMetricsMode("pixel");
				_panelElementsTextArea[current]->setPosition(relativeWidth*hudPanelTextPositionXRelative, hudPanelTextPositionYRelative);
				_panelElementsTextArea[current]->setDimensions(relativeWidth*2, relativeHeight*2);
					std::stringstream sString;//create a stringstream
					sString << _panelElementsText[current];//add number to the stream
				_panelElementsTextArea[current]->setText(sString.str());
				_panelElementsTextArea[current]->setTextSize(hudPanelFontSize);
				_panelElementsTextArea[current]->setFont(hudPanelFont);

				_panelElements[current]->addChild(_panelElementsTextArea[current]);

		_overlayPlay->add2D( _panelElements[current] );

		}
		// en el SOUL_REAPER (que es el arma inicial, debe de estar active)
		_weaponsBox[WeaponType::eSOUL_REAPER][ACTIVE]->setVisible(true);
		_weaponsBox[WeaponType::eSOUL_REAPER][NO_WEAPON]->setVisible(false);
		_panelElements[AMMO]->setVisible(false);
		
		//desactivo el cuadro estandar y activo el de arma activa.
		_panelWeapon[WeaponType::eSOUL_REAPER]->setMaterial("cuadroArmasInUse");

		//Pongo a false los visibles por si acaso no los pone solos
		for(int i = 1; i < WeaponType::eSIZE; ++i){
			_weaponsBox[i][NO_AMMO]->setVisible(false);
			_weaponsBox[i][ACTIVE]->setVisible(false);
		}
		
		//_overlayPlay->add3D("AK47", "AK47.mesh", new Vector3(0,0,-5));
		
		_overlayPlay->setVisible(true);

		 //////////////////////////////////////AQUI ME CREO EL OVERLAY PARA CUANDO SE MUERA

		_overlayDie = _server->createOverlay( "_overlayDie", scene );
		
		_panelDie = _server->createOverlay("panelDie", scene, "Panel");
		_panelDie->setMetricsMode("pixel");
		_panelDie->setPosition( 5*relativeWidth, 12*relativeHeight);
		_panelDie->setDimensions( relativeWidth*10, relativeHeight*10 );
        //panelDie->setMaterial("cuadroArmas");

		_textAreaDie = _server->createOverlay("_textAreaDie", scene, "TextArea");

				_textAreaDie->setMetricsMode("pixel");
				_textAreaDie->setPosition(0.5, 0.5);
				_textAreaDie->setDimensions(10, 10);

				_textAreaDie->setText("MUERE");
				_textAreaDie->setTextSize(46);
				_textAreaDie->setFont("fuenteSimple");
				
				_panelDie->addChild(_textAreaDie);

		_overlayDie->add2D( _panelDie );

		//_overlayDie->show();

		///////////////////////////////////////OVERLAY PARA LA LOCALIZACION DE IMPACTOS

		_overlayLocationImpact = _server->createOverlay("_overlayLocationImpact", scene);

		_overlayPanelLocationImpact = _server->createOverlay("_overlayPanelLocationImpact", scene, "Panel");
		_overlayPanelLocationImpact->setMetricsMode("pixel");
		_overlayPanelLocationImpact->setPosition((width*0.5)-128, (height*0.5)-128);
		_overlayPanelLocationImpact->setDimensions(256,256);
		_overlayPanelLocationImpact->setMaterial("localizadorImpacto");
		_overlayPanelLocationImpact->setVisible(true);

		_overlayLocationImpact->add2D(_overlayPanelLocationImpact);
		_overlayLocationImpact->setVisible(false);		

		//////////////////////////////////////AQUI ME CREO EL OVERLAY PARA EL DEBBUG

		_overlayDebug = _server->createOverlay( "_overlayDebug", scene );

		_panelDebug = _server->createOverlay("panelDebug", scene, "Panel");
		_panelDebug->setMetricsMode("pixel");

		//_panelDebug->setPosition(0,0);

		_panelDebug->setPosition( entityInfo->getFloatAttribute("hudPanelDebugPositionX")*relativeWidth, 
			entityInfo->getFloatAttribute("hudPanelDebugPositionY")*relativeHeight);
		
		_panelDebug->setDimensions( entityInfo->getFloatAttribute("hudPanelDebugWidth")*relativeWidth,
			entityInfo->getFloatAttribute("hudPanelDebugHeight")*relativeHeight );
        //_panelDebug->setMaterial("cuadroArmas");
		
		_panelDebug->setVisible(true);

		_textAreaDebug = _server->createOverlay("_textAreaDebug", scene, "TextArea");
				
				_textAreaDebug->setMetricsMode("pixel");
				_textAreaDebug->setPosition(0, 0);
				_textAreaDebug->setDimensions( entityInfo->getFloatAttribute("hudPanelDebugWidth")*relativeWidth,
			entityInfo->getFloatAttribute("hudPanelDebugHeight")*relativeHeight );

				_textAreaDebug->setText("Debug");
				_textAreaDebug->setTextSize(entityInfo->getIntAttribute("hudPanelDebugFontSize"));
				_textAreaDebug->setFont("fuenteSimple");

				//panelShield->addChild(textAreaHealth);
				
				_panelDebug->addChild(_textAreaDebug);

		_overlayDebug->add2D( _panelDebug );

		_overlayDebug->setVisible(false);

		return true;

	} // spawn
	//---------------------------------------------------------

	bool CHudOverlay::accept(const std::shared_ptr<CMessage>& message) {
		Logic::TMessageType msgType = message->getMessageType();

		return	msgType == Message::HUD_LIFE		|| 
				msgType == Message::HUD_SHIELD		|| 
				msgType == Message::HUD_AMMO		|| 
				msgType == Message::HUD_WEAPON		|| 
				msgType == Message::HUD_DISPERSION  ||
				msgType == Message::HUD_SPAWN		|| 
				msgType == Message::HUD_DEBUG		|| 
				msgType == Message::HUD_DEBUG_DATA	||
				msgType == Message::CHANGE_MATERIAL_HUD_WEAPON ||
				msgType == Message::IMPACT			||
				msgType == Message::HUD_DISPERSION	;

	} // accept
	//---------------------------------------------------------

	void CHudOverlay::process(const std::shared_ptr<CMessage>& message) {
		switch( message->getMessageType() ) {
			case Message::HUD_LIFE: {
				std::shared_ptr<CMessageHudLife> hudLifeMsg = std::static_pointer_cast<CMessageHudLife>(message);
				hudLife( hudLifeMsg->getLife() );
				break;
			}
			case Message::HUD_SHIELD: {
				std::shared_ptr<CMessageHudShield> hudShieldMsg = std::static_pointer_cast<CMessageHudShield>(message);
				hudShield( hudShieldMsg->getShield() );
				break;
			}
			case Message::HUD_AMMO: {
				std::shared_ptr<CMessageHudAmmo> hudAmmoMsg = std::static_pointer_cast<CMessageHudAmmo>(message);
				hudAmmo( hudAmmoMsg->getAmmo(), hudAmmoMsg->getWeapon() );
				break;
			}
			case Message::HUD_WEAPON: {
				std::shared_ptr<CMessageHudWeapon> hudWeaponMsg = std::static_pointer_cast<CMessageHudWeapon>(message);
				hudWeapon( hudWeaponMsg->getAmmo(), hudWeaponMsg->getWeapon() );
				break;
			}
			case Message::HUD_SPAWN: {
				std::shared_ptr<CMessageHudSpawn> hudSpawnMsg = std::static_pointer_cast<CMessageHudSpawn>(message);
				_spawnTime = hudSpawnMsg->getTime();
				_acumSpawn=3000;
				if(_spawnTime==0){
					hudRespawn();
				}else{
					hudDeath();
				}
				break;
			}
			case Message::HUD_DEBUG: {
				hudDebug();
				break;
			}
			case Message::HUD_DEBUG_DATA: {
				std::shared_ptr<CMessageHudDebugData> hudDebugDataMsg = std::static_pointer_cast<CMessageHudDebugData>(message);
				hudDebugData( hudDebugDataMsg->getKey(), hudDebugDataMsg->getValue() );
				break;
			}
			case Message::CHANGE_MATERIAL_HUD_WEAPON: {
				std::shared_ptr<CMessageChangeMaterialHudWeapon> chgMatMsg = std::static_pointer_cast<CMessageChangeMaterialHudWeapon>(message);
				changeMaterialActualWeapon( chgMatMsg->getMaterialName() );
				break;
			}
			case Message::IMPACT: {
				std::shared_ptr<CMessageImpact> impMes = std::static_pointer_cast<CMessageImpact>(message);
				hudDirectionImpact( impMes->getDirectionImpact() );
				break;
			}
			case Message::HUD_DISPERSION: {
				std::shared_ptr<CMessageHudDispersion> hudDisp = std::static_pointer_cast<CMessageHudDispersion>(message);
				_dispersionTime = hudDisp->getTime();
				_resetMirilla = hudDisp->getReset();

				if (!_resetMirilla)
				{
					_dispersionWidth = hudDisp->getHeight();
					_dispersionHeight = hudDisp->getWidth();
				}
				//printf("\n %f , %f", _dispersionWidth, _dispersionHeight);
				//Si es 0, es para resetear a la posición inicial
				/*
				if (_dispersionTime == 0)
					hudSizeCrossfire(_dispersionHeight, _dispersionWidth);
				*/
				//hudDispersion();
				//hudSizeCrossfire(hudDisp->getHeight(), hudDisp->getWidth());
				break;
			}
		}

	} // process
	//-------------------------------------------------------

	void CHudOverlay::onTick(unsigned int msecs)
	{

		temporal += msecs;

		if(_overlayLocationImpact->isVisible()){
			//dura 20 ticks
			++_contadorLocalizadorImpacto;
			if(_contadorLocalizadorImpacto >= 20){
				_overlayLocationImpact->setVisible(false);
			}
		}

		if(_overlayDie->isVisible()){
			_acumSpawn += msecs;
			if(_acumSpawn>1000){
				hudSpawn((--_spawnTime));
				_acumSpawn = 0;
				/////////////////////////////////////////////////////////////////////////////////////
				////////	Borrar en un futuro, espero que el server no llegue a -5		/////////
				/////////////////////////////////////////////////////////////////////////////////////
				if(_spawnTime<-5)
					hudRespawn();
			}
		}
		if(_overlayDebug->isVisible())
		{
			_sDebug.str("");
			_sDebug.clear();
			_acumDebug+=msecs;
			// han pasado el tiempo para actualizar el fps
			if(_acumDebug>200){		
				std::stringstream aux;
				aux << 1000.0f/(float)msecs;
				hudDebugData("FPS", aux.str());
				_acumDebug=0;
			}
			for (std::map<std::string,std::string>::iterator it=_textDebug.begin(); it!=_textDebug.end(); ++it){
				_sDebug << it->first << " => " << it->second << '\n';
			}
			std::shared_ptr<CMessageHudDebugData> m = std::make_shared<CMessageHudDebugData>();
			m->setKey("Posicion");
			m->setValue(_entity->getPosition());
			_entity->emitMessage(m);
		}
		_textAreaDebug->setText(_sDebug.str());

		//Dispersión
		if ((_dispersionTime > 0) || (_resetMirilla))
		{
			_dispersionTime -= msecs;
			hudDispersion();
		}
	}

	void CHudOverlay::hudLife(int health){
		_panelElementsText[HEALTH]= health;
		std::stringstream sHealth;
		sHealth << health;
		_panelElementsTextArea[HEALTH]->setText(sHealth.str());
	}
	//-------------------------------------------------------

	void CHudOverlay::hudShield(int shield){
		_panelElementsText[SHIELD] = shield;
		std::stringstream sShield;
		sShield << shield;
		_panelElementsTextArea[SHIELD]->setText(sShield.str());
	}
	//-------------------------------------------------------

	void CHudOverlay::hudAmmo(int ammo, int weapon){

		if(weapon == _actualWeapon)
		{
			_panelElementsText[AMMO] = ammo;
			if(ammo != 0){
				if(!_weaponsBox[_actualWeapon][ACTIVE]->isVisible())
				{
					_weaponsBox[_actualWeapon][NO_AMMO]->setVisible(false);
					_weaponsBox[_actualWeapon][ACTIVE]->setVisible(true);
				}
			}else{
				if(_weaponsBox[_actualWeapon][ACTIVE]->isVisible())
				{
					_weaponsBox[_actualWeapon][ACTIVE]->setVisible(false);
					_weaponsBox[_actualWeapon][NO_AMMO]->setVisible(true);
				}
			}//fin else	ammo!=0
			std::stringstream sAmmo;
			sAmmo << ammo;
			_panelElementsTextArea[AMMO]->setText(sAmmo.str());
		}//fin weapon == _actualweapon
		else{
			if(!_weaponsBox[weapon][ACTIVE]->isVisible()){
				_weaponsBox[weapon][ACTIVE]->setVisible(true);
				_weaponsBox[weapon][NO_WEAPON]->setVisible(false);
				_weaponsBox[weapon][NO_AMMO]->setVisible(false);
			}
		}
	}
	//-------------------------------------------------------

	void CHudOverlay::hudWeapon(int ammo, int weapon){

		//hudParticle("partilcle");
		//if(weapon != _actualWeapon && _actualWeapon != 0)
		if(weapon != _actualWeapon)
		{
			_panelWeapon[_actualWeapon]->setMaterial("cuadroArmas");
			_panelWeapon[weapon]->setMaterial("cuadroArmasInUse");
			// cambio el arma activa por la recien pulsada
			_actualWeapon = weapon;

			_weaponsBox[_actualWeapon][ACTIVE]->setVisible(true);
			_weaponsBox[_actualWeapon][NO_AMMO]->setVisible(false);
			_weaponsBox[_actualWeapon][NO_WEAPON]->setVisible(false);

			if((WeaponType::Enum)_actualWeapon == WeaponType::eSOUL_REAPER){
				_panelElements[AMMO]->setVisible(false);
			}else{
				_panelElements[AMMO]->setVisible(true);
			}
			if((WeaponType::Enum)_actualWeapon == WeaponType::eMINIGUN){
				// puesto a hierro por ahora :D
				hudSizeCrossfire(9,8); //HERRERIA
			}else{
				hudSizeCrossfire(_sizeCrossFireX,_sizeCrossFireY);
			}			
		}
		hudAmmo(ammo, weapon);
	}
	//-------------------------------------------------------

	void CHudOverlay::hudSpawn(int spawmTime){
		std::stringstream sSpawn;
		sSpawn << "OWNED \n Tiempo de respawn: " << spawmTime;
		_textAreaDie->setText(sSpawn.str());
	}
	//-------------------------------------------------------

	void CHudOverlay::hudDeath(){
		if(_overlayPlay->isVisible()){
			_overlayPlay->setVisible(false);
		}
		if(!_overlayDie->isVisible())
			_overlayDie->setVisible(true);
	}
	//-------------------------------------------------------

	void CHudOverlay::hudRespawn(){
		_overlayDie->setVisible(false);
			//reset para volver a mostrar solo el arma inicial al hacer show
		for(int i=1; i<WeaponType::eSIZE;++i){
			_weaponsBox[i][NO_WEAPON]->setVisible(true);
			_weaponsBox[i][ACTIVE]->setVisible(false);
			_weaponsBox[i][NO_AMMO]->setVisible(false);
		}
		_overlayPlay->setVisible(true);
		_actualWeapon = WeaponType::eSOUL_REAPER;
	}
	//-------------------------------------------------------

	void CHudOverlay::hudParticle(const std::string &nameParticle){
		
		/*
		Graphics::CParticle *particle = new Graphics::CParticle("particle", "SmokeParticles", true);
		
		Graphics::COverlay *overlayParticle = _server->createOverlay( "_overlay3DParticle");
		overlayParticle->add3D(particle, new Vector3(0,0,-20));
		*/
	}
	//-------------------------------------------------------

	void CHudOverlay::hudDebug(){
		_overlayDebug->setVisible(!_overlayDebug->isVisible());
	}
	//-------------------------------------------------------

	void CHudOverlay::hudDebugData(const std::string &key, const std::string &value){
		if(_textDebug.find(key) == _textDebug.end()){
			std::pair<std::string, std::string> aux(key, value);
			_textDebug.insert(aux);
		}else{
			_textDebug.find(key)->second = value;
		}
	}
	//-------------------------------------------------------

	void CHudOverlay::changeMaterialActualWeapon(const std::string &materialName){
		if(materialName == "original"){
			WeaponType::Enum current;
			for(int i = 0; i < 6 ; ++i){
				current = (WeaponType::Enum)i;
			}
		}else{
			for(int i = 0; i < 6 ; ++i){
			}
		}
	}
	
	//-------------------------------------------------------

	void CHudOverlay::hudDirectionImpact(float radianAngle){
		_overlayLocationImpact->setRotation(radianAngle);
		_overlayLocationImpact->setVisible(true);
		_contadorLocalizadorImpacto = 0;
	}
	//-------------------------------------------------------
	
	void CHudOverlay::hudDispersion(){
		//Compruebo que no reste más de unos mínimos

		if (!_resetMirilla)
		{
			//Estamos disparando normal, no reseteando la mirilla
			//Comprobación del límite de pequeña que pueda ser
			if ((_dispersionHeight > 3.0f) && (_dispersionWidth > 4.0f))
			{
				_dispersionHeight -= 0.3f;
				_dispersionWidth -= 0.3f;			
			}
		}
		else
		{
			//Reseteamos mirilla
			//Es más pequeña, tengo que aumentarle
			if ((_dispersionHeight < 8.0f) && (_dispersionWidth < 9.0f))
			{
				//Aumento
				_dispersionHeight += 0.3f;
				_dispersionWidth += 0.3f;
				//Compruebo si con este aumento ya es más grande, para parar el reseteo de mirilla
				if ((_dispersionHeight >  8.0f) && (_dispersionWidth > 9.0f))				
				{
					_resetMirilla = false;
					_dispersionTime = 0; //por si aca
				}
			}
			else if ((_dispersionHeight > 8.0f) && (_dispersionWidth > 9.0f))			
			{
				//Aumento
				_dispersionHeight -= 0.4f;
				_dispersionWidth -= 0.4f;
				//Compruebo si con este aumento ya es más grande, para parar el reseteo de mirilla
				if ((_dispersionHeight < 8.0f) && (_dispersionWidth < 9.0f))
				{
					_resetMirilla = false;
					_dispersionTime = 0; //porsiaca
				}
			}
			else
			{
				//reset porque es igual
				_resetMirilla = false;
				_dispersionTime = 0; //porsiaca
			}

		}

		
		//std::cout << "DispersionHeight: " <<  _dispersionHeight << std::endl;
		hudSizeCrossfire(_dispersionHeight, _dispersionWidth);				
	}
	//-------------------------------------------------------

	void CHudOverlay::hudSizeCrossfire(float width, float height){
		/*
		width*=0.001;
		height*=0.001;		
		_panelMiraMovible->setPosition(_panelMiraMovible->getPositionX() - width*0.5, _panelMiraMovible->getPositionY() - height*0.5);
		_panelMiraMovible->setDimensions(_panelMiraMovible->getWidth() + width,_panelMiraMovible->getHeight() + height);
		*/
		if ((width < .0f) && (height < .0f))
		{
			//Si ancho y alto = -1; los reseteo a su valor inicial,
			//porque significa que hemos soltado el botón derecho y mandé estos valores.
			width = _sizeCrossFireX;
			height = _sizeCrossFireY;
		}
		_panelMiraMovible->setPosition(0.5f-((width/2)*0.01f), 0.5f-((height/2)*0.01f));
		_panelMiraMovible->setDimensions(width*0.01,height*0.01);  
	}
	//-------------------------------------------------------

	void CHudOverlay::onDeactivate(){
		_overlayPlay->setVisible(false);
		_overlayDie->setVisible(false);	
		_overlayDebug->setVisible(false);
		_overlayLocationImpact->setVisible(false);
	}
	//-------------------------------------------------------

	std::string CHudOverlay::toText(eSkillIndex skill){
		switch(skill){
			case PRIMARY_SKILL: return "primarySkill";
				break;
			case SECONDARY_SKILL: return "secondarySkill";
				break;
			default: return "";
			}
	}
	//-------------------------------------------------------

	std::string CHudOverlay::toText(eOverlayWeaponState state){
		switch(state){
			case ACTIVE: return "ACTIVE";
				break;
			case NO_AMMO: return "NO_AMMO";
				break;
			case NO_WEAPON: return "NO_WEAPON";
				break;
			default: return "";
			}
	}
	//-------------------------------------------------------

	std::string CHudOverlay::toText(eOverlayElements element){
		switch(element){
			case HEALTH: return "HEALTH";
				break;
			case SHIELD: return "SHIELD";
				break;
			case AMMO: return "AMMO";
				break;
			default: return "";
			}
	}
	//-------------------------------------------------------
} // namespace Logic