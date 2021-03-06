/**
@file Graphics.cpp

Contiene la implementaci�n del componente que controla la representaci�n
gr�fica de la entidad.
 
@see Logic::CGraphics
@see Logic::IComponent

@author David Llans�
@date Agosto, 2010
*/

#include "Graphics.h"

#include "Logic/Entity/Entity.h"
#include "Logic/Maps/Map.h"
#include "Map/MapEntity.h"

#include "Graphics/Scene.h"
#include "Graphics/Entity.h"
#include "Graphics/StaticEntity.h"

#include "Logic/Messages/MessageActivate.h"
#include "Logic/Messages/MessageChangeMaterial.h"
#include "Logic/Messages/MessageDecal.h"


#include "Graphics/OgreDecal.h"

namespace Logic 
{
	IMP_FACTORY(CGraphics);
	
	//---------------------------------------------------------

	CGraphics::~CGraphics() 
	{
		if(_graphicsEntity)
		{
			_scene->removeEntity(_graphicsEntity);
			_graphicsEntity = 0;
		}

	} // ~CGraphics
	
	//---------------------------------------------------------

	bool CGraphics::spawn(CEntity *entity, CMap *map, const Map::CEntity *entityInfo) 
	{
		if(!IComponent::spawn(entity,map,entityInfo))
			return false;
		
		_scene = _entity->getMap()->getScene();

		if(entityInfo->hasAttribute("model"))
			_model = entityInfo->getStringAttribute("model");
		
		_graphicsEntity = createGraphicsEntity(entityInfo);
		if(!_graphicsEntity)
			return false;

		if(entityInfo->hasAttribute("scale")){

			//con el escale puedo escalar lo que quiera cuanto quiera. El mapa ya sale al tama�o que queramos ^^
			_graphicsEntity->setScale(entityInfo->getVector3Attribute("scale"));
			// con esto puedo girar lo que quiera cuanto quiera. Lo he probado con el mapa
		}
		
		//�queremos material custom?
		/*if(entityInfo->hasAttribute("materialName") && _model == "heavy.mesh"){
			//material type
			std::string materialName = entityInfo->getStringAttribute("materialName");
			_material.push_back(materialName+"HeadBlue");
			_material.push_back(materialName+"EyeLBlue");
			_material.push_back(materialName+"EyeRBlue");
			_material.push_back(materialName+"BodyBlue");
			_material.push_back(materialName+"HandsBlue");

			_graphicsEntity->changeMaterial(_material);
		}else */if (entityInfo->hasAttribute("materialName")){
			_material = entityInfo->getStringAttribute("materialName");
		}

		_lightMask = readLightMask(entityInfo);

		return true;

	} // spawn
	
	//---------------------------------------------------------

	Graphics::CEntity* CGraphics::createGraphicsEntity(const Map::CEntity *entityInfo) {
		bool isStatic = false;
		if(entityInfo->hasAttribute("static"))
			isStatic = entityInfo->getBoolAttribute("static");

		if(isStatic) {
			_graphicsEntity = new Graphics::CStaticEntity(_entity->getName(),_model);
			if(!_scene->addStaticEntity((Graphics::CStaticEntity*)_graphicsEntity))
				return 0;
		}
		else {
			_graphicsEntity = new Graphics::CEntity(_entity->getName(),_model);
			if(!_scene->addEntity(_graphicsEntity))
				return 0;
		}

		// La fase de spawn de todas las entidades se encarga de leer posicion y orientacion
		// por eso al ejecutar esta instruccion estamos colocando a la entidad grafica en
		// su sitio
		_graphicsEntity->setTransform(_entity->getPosition(), _entity->getOrientation());
		
		return _graphicsEntity;

	} // createGraphicsEntity

	//---------------------------------------------------------

	unsigned int CGraphics::readLightMask(const Map::CEntity *entityInfo) {
		std::vector<unsigned int> lightGroups;

		if( entityInfo->hasAttribute("lightMask") ) {
			std::istringstream groupListString( entityInfo->getStringAttribute("lightMask") );

			// Para cada cadena entre comas...
			do {
				std::string groupNumber;
				std::getline(groupListString, groupNumber, ',');	// linea entre delimitadores
				
				std::istringstream str(groupNumber);				// wrappeamos cadena como Input Stream
				do {												// Le quitamos los espacios...
					std::getline(str, groupNumber, ' ');			// linea entre espacios
				} while ( groupNumber.size() == 0 && !str.eof() );

				lightGroups.push_back( atoi(groupNumber.c_str()) );
			} while ( !groupListString.eof() );
		}

		unsigned int lightMask;
		if( !lightGroups.empty() ) {
			lightMask = 0;
			for(int i = 0; i < lightGroups.size(); ++i) {
				lightMask |= (1 << lightGroups[i]);
			}
		}
		else {
			lightMask = 0xFFFFFFFF;
		}

		return lightMask;
	}

	//---------------------------------------------------------
	
	void CGraphics::setTransform(const Vector3 &position, const Ogre::Quaternion &orientation) {
		_graphicsEntity->setTransform(position,orientation);
	}

