/**
@file Math.h

Este fichero contiene la definici�n de ciertos tipos de datos
utilizados por la aplicaci�n y relacionados con la base
matem�tica; en particular, define distintos tipos de matriz,
vector, etc.
<p>
En la pr�ctica, los tipos son simplemente tipos sin�nimos
a los de Ogre, para evitar p�rdidas de tiempo en conversiones
superfluas entre la aplicaci�n y Ogre (s� habr� que hacerlas
entre la aplicaci�n y el motor de f�sica, por ejemplo).
<p>
Se ofrecen tambi�n una serie de funciones auxiliares.
 
@author David Llans�
@date Julio, 2010
*/

#ifndef __BaseSubsystems_Math_H
#define __BaseSubsystems_Math_H

// Includes de Ogre donde se definen los tipos
#include "Euler.h"
#include <OgreVector2.h>
#include <OgreVector3.h>
#include <OgreVector4.h>
#include <OgreQuaternion.h>
#include <OgreMatrix3.h>
#include <OgreMatrix4.h>
#include <OgreRay.h>
#include <OgreLight.h>
#include <string>

/**
Clase de ayuda para pasar de grados de euler a quaterniones
y viceversa.
*/
typedef Ogre::Euler Euler;

/**
Definicion de matriz de 4x4. La definici�n del tipo de datos
es la misma que la utilizada por el motor gr�fico, por lo tanto
es dependiente del motor usado.
 */
typedef Ogre::Matrix4 Matrix4;

/**
Definici�n de matriz de 3x3 de rotaci�n. La definici�n del tipo 
de datos es la misma que la utilizada por el motor gr�fico, por 
lo tanto es dependiente del motor usado.
 */
typedef Ogre::Matrix3 Matrix3;

/**
Vector (o punto) 2d utilizado. La definici�n del tipo de datos
es la misma que la utilizada por el motor gr�fico, por lo tanto
es dependiente del motor usado.
 */
typedef Ogre::Vector2 Vector2;

/**
Vector (o punto) 3d utilizado. La definici�n del tipo de datos
es la misma que la utilizada por el motor gr�fico, por lo tanto
es dependiente del motor usado.
 */
typedef	Ogre::Vector3 Vector3;

/**
Vector (o punto) 4d utilizado. La definici�n del tipo de datos
es la misma que la utilizada por el motor gr�fico, por lo tanto
es dependiente del motor usado.
 */
typedef	Ogre::Vector4 Vector4;

/**
Quaternion, usado para c�lculos de rotaciones tridimensionales. 
La definici�n del tipo de datos es la misma que la utilizada por 
el motor gr�fico, por lo tanto es dependiente del motor usado.
 */
typedef Ogre::Quaternion Quaternion;

/**
Rayo. La definici�n del tipo de datos es la misma que la utilizada 
por el motor gr�fico, por lo tanto es dependiente del motor usado.
 */
typedef Ogre::Ray Ray;

/**
Namespace en el que ofrecemos alguna definici�n de constante
matam�tica y m�todos para convertir grados en radianes, etc.
*/
namespace Math
{
	/**
	Definici�n de la constante PI.
	*/
	static const float PI = float( 4.0f * atan( 1.0f ) );

	/**
	Definici�n de la constante PI.
	*/
	static const float HALF_PI = PI * 0.5f;
	/**
	Constante para pasar de grados a radianes.
	*/
	static const float _deg2Rad = PI / 180.0f;

	/**
	Constante para pasar de radianes a grados.
	*/
	static const float _rad2Deg = 180.0f / PI;

	/**
	Transforma grados en radianes.

	@param degree �ngulo en grados.
	@return �ngulo en radianes.
	*/
	static float fromDegreesToRadians(float degrees) {return degrees*_deg2Rad;}

	/**
	Transforma radianes en grados.

	@param radian �ngulo en radianes.
	@return �ngulo en grados.
	*/
	static float fromRadiansToDegrees(float radians) {return radians*_rad2Deg;}
	

