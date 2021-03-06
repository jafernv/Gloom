/**
@file Entity.cpp

Contiene la implementaci�n de la clase que representa una entidad leida 
del fichero del mapa.

@author David Llans� Garc�a
@date Agosto, 2010
*/
#include "MapEntity.h"

#include <cassert>

namespace Map {
	
	typedef std::pair<std::string, std::string> TSSPar;

	void CEntity::setAttribute(const std::string &attr, const std::string &value)
	{
		TSSPar elem(attr,value);
		//assert(!&_attributes && "attributes no esta definido");

		if(_attributes.count(attr))
			_attributes.erase(attr);
		_attributes.insert(elem);

	} // setAttribute

	//--------------------------------------------------------
	//--------------------------------------------------------

	void CEntity::setAttribute(CEntity *info)
	{
		TAttrList::const_iterator it = info->_attributes.begin();
		for(;it!=info->_attributes.end();++it){
			//buscamos por si ya existia el atributo y hay que modificarlo
			TAttrList::const_iterator it2 = _attributes.find((*it).first);
			if(it2!=_attributes.end()){//si lo ha encontrado ...
				//lo eliminamos de la tabla (para no dejar rastro)
				_attributes.erase(it2);
				//metemos los valores nuevos en la tabla
				TSSPar elem((*it).first, (*it).second);
				_attributes.insert(elem);
			}else{// si no lo ha encotnrado
				TSSPar elem((*it).first, (*it).second);
				_attributes.insert(elem);
			}
		}

	} // setAttribute

	//--------------------------------------------------------

	bool CEntity::hasAttribute(const std::string &attr) const
	{
		// Casos especiales
		if(!attr.compare("name"))
			return true;
		if(!attr.compare("type"))
			return true;
		if(_attributes.count(attr))
			return true;
		return false;

	} // hasAttribute

	//--------------------------------------------------------

	const std::string &CEntity::getStringAttribute(const std::string &attr) const
	{
		// Casos especiales
		if(!attr.compare("name"))
			return _name;
		if(!attr.compare("type"))
			return _type;
		return (*_attributes.find(attr)).second;

	} // getStringAttribute

	//--------------------------------------------------------

	int CEntity::getIntAttribute(const std::string &attr) const
	{
		return atoi((*_attributes.find(attr)).second.c_str());

	} // getIntAttribute

	//--------------------------------------------------------

	float CEntity::getFloatAttribute(const std::string &attr) const
	{
		return (float)atof((*_attributes.find(attr)).second.c_str());

	} // getFloatAttribute

	//--------------------------------------------------------

	double CEntity::getDoubleAttribute(const std::string &attr) const
	{
		return atof((*_attributes.find(attr)).second.c_str());

	} // getDoubleAttribute

	//--------------------------------------------------------

	bool CEntity::getBoolAttribute(const std::string &attr) const
	{
		if(!(*_attributes.find(attr)).second.compare("true"))
			return true;
		else if(!(*_attributes.find(attr)).second.compare("false"))
			return false;
		throw new std::exception("Leido archivo booleano que no es ni true ni false.");

	} // getBoolAttribute

	//--------------------------------------------------------

	const Vector3 CEntity::getVector3Attribute(const std::string &attr) const
	{
		// Recuperamos la cadena  "x y z"
		std::string position = (*_attributes.find(attr)).second;
		int space1 = position.find(' ');
		float x = (float)atof(position.substr(0,space1).c_str());
		int space2 = position.find(' ',space1+1);
		float y = (float)atof(position.substr(space1+1,space2-(space1+1)).c_str());
		float z = (float)atof(position.substr(space2+1,position.size()-(space2+1)).c_str());

		return Vector3(x,y,z);

	} // getVector3Attribute

	//--------------------------------------------------------

	const Quaternion CEntity::getQuaternionAttribute(const std::string &attr) const {
		std::stringstream quatStream( (*_attributes.find(attr)).second );

		Quaternion quat;
		quatStream >> quat.x;
		quatStream >> quat.y;
		quatStream >> quat.z;
		quatStream >> quat.w;

		return quat;
	}

} // namespace Map
