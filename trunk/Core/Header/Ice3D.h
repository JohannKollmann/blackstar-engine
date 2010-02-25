
#pragma once

#include "Ogre.h"

namespace Ice
{

	class Point3D
	{
	public:
		virtual void SetGlobalPosition(Ogre::Vector3 position) = 0;
		virtual Ogre::Vector3 GetGlobalPosition() = 0;
	};

	class SimplePoint3D
	{
	private:
		Ogre::Vector3 mPosition;
	public:
		void SetGlobalPosition(Ogre::Vector3 position) { mPosition = position; }
		Ogre::Vector3 GetGlobalPosition() { return mPosition; }
	};
	
	class Transformable3D : public Point3D
	{
		virtual void SetGlobalOrientation(Ogre::Quaternion q) = 0;
		virtual Ogre::Quaternion GetGlobalOrientation() = 0;
		virtual void SetGlobalScale(Ogre::Vector3 position) = 0;
		virtual Ogre::Vector3 GetGlobalScale() = 0;
	};

}