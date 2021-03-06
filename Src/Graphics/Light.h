//---------------------------------------------------------------------------
// Light.h
//---------------------------------------------------------------------------

/**
@file Light.h

Contiene la declaraci�n de la clase que representa una Luz.

@see Graphics::Clight

@author Rub�n Mulero
@author Francisco Aisa Garc�a
@date March, 2013
*/

#ifndef __Graphics_Light_H
#define __Graphics_Light_H

#include "BaseSubsystems/Math.h"

// Predeclaraci�n de clases para ahorrar tiempo de compilaci�n
namespace Ogre {
	class SceneNode;
	class Light;
}

namespace Graphics {
	class CScene;
}

namespace Graphics {

	struct LightType {
		enum Enum {
			eDIRECTIONAL_LIGHT,
			ePOINT_LIGHT,
			eSPOT_LIGHT
		};
	};

	/**
	Clase que representa las luces en el escenario de ogre, contiene una referencia
	que representa la luz de ogre y otra que es el nodo de la escena al que est� 
	asociada esa luz
	Al construir la clase no se crean las luces, la construcci�n de un objeto de
	esta clase representa la luz, pero hay que usar los m�todos que nos proporciona
	para crear y manejar la clase a nuestro antojo
	*/
	class CLight{
	public:
		CLight(const LightType::Enum lightType, const Vector3& position = Vector3::ZERO, const Vector3& direction = Vector3::NEGATIVE_UNIT_Y);
		~CLight();

		/**
		M�todo que setea la posici�n de la luz
		@param position la posici�n en la que queremos la luz
		*/
		void setPosition(const Vector3& position);
		/**
		M�todo que setea la direcci�n de la luz
		@param direction la direcci�n en la que queremos que mire la luz
		*/
		void setDirection(const Vector3& direction);
		/**
		M�todo que setea la rotaci�n de la luz
		@param Quaternion que expresa la direcci�n en la que queremos
		girar la luz.
		*/
		void setOrientation(const Quaternion& rotation);
		/**
		M�todo controla las sombras que castea la luz.
		@param enabled indica si las sombras se activan o se desactivan
		*/
		void setCastShadows(bool enabled);
		/**
		M�todo que setea el color de la luz
		@param r el rojo
		@param g el verde
		@param b el azul
		*/
		void setColor(float r, float g, float b);
		/**
		M�todo para setear la atenuaci�n de la luz
		@param Range rango de alcance de la luz.
		@param Kc Constante de atenuaci�n constante.
		@param Kl Constante de atenuaci�n lineal.
		@param Kq Constante de atenuaci�n cuadr�tica.
		*/
		void setAttenuation(float Range, float Kc, float Kl, float Kq);
		/**
		M�todo para configurar los par�metros de los spotlights.
		@param innerAngle �ngulo del cono interior (en radianes).
		@param outerAngle �ngulo del cono exterior (en radianes).
		*/
		void setSpotLightParams(float innerAngle, float outerAngle);
		
		/**
		M�todo que devuelve la direcci�n de la luz
		@return la direcci�n en la que mira la luz
		*/
		Vector3 getDirection();
		/**
		M�todo que devuelve la posici�n de la luz
		@return la posici�n de la luz
		*/
		Vector3 getPosition();
		/**
		Asigna un grupo a la luz. Si el grupo de esta luz no esta incluido en
		la mascara de luces asignada a las entidades graficas, esta luz no
		afectara a dicha entidad.

		@param group Grupo de la luz.
		*/
		void setGroup(unsigned int group);
		/**
		Configura la luz como estatica o dinamica. Dependiendo de si la luz
		ha sido clasificada como estatica o dinamica, el shader de bump
		se calculara de una forma distinta para aprovechar los lightmaps.

		@param isStatic true si queremos que la luz sea estatica.
		*/
		void setStatic(bool isStatic);
		/**
		Devuelve el nombre de la luz.

		@return El nombre de la luz.
		*/
		std::string getName();

	protected:

		Ogre::Light* _light;
	};

}

#endif