	/**
	Transforma orientacion en grados en orientacion en quaternion.
	Nota: Las funciones trigonometricas tienen peque�isimas desviaciones en decimales por ejemplo al hacer el cos de 90 grados.
	Si esa infima desviaci�n influyera, deberia de tratarse con casos especiales todos aquellos valores que sean naturales.

	@param degrees �ngulo en grados.
	@param axe Vector que indica sobre que eje rotamos.
	@return �ngulo en quaternion.
	*/
	static Quaternion fromDegreesToQuaternion(float degrees,const Vector3 &axe){
		//Hacemos una conversion inicial de grados a radianes ya que las funciones trigonometricas trabajan con radianes
		float rad= degrees*Math::PI/180;
		return Quaternion(cos(rad/2.0f), sin(rad/2.0f) * axe.x, sin(rad/2.0f)* axe.y, sin(rad/2.0f) * axe.z);
	}

	/**
	Transforma orientacion en radianes en orientacion en quaternion.
	Nota: Las funciones trigonometricas tienen peque�isimas desviaciones en decimales por ejemplo al hacer el cos de 90 grados.
	Si esa infima desviaci�n influyera, deberia de tratarse con casos especiales todos aquellos valores que sean naturales.

	@param rad �ngulo en radianes.
	@param axe Vector que indica sobre que eje rotamos.
	@return �ngulo en quaternion.
	*/
	static Quaternion fromRadsToQuaternion(float rad,const Vector3 &axe){
		return Quaternion(cos(rad/2.0f), sin(rad/2.0f) * axe.x, sin(rad/2.0f)* axe.y, sin(rad/2.0f) * axe.z);
	}


	/**
	Dado el angulo en radianes del yaw, pitch y roll crea el quaternion equivalente.
	Nota: Las funciones trigonometricas tienen peque�isimas desviaciones en decimales por ejemplo al hacer el cos de 90 grados.
	Si esa infima desviaci�n influyera, deberia de tratarse con casos especiales todos aquellos valores que sean naturales.

	@param yaw Yaw en radianes.
	@param pitch Pitch en radianes.
	@param roll Roll en radianes.
	@return Quaternion resultante.
	*/
	static Quaternion setQuaternion(float yaw, float pitch, float roll )
	{
	  
		Quaternion qy( cos(yaw/2.0f), 0, sin(yaw/2.0f),0);
      Quaternion qp( cos(pitch/2.0f), sin(pitch/2.0f), 0, 0);
      Quaternion qr( cos(roll/2.0f), 0, 0, sin(roll/2.0f) );
	  Quaternion q;
	  q=qy*qp*qr;
      return q;
	}


	/**
	Devuelve en grados de Euler los grados de yaw,pitch y roll por los que esta formado el quaternion.

	@param q Quaternion a extraer los grados.
	@return Vector3 con el yaw,pitch,roll.
	*/
	static Vector3 getEulerYawPitchRoll(const Quaternion& quaternion)
	{
		Ogre::Radian mPitch,mRoll,mYaw;
		quaternion.getPitch();
		mYaw = Ogre::Math::ATan2(2 * quaternion.y * quaternion.w - 2 * quaternion.x * quaternion.z, 1 - 2 * Ogre::Math::Pow(quaternion.y, 2) - 2 * Ogre::Math::Pow(quaternion.z, 2));
		mRoll = Ogre::Math::ASin(2 * quaternion.x * quaternion.y + 2 * quaternion.z * quaternion.w);
		mPitch = Ogre::Math::ATan2(2 * quaternion.x * quaternion.w - 2 * quaternion.y * quaternion.z, 1 - 2 * Ogre::Math::Pow(quaternion.x, 2) - 2 * Ogre::Math::Pow(quaternion.z, 2));
 
		//Solo a�adir si se dejara que pudieras llegar a 90/-90� de pitch
		//En cuyo caso las funciones de getYawQuaternion y getPitchQuaternion deber�an a�adirlo tambien
		/*
		if(quaternion.x * quaternion.y + quaternion.z * quaternion.w == 0.5)
		{
			mYaw = 2.0f * Ogre::Math::ATan2(quaternion.x, quaternion.w);
			mPitch = 0.0f;
		}
		else if(quaternion.x * quaternion.y + quaternion.z * quaternion.w == -0.5)
		{ 
			mYaw = -2.0f * Ogre::Math::ATan2(quaternion.x, quaternion.w);
			mPitch = 0.0f;
		}
		*/
		return Vector3(mYaw.valueRadians(),mPitch.valueRadians(),mRoll.valueRadians());
	}