	//---------------------------------------------------------

	bool CGraphics::accept(const std::shared_ptr<CMessage>& message) {
		Logic::TMessageType msgType = message->getMessageType();

		return msgType == Message::ACTIVATE			|| 
			   msgType == Message::CHANGE_MATERIAL	||
			   msgType == Message::DECAL			;
	} // accept
	
	//---------------------------------------------------------

	void CGraphics::process(const std::shared_ptr<CMessage>& message) {
		switch( message->getMessageType() ) {
			case Message::ACTIVATE: {
				setVisible(std::static_pointer_cast<CMessageActivate>(message)->getActivated());
				break;
			}
			case Message::CHANGE_MATERIAL: {
				std::shared_ptr<CMessageChangeMaterial> chgMatMsg = std::static_pointer_cast<CMessageChangeMaterial>(message);
				changeMaterial( chgMatMsg->getMaterialName() );
				break;
			}
			case Message::DECAL: {
				std::shared_ptr<CMessageDecal> msgDecal = std::static_pointer_cast<CMessageDecal>(message);
				drawDecal(msgDecal->getPosition(), msgDecal->getTexture(), msgDecal->getRandomSize());
				break;
			}
		}

		} // process

	//---------------------------------------------------------

	void CGraphics::setVisible(bool renderGraphicEntity) {
		_graphicsEntity->setVisible(renderGraphicEntity);
	}

	//---------------------------------------------------------

	void CGraphics::changeMaterial(const std::string& materialName) {
		if(materialName != "original")
			_graphicsEntity->changeMaterial(materialName);
		else
			_graphicsEntity->changeMaterial(_material);
	}

	//---------------------------------------------------------

	void CGraphics::changeMaterial(const std::list<std::string>& materialList) {
		_graphicsEntity->changeMaterial(materialList);
	}

	//---------------------------------------------------------

	void CGraphics::onActivate() {
		setVisible(true);
		_graphicsEntity->setTransform(_entity->getPosition(),_entity->getOrientation());

	}//---------------------------------------------------------
	//onActivate


	void CGraphics::onTick(unsigned int msecs){
		_graphicsEntity->setTransform(_entity->getPosition(),_entity->getOrientation());
	}//---------------------------------------------------------
	//onTick

	void CGraphics::changeScale(float newScale){
		_graphicsEntity->setScale(newScale);		
	}//---------------------------------------------------------
	//changeScale

	void CGraphics::onDeactivate() {
		setVisible(false);
	}
	//---------------------------------------------------------

	void CGraphics::onStart() {
		_graphicsEntity->setLightMask(_lightMask);
	}
	//---------------------------------------------------------

	void CGraphics::setPosition(Vector3 vPos){
		_graphicsEntity->setPosition(vPos);
	}
	//---------------------------------------------------------

	void CGraphics::drawDecal(Vector3 vPos, std::string vTexture, bool bRandomSize) {
		OgreDecal::OgreMesh worldMesh;

		/// This method will extract all of the triangles from the mesh to be used later. Only should be called once.
		/// If you scale your mesh at all, pass it in here.

		worldMesh.initialize( this->getOgreMesh()->getMesh(), Vector3(1,1,1));
 
		/// Get the DecalGenerator singleton and initialize it
		OgreDecal::DecalGenerator& generator = OgreDecal::DecalGenerator::getSingleton();
		//generator.initialize( sceneMgr );
		generator.initialize(this->getSceneManager());
 
		/// Set Decal parameters:
		Ogre::Vector3 pos = vPos; /// Send a ray into the mesh
		float width = 0.5f;//1.0f;
		float height = 0.5f;//1.0f;

		if (bRandomSize)
		{
			int iRandom = rand() % 130 + 10;//Random entre 150 y 5

			width += iRandom/100.0f;
			height += iRandom/100.0f;
		}

		/// We have everything ready to go. Now it's time to actually generate the decal:
		if (_primerDecal)
		{
			vListaDecals.push_back(this->getSceneManager()->createManualObject());
			//decalObject[_iContadorDecals] = this->getSceneManager()->createManualObject();
		}

		_iContadorDecals++;
		if (_iContadorDecals > _iMaxDecals)
		{
			_iContadorDecals = 1;//Lo reseteo a 1 porque luego voy a restar uno para su acceso en el �ndice
			_primerDecal = false;
		}

		//OgreDecal::Decal decal = generator.createDecal( &worldMesh, pos, width, height, textureName, true, decalObject[_iContadorDecals] );
		Ogre::ManualObject* manObject =  vListaDecals[_iContadorDecals-1];
		OgreDecal::Decal decal = generator.createDecal( &worldMesh, pos, width, height, vTexture, false, manObject );		
		/// Render the decal object. Always verify the returned object - it will be NULL if no decal could be created.
		if ((decal.object) && (_primerDecal)) {
			this->getSceneManager()->getRootSceneNode()->createChildSceneNode()->attachObject( decal.object );
		}
	} // decals

} // namespace Logic

