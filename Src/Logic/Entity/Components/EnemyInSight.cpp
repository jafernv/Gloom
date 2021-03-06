/**
@file EnemyInSight.cpp


@see Logic::CEnemyInSight
@see Logic::IComponent

@author Francisco Aisa Garc�a
@date Agosto, 2013
*/

#include "EnemyInSight.h"
#include "CharacterName.h"

#include "Application/BaseApplication.h"
#include "Map/MapEntity.h"
#include "Logic/Maps/Map.h"
#include "Logic/Server.h"
#include "Logic/GameNetPlayersManager.h"

#include "Physics/Server.h"
#include "Physics/RaycastHit.h"

#include <vector>

using namespace std;

namespace Logic {

	IMP_FACTORY(CEnemyInSight);

	//________________________________________________________________________

	CEnemyInSight::CEnemyInSight() : _visibilityTimeStep(3000),
									 _raycastTimeStep(500),
									 _raycastTimer(_raycastTimeStep) {

		_physicsServer = Physics::CServer::getSingletonPtr();
	}

	//________________________________________________________________________

	CEnemyInSight::~CEnemyInSight() {
		// Nada que hacer
	}

	//________________________________________________________________________

	bool CEnemyInSight::spawn(CEntity* entity, CMap* map, const Map::CEntity* entityInfo) {
		if( !IComponent::spawn(entity,map,entityInfo) ) return false;

		assert( entityInfo->hasAttribute("heightShoot") );
		_heightShoot = entityInfo->getFloatAttribute("heightShoot");

		return true;
	} // spawn

	//________________________________________________________________________

	void CEnemyInSight::onTick(unsigned int msecs) {
		if( !_namesBeingShown.empty() ) {
			vector< map<CEntity*, int>::iterator > entitiesToBeDeleted;
			entitiesToBeDeleted.reserve( _namesBeingShown.size() );

			auto it = _namesBeingShown.begin();
			for(; it != _namesBeingShown.end(); ++it) {
				it->second -= msecs;
				if(it->second < 0) {
					entitiesToBeDeleted.push_back(it);
				}
			}

			CEntity* entity;
			CCharacterName* charName;
			CMap* map = CServer::getSingletonPtr()->getMap();
			for(int i = 0; i < entitiesToBeDeleted.size(); ++i) {
				entity = map->getEntityByID( entitiesToBeDeleted[i]->first->getEntityID() );
				if(entity != NULL) { // Nos aseguramos de que la entidad no haya sido borrada
					charName = entitiesToBeDeleted[i]->first->getComponent<CCharacterName>("CCharacterName");
					if(charName != NULL) {
						charName->setVisible(false);
						_namesBeingShown.erase(entitiesToBeDeleted[i]);
					}
				}
			}
		}
		
		_raycastTimer -= msecs;
		if(_raycastTimer < 0) {
			_raycastTimer = _raycastTimeStep;
			
			Ray straightLine(_entity->getPosition() + Vector3(0.0f, _heightShoot, 0.0f), _entity->getOrientation() * Vector3::NEGATIVE_UNIT_Z);
			std::vector<Physics::CRaycastHit> hits;
			_physicsServer->raycastMultiple(straightLine, 600.0f, hits, true, Physics::CollisionGroup::ePLAYER);
			
			if( !hits.empty() ) {
				CEntity* enemyEntity = NULL;
				for(int i = 0; i < hits.size(); ++i) {
					if(hits[i].entity != _entity) {
						enemyEntity = hits[i].entity;
						break;
					}
				}

				if(enemyEntity != NULL) {
					CGameNetPlayersManager* playersMgr = CGameNetPlayersManager::getSingletonPtr();
					TeamFaction::Enum myTeam = playersMgr->getTeamUsingEntityId( _entity->getEntityID() );
					TeamFaction::Enum otherEntityTeam = playersMgr->getTeamUsingEntityId( enemyEntity->getEntityID() );

					if(myTeam == TeamFaction::eNONE || otherEntityTeam == TeamFaction::eNONE || myTeam != otherEntityTeam) {
						CCharacterName* charName = enemyEntity->getComponent<CCharacterName>("CCharacterName");
						if(charName != NULL) {
							_namesBeingShown[enemyEntity] = _visibilityTimeStep;
							charName->setVisible(true);
						}
					}
				}
			}
		}
	}

} // namespace Logic