	/**
	Devuelve el quaternion yaw extraido del quaternion pasado como par�metro.

	@param q Quaternion a extraer el quaternion yaw.
	@return Quaternion yaw.
	*/
	static Quaternion getYawQuaternion(const Quaternion& quaternion)
	{
		Ogre::Radian mYaw = quaternion.getYaw();
		return Quaternion(cos(mYaw.valueRadians()/2.0f), 0, sin(mYaw.valueRadians()/2.0f),0);
	}


	/**
	Devuelve el quaternion pitch extraido del quaternion pasado como par�metro.

	@param q Quaternion a extraer el quaternion pitch.
	@return Quaternion pitch.
	*/
	static Quaternion getPitchQuaternion(const Quaternion& quaternion)
	{
		Ogre::Radian mPitch = quaternion.getPitch();
		return Quaternion(cos(mPitch.valueRadians()/2.0f), sin(mPitch.valueRadians()/2.0f), 0, 0);
	}


	/**
	Devuelve el quaternion roll extraido del quaternion pasado como par�metro.

	@param q Quaternion a extraer el quaternion roll.
	@return Quaternion roll.
	*/
	static Quaternion getRollQuaternion(const Quaternion& quaternion)
	{
		Ogre::Radian mRoll = quaternion.getRoll();
		return Quaternion(cos(mRoll.valueRadians()/2.0f), 0, sin(mRoll.valueRadians()/2.0f),0);
	}


	/**
	Rota el quaternion pasado por par�metro con los radianes especificados tambien por par�metro sobre el eje especificado.
	Por ejemplo si queremos rotar el yaw, pasariamos el axe(0,1,0) con rotation(radianes) y el quaternion a rotar.

	@param axe Eje sobre el que rotaremos.
	@param rotation Radianes que rotaremos.
	@param quaternion Quaternion que ser� rotado.
	*/
	static void rotate(const Vector3 &axis, Ogre::Radian angle, Quaternion &quaternion ){
		Quaternion q;
		q.FromAngleAxis(angle,axis);
		Quaternion qnorm = q;
		qnorm.normalise();
		quaternion = qnorm * quaternion;
	}


	/**
	Crea un vector unitario de direcci�n a partir de un angulo de
	orientaci�n en radianes.

	@param orientation Orientaci�n en radianes.
	@return Vector unitario en el plano ZX.
	*/
	static Vector3 getDirection(float orientation) 
	{
		return Vector3(-sin(orientation), 0, -cos(orientation));

	} // getDirection

	/**
	Crea un vector unitario de direcci�n vertical a partir de un angulo de
	orientaci�n en radianes.

	@param orientation Orientaci�n en radianes.
	@return Vector unitario en el plano ZX.
	*/
	static Vector3 getDirectionPitch(float orientation) 
	{
		
		return Vector3(0, orientation, 0);

	} // getDirection
	
	/**
	Aplica un viraje a una matriz de transformaci�n.

	@param turn Giro en radianes que se quiere aplicar.
	@param transform Matriz de transformaci�n a modificar.
	*/
	static void yaw(float turn, Matrix4& transform) 
	{
		Matrix3 rotation;
		transform.extract3x3Matrix(rotation);
		Ogre::Radian yaw, pitch, roll;
		rotation.ToEulerAnglesYXZ(yaw, pitch, roll);
		Ogre::Radian newYaw = yaw + Ogre::Radian(turn);
		rotation.FromEulerAnglesYXZ(newYaw, pitch, roll);
		transform = rotation;

	} // yaw
	
