
#pragma once

#include "Ogre.h"

namespace Ice
{

	class Point3D
	{
	public:
		virtual void SetGlobalPosition(const Ogre::Vector3 &position) = 0;
		virtual Ogre::Vector3 GetGlobalPosition() = 0;

		virtual ~Point3D() {}
	};

	class SimplePoint3D : public Point3D
	{
	protected:
		Ogre::Vector3 mPosition;
	public:
		SimplePoint3D() {}
		SimplePoint3D(Ogre::Vector3 position) : mPosition(position) {}
		virtual ~SimplePoint3D() {}
		void SetGlobalPosition(const Ogre::Vector3 &position) { mPosition = position; }
		Ogre::Vector3 GetGlobalPosition() { return mPosition; }
	};
	
	class Transformable3D : public Point3D
	{
	public:
		virtual ~Transformable3D() {}
		virtual void SetGlobalOrientation(const Ogre::Quaternion &q) = 0;
		virtual Ogre::Quaternion GetGlobalOrientation() = 0;
		virtual void SetGlobalScale(const Ogre::Vector3 &scale) = 0;
		virtual Ogre::Vector3 GetGlobalScale() = 0;
	};

}