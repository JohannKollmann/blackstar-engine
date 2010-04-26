
#include "OgrePhysXNodeRenderable.h"

namespace OgrePhysX
{

	NodeRenderable::NodeRenderable(Ogre::SceneNode* node)
	{
		mNode = node;
	}
	NodeRenderable::~NodeRenderable(void)
	{
	}

	void NodeRenderable::setTransform(Ogre::Vector3 position, Ogre::Quaternion rotation)
	{
		mNode->setPosition(position);
		mNode->setOrientation(rotation);
	}
}