	/**
	Extrae el estado del viraje de una matriz de transformaci�n.

	@param transform Matriz de transformaci�n.
	@return Viraje de la entidad.
	*/
	static float getYaw(const Matrix4& transform) 
	{
		Matrix3 rotation;
		transform.extract3x3Matrix(rotation);
		Ogre::Radian yaw, pitch, roll;
		rotation.ToEulerAnglesYXZ(yaw, pitch, roll);
		return yaw.valueRadians();

	} // getYaw

	static float getYaw(const Quaternion& quaternion) 
	{
		return quaternion.getYaw().valueRadians();

	} // getYaw

	static float getPitch(const Quaternion& quaternion) 
	{
		return quaternion.getPitch().valueRadians();

	} // getYaw


	static float getRoll(const Quaternion& quaternion) 
	{
		return quaternion.getRoll().valueRadians();

	} // getYaw
	
	/**
	Establece un viraje a una matriz de transformaci�n.

	@param turn Giro en radianes que se quiere etablecer.
	@param transform Matriz de transformaci�n a modificar.
	*/
	static void setYaw(float turn, Matrix4& transform) 
	{
		// Reiniciamos la matriz de rotaci�n
		transform = Matrix3::IDENTITY;
		// Sobre esta rotamos.
		Math::yaw(turn,transform);

	} // setYaw

	/**
	Aplica un subviraje a una matriz de transformaci�n.

	@param turn Giro en radianes que se quiere aplicar.
	@param transform Matriz de transformaci�n a modificar.
	*/
	static void pitch(float turn, Matrix4& transform) 
	{

		Matrix3 rotation;
		transform.extract3x3Matrix(rotation);
		Ogre::Radian yaw, pitch, roll;
		
		rotation.ToEulerAnglesYXZ(yaw, pitch, roll);
		Ogre::Radian newPitch = pitch + Ogre::Radian(turn);
		rotation.FromEulerAnglesYXZ(yaw, newPitch, roll);

		/*
<<<<<<< HEAD
		rotation.ToEulerAnglesYXZ(yaw, pitch, roll);
		Ogre::Radian newPitch = pitch + Ogre::Radian(turn);
		rotation.FromEulerAnglesYXZ(yaw, newPitch, roll);
=======
		rotation.ToEulerAnglesYXZ(yaw, pitch, roll);

		Ogre::Radian newPitch = pitch + Ogre::Radian(turn);

		rotation.FromEulerAnglesYXZ(yaw, newPitch, roll);
>>>>>>> movimiento
		*/

		transform = rotation;
	} // pitch
	
	/**
	Extrae el estado del subviraje de una matriz de transformaci�n.

	@param transform Matriz de transformaci�n.
	@return Viraje de la entidad.
	*/
	static float getPitch(const Matrix4& transform) 
	{
		Matrix3 rotation;
		transform.extract3x3Matrix(rotation);
		Ogre::Radian yaw, pitch, roll;
		rotation.ToEulerAnglesYXZ(yaw, pitch, roll);
		return pitch.valueRadians();

	} // getPitch
	
	/**
	Establece un subviraje a una matriz de transformaci�n.

	@param turn Giro en radianes que se quiere etablecer.
	@param transform Matriz de transformaci�n a modificar.
	*/
	static void setPitch(float turn, Matrix4& transform) 
	{
		// Reiniciamos la matriz de rotaci�n
		transform = Matrix3::IDENTITY;
		// Sobre esta rotamos.
		Math::pitch(turn,transform);

	} // setPitch

	/**
	Establece un subviraje a una matriz de transformaci�n.

	@param turn Giro en radianes que se quiere etablecer.
	@param transform Matriz de transformaci�n a modificar.
	*/
	static void setPitch(float turn, Matrix3& rotation) 
	{
		rotation = Matrix3::IDENTITY;

		Ogre::Radian yaw, pitch, roll;
		
		rotation.ToEulerAnglesYXZ(yaw, pitch, roll);
		rotation.FromEulerAnglesYXZ(yaw, Ogre::Radian(turn), roll);

	} // setPitch

