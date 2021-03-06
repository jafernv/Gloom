#include "MessageTransformSnapshot.h"

#include "Logic/Entity/MessageFactory.h"

namespace Logic {

	IMP_FACTORYMESSAGE(CMessageTransformSnapshot);

	//__________________________________________________________________

	CMessageTransformSnapshot::CMessageTransformSnapshot() : CMessage(Message::TRANSFORM_SNAPSHOT) {
		// Nada que hacer
	}

	//__________________________________________________________________

	CMessageTransformSnapshot::~CMessageTransformSnapshot() {
		// Nada que hacer
	}

	//__________________________________________________________________

	void CMessageTransformSnapshot::setBuffer(const std::vector<Matrix4> buffer) {
		_transformBuffer = buffer;
	}

	//__________________________________________________________________
		
	std::vector<Matrix4> CMessageTransformSnapshot::getBuffer() {
		return _transformBuffer;
	}

	//__________________________________________________________________
		
	Net::CBuffer CMessageTransformSnapshot::serialize() {
		// Tama�o igual = cabecera(int) + tambuffer(int) + num matrices (5 floats * tamBuffer)
		int bufferSize = _transformBuffer.size();
		Net::CBuffer buffer( (2 * sizeof(int)) + (bufferSize * 5 * sizeof(float)) );
		buffer.serialize(std::string("CMessageTransformSnapshot"), true);
		buffer.serialize(bufferSize);
		for(int i = 0; i < bufferSize; ++i) {
			buffer.serialize(_transformBuffer[i]);
		}
		
		return buffer;
	}

	//__________________________________________________________________

	void CMessageTransformSnapshot::deserialize(Net::CBuffer& buffer) {
		int bufferSize;
		// Deserializar el tama�o del buffer
		buffer.deserialize(bufferSize);

		// Resize del buffer al tama�o leido
		_transformBuffer.clear();
		_transformBuffer.resize(bufferSize);
		// Leer tantas matrices como nos diga el tama�o
		for(int i = 0; i < bufferSize; ++i) {
			buffer.deserialize(_transformBuffer[i]);
		}
	}

};
