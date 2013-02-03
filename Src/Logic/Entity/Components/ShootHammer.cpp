/**
@file Shoot.cpp

Contiene la implementaci�n del componente que gestiona las armas y que administra el disparo.
 
@see Logic::CShoot
@see Logic::IComponent

*/

#include "ShootHammer.h"

#include "Logic/Entity/Entity.h"
#include "Map/MapEntity.h"
#include "Physics/Server.h"
#include "Graphics/Server.h"
#include "Graphics/Scene.h"
#include "Logic/Entity/Components/ArrayGraphics.h"
#include "Logic/Entity/Components/Life.h"
#include "Logic/Entity/Components/Shoot.h"

#include "Logic/Messages/MessageControl.h"
#include "Logic/Messages/MessageDamaged.h"
#include "Logic/Messages/MessageRebound.h"

#include "Graphics/Camera.h"



#include <OgreSceneManager.h>
#include <OgreMaterialManager.h>
#include <OgreManualObject.h>

namespace Logic 
{
	IMP_FACTORY(CShootHammer);
	
	bool CShootHammer::spawn(CEntity* entity, CMap *map, const Map::CEntity *entityInfo){

		if(!CShoot::spawn(entity,map,entityInfo))
			return false;
		
		_damageReflect = entityInfo->getIntAttribute("weaponHammerDamageReflect");
		
		return true;
	}


	void CShootHammer::shoot(){
		
		std::cout << "DISPARO HAMMER" << std::endl;
		//Generaci�n del rayo habiendo obtenido antes el origen y la direcci�n
		Graphics::CCamera* camera = Graphics::CServer::getSingletonPtr()->getActiveScene()->getCamera();
		
		
		// Para generalizar las armas, todas tendras tantas balas como la variable numberShoots
		for(int i = 0; i < _numberShoots; ++i)
		{

			// Se corrige la posicion de la camara
			Vector3 direction = camera->getTargetCameraPosition() - camera->getCameraPosition();
			direction.normalise();
			//El origen debe ser m�nimo la capsula y por si miramos al suelo la separaci�n m�nima debe ser 1.5f ( en un futuro es probable que sea recomendable cambiar por no chocar con el grupo de uno mismo )
			Vector3 origin = camera->getCameraPosition() + ((_capsuleRadius + 1.5f) * direction);


			//Me dispongo a calcular la desviacion del arma, en el map.txt se pondra en grados de dispersion (0 => sin dispersion)
			Ogre::Radian angle = Ogre::Radian( (  (((float)(rand() % 100))/100.0f) * (_dispersion)) /100);

			

			//Esto hace un random total, lo que significa, por ejemplo, que puede que todas las balas vayan hacia la derecha 
			Vector3 dispersionDirection = direction.randomDeviant(angle);
			dispersionDirection.normalise();

			// Creamos el ray desde el origen en la direccion del raton (desvido ya aplicado)
			Ray ray(origin, dispersionDirection);
			


			////////////////////////////////////////////////Dibujo del rayo


				Graphics::CScene *scene = Graphics::CServer::getSingletonPtr()->getActiveScene();
				Ogre::SceneManager *mSceneMgr = scene->getSceneMgr();

				
				std::stringstream aux;
				aux << "laser" << _nameWeapon << _temporal;
				++_temporal;
				std::string laser = aux.str();


				Ogre::ManualObject* myManualObject =  mSceneMgr->createManualObject(laser); 
				Ogre::SceneNode* myManualObjectNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(laser+"_node"); 
 

				myManualObject->begin("laser", Ogre::RenderOperation::OT_LINE_STRIP);
				Vector3 v = ray.getOrigin();
				myManualObject->position(v.x,v.y,v.z);


			for(int i=0; i < _distance;++i){
				Vector3 v = ray.getPoint(i);
				myManualObject->position(v.x,v.y,v.z);
				// etc 
			}

				myManualObject->end(); 
				myManualObjectNode->attachObject(myManualObject);


			//////////////////////////////////fin del dibujado del rayo

			//Rayo lanzado por el servidor de f�sicas de acuerdo a la distancia de potencia del arma
			CEntity *entityCollision = Physics::CServer::getSingletonPtr()->raycastClosest(ray, _distance);
		
			//Si hay colisi�n env�amos a dicha entidad un mensaje de da�o
			if(entityCollision)
			{
				if(entityCollision->getName().compare("World")==0){

					Vector3 direccionOpuestaRay= ray.getDirection()*-1;
					Logic::CMessageRebound *m=new Logic::CMessageRebound();
					m->setDir(direccionOpuestaRay);
					_entity->emitMessage(m);

					Logic::CMessageDamaged *damage=new Logic::CMessageDamaged();
					damage->setDamage(_damageReflect);
					_entity->emitMessage(damage);

				}
				// LLamar al componente que corresponda con el da�o hecho (solamente si no fuera el mundo el del choque)
				//entity->
				else{
					Logic::CMessageDamaged *m=new Logic::CMessageDamaged();
					m->setDamage(_damage);
					entityCollision->emitMessage(m);
				}
			}
			
			//Para el rebote, si hay colision con la entidad mundo pues reboto en la direcci�n opuesta a la que miro
			
				
		
		}// fin del bucle para multiples disparos
	} // shoot

	
	//---------------------------------------------------------

} // namespace Logic