	/**
	Aplica un viraje a una matriz de transformaci�n.

	@param turn Giro en radianes que se quiere aplicar.
	@param transform Matriz de transformaci�n a modificar.
	*/
	static void roll(float turn, Matrix4& transform) 
	{
		Matrix3 rotation;
		transform.extract3x3Matrix(rotation);
		Ogre::Radian yaw, pitch, roll;
		rotation.ToEulerAnglesYXZ(yaw, pitch, roll);
		Ogre::Radian newRoll = roll + Ogre::Radian(turn);
		rotation.FromEulerAnglesYXZ(yaw, pitch, newRoll);
		transform = rotation;

	} // roll
	
	/**
	Extrae el estado del viraje de una matriz de transformaci�n.

	@param transform Matriz de transformaci�n.
	@return Viraje de la entidad.
	*/
	static float getRoll(const Matrix4& transform) 
	{
		Matrix3 rotation;
		transform.extract3x3Matrix(rotation);
		Ogre::Radian yaw, pitch, roll;
		rotation.ToEulerAnglesYXZ(yaw, pitch, roll);
		return roll.valueRadians();

	} // getRoll
	
	/**
	Establece un viraje a una matriz de transformaci�n.

	@param turn Giro en radianes que se quiere etablecer.
	@param transform Matriz de transformaci�n a modificar.
	*/
	static void setRoll(float turn, Matrix4& transform)
	{
		// Reiniciamos la matriz de rotaci�n
		transform = Matrix3::IDENTITY;
		// Sobre esta rotamos.
		Math::roll(turn,transform);

	} // setRoll

	/**
	Aplica un viraje a una matriz de transformaci�n.

	@param turnP Giro en radianes que se quiere aplicar al pitch.
	@param turnY Giro en radianes que se quiere aplicar al yaw.
	@param transform Matriz de transformaci�n a modificar.
	*/
	static void pitchYaw(float turnP, float turnY, Matrix4& transform) 
	{

		Matrix3 rotation;
		transform.extract3x3Matrix(rotation);
		Ogre::Radian yaw, pitch, roll;
		rotation.ToEulerAnglesYXZ(yaw, pitch, roll);
		Ogre::Radian newPitch = pitch + Ogre::Radian(turnP);
		Ogre::Radian newYaw = yaw + Ogre::Radian(turnY);

		rotation.FromEulerAnglesYXZ(newYaw, newPitch, roll);
		transform = rotation;

	} // pitchYaw


	/**
	Establece un subviraje a una matriz de transformaci�n.

	@param turnP Giro en radianes que se quiere etablecer en el eje Y.
	@param turnY Giro en radianes que se quiere etablecer en el eje X.
	@param transform Matriz de transformaci�n a modificar.
	*/
	static void setPitchYaw(float turnP, float turnY, Matrix4& transform) 
	{
		// Reiniciamos la matriz de rotaci�n
		transform = Matrix3::IDENTITY;
		// Sobre esta rotamos.
		Math::pitchYaw(turnP,turnY,transform);
	} // setPitchYaw

	/**
	Aplica un viraje a una matriz de transformaci�n.

	@param turnP Giro en radianes que se quiere aplicar al pitch.
	@param turnY Giro en radianes que se quiere aplicar al yaw.
	@param turnR Giro en radianes que se quiere aplicar al roll.
	@param transform Matriz de transformaci�n a modificar.
	*/
	static void pitchYawRoll(float turnP, float turnY,float turnR, Matrix4& transform) 
	{

		Matrix3 rotation;
		transform.extract3x3Matrix(rotation);
		Ogre::Radian yaw, pitch, roll;
		rotation.ToEulerAnglesYXZ(yaw, pitch, roll);
		Ogre::Radian newPitch = pitch + Ogre::Radian(turnP);
		Ogre::Radian newYaw = yaw + Ogre::Radian(turnY);
		Ogre::Radian newRoll = roll + Ogre::Radian(turnR);

		rotation.FromEulerAnglesYXZ(newYaw, newPitch, newRoll);
		transform = rotation;

	} // pitch


