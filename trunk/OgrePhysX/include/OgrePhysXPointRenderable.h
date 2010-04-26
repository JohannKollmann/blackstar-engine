#pragma once

#include "OgrePhysXClasses.h"
#include "Ogre.h"

namespace OgrePhysX
{
	class OgrePhysXClass PointRenderable
	{
	public:
		virtual ~PointRenderable() {}

		virtual void setTransform(Ogre::Vector3 position, Ogre::Quaternion rotation) = 0;
	};
}