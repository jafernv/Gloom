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
		
		_currentAmmo = 1;

		_damageReflect = entityInfo->getIntAttribute("weaponHammerDamageReflect");
		
		return true;
	}

	void CShootHammer::decrementAmmo() {
		// Redefinimos el metodo para que no se haga nada ya que el hammer
		// realmente no tiene municion
	}// decrementAmmo
	//---------------------------------------------------------

	void CShootHammer::triggerHitMessages(std::pair<CEntity*, Ray> entityHit) {
		if(entityHit.first->getName().compare("World")==0){
			Vector3 direccionOpuestaRay= entityHit.second.getDirection()*-1;
			Logic::CMessageRebound *m=new Logic::CMessageRebound();
			m->setDir(direccionOpuestaRay);
			_entity->emitMessage(m);

			Logic::CMessageDamaged *damage=new Logic::CMessageDamaged();
			damage->setDamage(_damageReflect);
			damage->setEnemy(_entity);
			_entity->emitMessage(damage);

		}
		// LLamar al componente que corresponda con el da�o hecho (solamente si no fuera el mundo el del choque)
		else{
			Logic::CMessageDamaged *m=new Logic::CMessageDamaged();
			m->setDamage(_damage);
			m->setEnemy(entityHit.first);
			entityHit.first->emitMessage(m);
		}
	}// triggerHitMessages
	//---------------------------------------------------------

} // namespace Logic