	static void yaw(float turn, Quaternion &quaternion)
	{
		Vector3 axis;

		// Rotate around local Y axis
		axis = quaternion * Vector3::UNIT_Y;
		rotate(axis, Ogre::Radian(turn), quaternion);
	}

	static void pitch(float turn, Quaternion &quaternion)
	{
		Vector3 axis;

		// Rotate around local Y axis
		axis = quaternion * Vector3::UNIT_X;
		rotate(axis, Ogre::Radian(turn), quaternion);
	}

	static void roll(float turn, Quaternion &quaternion)
	{
		Vector3 axis;

		// Rotate around local Y axis
		axis = quaternion * Vector3::UNIT_Z;
		rotate(axis, Ogre::Radian(turn), quaternion);
	}

	/**
	Establece un subviraje a una matriz de transformaci�n.

	@param turnP Giro en radianes que se quiere etablecer en el eje Y.
	@param turnY Giro en radianes que se quiere etablecer en el eje X.
	@param turnR Giro en radianes que se quiere etablecer en el eje Z.
	@param transform Matriz de transformaci�n a modificar.
	*/
	static void setPitchYawRoll(float turnP, float turnY, float turnR, Matrix4& transform) 
	{
		// Reiniciamos la matriz de rotaci�n
		transform = Matrix3::IDENTITY;
		// Sobre esta rotamos.
		Math::pitchYawRoll(turnP,turnY,turnR,transform);

	} // setPitchYawRoll
	
	/**
	Crea un vector unitario de direcci�n en el plano ZX a partir 
	de una matriz de transformaci�n.

	@param transform Matriz de transformaci�n.
	@return Vector unitario en el plano ZX.
	*/
	static Vector3 getDirection(const Matrix4& transform) 
	{
		Vector3 yaw = getDirection(getYaw(transform));
		Vector3 pitch = getDirectionPitch(getPitch(transform));
		Vector3 direction = yaw + pitch;
		direction.normalise();
		return direction;

	} // getDirection

	/**
	Calcula el valor CRC de un string pasado como parametro.

	@param nombre string del que se desea calcular el CRC.
	@return el n�mero entero resultado de calcular el CRC.
	*/
	static int CRC(const std::string nombre) 
	{
		
		int count = nombre.size();
		unsigned int  crc;
		unsigned int i;
		const char* ptr = nombre.c_str();
		crc = 0;

		while (--count >= 0)
		{
			crc = crc ^ (unsigned short)*ptr++ << 8;
			i = 8;
			do
			{
				if (crc & 0x8000)
					crc = crc << 1 ^ 0x1021;
				else
					crc = crc << 1;
			} while(--i);
		}
		return (crc);

	} // CRC

	// Funciones para obtener n�meros randomizados

	/**
	Resetea el generador de n�meros aleatorios con el reloj del sistema.
	Llamar al comienzo del programa para inicializar el generador.
	*/
	static void seed() {
		srand( time(0) );
	}

	/**
	Generar uniformemente un n�mero aleatorio en
	el intervalo R[0,1].

	@return Un n�mero aleatorio uniformemente distribuido
	en el intervalo R[0,1].
	*/
	static double unifRand() {
		return rand() / double(RAND_MAX);
	}

	/**
	Generar un n�mero aleatorio en el intervalo
	de los reales dado.

	@param a Cota inferior.
	@param b Cota superior.
	@return Un n�mero aleatorio uniformemente
	distribuido en el intervalo R[a,b].
	*/
	static double unifRand(double a, double b) {
		return (b - a) * unifRand() + a;
	}

	/**
	Generar un entero aleatorio entre 1 y un valor dado.

	@param n Cota superior.
	@return Un n�mero aleatorio uniformemente distribuido
	en el intervalo Z[1, n].
	*/
	static long unifRand(long n) {
		if (n < 0) n = -n;
		if (n == 0) return 0;

		long guard = (long) (unifRand() * n) + 1;
		return (guard > n) ? n : guard;
	}


} // namespace Math

#endif // __BaseSubsystems_Math_H