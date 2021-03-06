
#include "IceGOCOgreNode.h"
#include "IceMain.h"
#include "IceGameObject.h"
#include "Ogre.h"

namespace Ice
{
	GOCOgreNode::GOCOgreNode(void)
	{
		mNode = Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode();
	}
	GOCOgreNode::~GOCOgreNode(void)
	{
		 Main::Instance().GetOgreSceneMgr()->destroySceneNode(mNode);
	}

	void GOCOgreNode::UpdatePosition(const Ogre::Vector3 &position)
	{
		mNode->setPosition(position);
	}
	void GOCOgreNode::UpdateOrientation(const Ogre::Quaternion &orientation)
	{
		mNode->setOrientation(orientation);
	}
	void GOCOgreNode::UpdateScale(const Ogre::Vector3 &scale)
	{
		mNode->setScale(scale);
	}

	GOCOgreNodeUser::~GOCOgreNodeUser()
	{
		GameObjectPtr owner = mOwnerGO.lock();
		if (owner.get())
		{
			if (GetNode()->numAttachedObjects() == 0)
				owner->RemoveComponent("OgreNode");
		}
	}
	Ogre::SceneNode* GOCOgreNodeUser::GetNode()
	{
		GameObjectPtr owner = mOwnerGO.lock();
		IceAssert(owner.get())

		GOCOgreNode *gocNode = owner->CreateOrRetrieveComponent<GOCOgreNode>();
		return gocNode->GetNode();
	}
}
