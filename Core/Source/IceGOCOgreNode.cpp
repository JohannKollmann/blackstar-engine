
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

	void GOCOgreNode::UpdatePosition(Ogre::Vector3 position)
	{
		mNode->setPosition(position);
	}
	void GOCOgreNode::UpdateOrientation(Ogre::Quaternion orientation)
	{
		mNode->setOrientation(orientation);
	}
	void GOCOgreNode::UpdateScale(Ogre::Vector3 scale)
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

		GOCOgreNode *gocNode = owner->GetComponent<GOCOgreNode>();
		if (gocNode) return gocNode->GetNode();

		gocNode = new GOCOgreNode();
		owner->AddComponent(GOComponentPtr(gocNode));
		return gocNode->GetNode();
	}
}
