
#include "..\Header\SGTGOCPlayerInput.h"
#include "OIS/OIS.h"
#include "SGTMessageSystem.h"
#include "SGTGameObject.h"
#include "SGTMain.h"
#include "SGTCameraController.h"

SGTGOCPlayerInput::SGTGOCPlayerInput(void)
{
	mDirection = Ogre::Vector3(0,0,0);
	SGTMessageSystem::Instance().JoinNewsgroup(this, "KEY_DOWN");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "KEY_UP");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "MOUSE_MOVE");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
}

SGTGOCPlayerInput::~SGTGOCPlayerInput(void)
{
}

void SGTGOCPlayerInput::ReceiveMessage(SGTMsg &msg)
{
	if (msg.mNewsgroup == "KEY_DOWN")
	{
		OIS::KeyCode kc = (OIS::KeyCode)msg.mData.GetInt("KEY_ID_OIS");
		bool move = false;
		if (kc == OIS::KC_W)
		{
			mDirection = Ogre::Vector3(0,0,-1);
			move = true;
		}
		if (kc == OIS::KC_S)
		{
			mDirection = Ogre::Vector3(0,0,1);
			move = true;
		}
		if (kc == OIS::KC_A)
		{
			mDirection = Ogre::Vector3(-1,0,0);
			move = true;
		}
		if (kc == OIS::KC_D)
		{
			mDirection = Ogre::Vector3(1,0,0);
			move = true;
		}
		if (move)
		{
			SGTObjectMsg *objmsg = new SGTObjectMsg();
			objmsg->mName = "ChangeCharacterDirection";
			objmsg->mData.AddOgreVec3("Direction", mDirection);
			mOwnerGO->SendInstantMessage("GOCPhysics", Ogre::SharedPtr<SGTObjectMsg>(objmsg));
		}
		return;
	}
	if (msg.mNewsgroup == "KEY_UP")
	{
		OIS::KeyCode kc = (OIS::KeyCode)msg.mData.GetInt("KEY_ID_OIS");
		bool move = false;
		if (kc == OIS::KC_W)
		{
			mDirection = Ogre::Vector3(0,0,0);
			move = true;
		}
		if (kc == OIS::KC_S)
		{
			mDirection = Ogre::Vector3(0,0,0);
			move = true;
		}
		if (kc == OIS::KC_A)
		{
			mDirection = Ogre::Vector3(0,0,0);
			move = true;
		}
		if (kc == OIS::KC_D)
		{
			mDirection = Ogre::Vector3(0,0,0);
			move = true;
		}
		if (move)
		{
			SGTObjectMsg *objmsg = new SGTObjectMsg();
			objmsg->mName = "ChangeCharacterDirection";
			objmsg->mData.AddOgreVec3("Direction", mDirection);
			mOwnerGO->SendInstantMessage("GOCPhysics", Ogre::SharedPtr<SGTObjectMsg>(objmsg));
		}
		return;
	}
	if (msg.mNewsgroup == "MOUSE_MOVE")
	{
		mOwnerGO->Rotate(Ogre::Vector3(0,1,0), Ogre::Radian((Ogre::Degree(-msg.mData.GetInt("ROT_X_REL") * 0.2f))));
		//mOwnerGO->Rotate(Ogre::Vector3(1,0,0), Ogre::Radian((Ogre::Degree(-msg.mData.GetInt("ROT_Y_REL") * 0.2f))));
	}
	/*if (msg.mNewsgroup == "UPDATE_PER_FRAME")
	{
		if (mDirection != Ogre::Vector3(0,0,0))
		{
			SGTObjectMsg *objmsg = new SGTObjectMsg();
			objmsg->mName = "ChangeCharacterDirection";
			objmsg->mData.AddOgreVec3("Direction", mDirection * msg.mData.GetFloat("TIME"));
			mOwnerGO->SendInstantMessage("GOCPhysics", Ogre::SharedPtr<SGTObjectMsg>(objmsg));
		}
	}*/
}


//CameraController

SGTGOCCameraController::SGTGOCCameraController(Ogre::Camera *camera)
{
	mCamera = camera;
	mCenterNode = SGTMain::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode();
	mCenterNode->attachObject(camera);
	mTargetOffset = Ogre::Vector3(0, 2, -0.5);
	mPositionOffset = Ogre::Vector3(0, 1, 5);
	mCamera->setPosition(mPositionOffset);

	SGTMain::Instance().GetCameraController()->mMove = false;
	SGTMain::Instance().GetCameraController()->mXRot = false;
	SGTMain::Instance().GetCameraController()->mYRot = false;
}

SGTGOCCameraController::~SGTGOCCameraController()
{
	SGTMain::Instance().GetOgreSceneMgr()->destroySceneNode(mCenterNode);
}

void SGTGOCCameraController::ReceiveMessage(SGTMsg &msg)
{
}

void SGTGOCCameraController::ReceiveObjectMessage(Ogre::SharedPtr<SGTObjectMsg> msg)
{
	if (msg->mName == "Update_Position" || msg->mName == "Update_Transform")
	{
		Ogre::Vector3 pos = msg->mData.GetOgreVec3("Position");
		mCenterNode->setPosition(pos);
		mCamera->lookAt(mCenterNode->getPosition());
	}
	if (msg->mName == "Update_Orientation" || msg->mName == "Update_Transform")
	{
		mCenterNode->setOrientation(msg->mData.GetOgreQuat("Orientation"));
	}
}

void SGTGOCCameraController::UpdatePosition(Ogre::Vector3 position)
{
	mCenterNode->setPosition(position);
	mCamera->lookAt(mCenterNode->getPosition());
}

void SGTGOCCameraController::UpdateOrientation(Ogre::Quaternion orientation)
{
	mCenterNode->setOrientation(orientation);
}
