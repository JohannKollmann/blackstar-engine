
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


	GOCOgreNodeUser::~GOCOgreNodeUser()
	{
		if (GetNode()->numAttachedObjects() == 0)
			mOwnerGO->RemoveComponent("OgreNode");
	}
	Ogre::SceneNode* GOCOgreNodeUser::GetNode()
	{
		if (!mOwnerGO)
		{
			OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR, "Error in GOCOgreNodeUser::GetNode: Component has no OwnerGO yet!", "IceGOCOgreNode.cpp");
			return nullptr;
		}
		GOCOgreNode *gocNode = mOwnerGO->GetComponent<GOCOgreNode>();
		if (gocNode) return gocNode->GetNode();

		gocNode = new GOCOgreNode();
		mOwnerGO->AddComponent(gocNode);
		return gocNode->GetNode();
	}
}
