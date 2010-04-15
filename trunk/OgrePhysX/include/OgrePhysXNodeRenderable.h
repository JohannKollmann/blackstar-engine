#pragma once

#include "OgrePhysXClasses.h"
#include "OgrePhysXPointRenderable.h"
#include "Ogre.h"

namespace OgrePhysX
{
	class OgrePhysXClass NodeRenderable : public PointRenderable
	{
	private:
		Ogre::SceneNode *mNode;

	public:
		NodeRenderable(Ogre::SceneNode* node);
		~NodeRenderable(void);

		void setTransform(Ogre::Vector3 position, Ogre::Quaternion rotation);
	};

}