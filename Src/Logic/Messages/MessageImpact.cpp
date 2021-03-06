#include "MessageImpact.h"

#include "Logic/Entity/MessageFactory.h"
#include "Logic/Entity/Entity.h"
#include "Logic/Maps/Map.h"
#include "Logic/Server.h"

#include <string>

namespace Logic {

	IMP_FACTORYMESSAGE(CMessageImpact);

	CMessageImpact::CMessageImpact() : CMessage(Message::IMPACT) {
		// Nada que hacer
	}//
	//----------------------------------------------------------

	CMessageImpact::~CMessageImpact() {
		// Nada que hacer
	}
	//----------------------------------------------------------

	void CMessageImpact::setDirectionImpact(float fAngle) {
		this->_fAngleImpactDirection = fAngle;
	}//
	//----------------------------------------------------------
	
	Net::CBuffer CMessageImpact::serialize() {
		Logic::TEntityID id = _entity->getEntityID();

		Net::CBuffer buffer( sizeof(int) + sizeof(id) );
		buffer.serialize(std::string("CMessageSetRelatedEntity"), true);
		buffer.serialize(id);
		
		return buffer;
	}//
	//----------------------------------------------------------

	void CMessageImpact::deserialize(Net::CBuffer& buffer) {
		TEntityID id;
		// Por problemas con enumerados utilizamos directamente
		// el read en vez del deserialize
		buffer.read( &id, sizeof(id) );

		_entity = Logic::CServer::getSingletonPtr()->getMap()->getEntityByID(id);
	}

};
