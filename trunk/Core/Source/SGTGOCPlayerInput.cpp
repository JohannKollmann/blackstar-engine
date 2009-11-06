
#include "..\Header\SGTGOCPlayerInput.h"
#include "OIS/OIS.h"
#include "SGTMessageSystem.h"
#include "SGTGameObject.h"
#include "SGTMain.h"
#include "SGTCameraController.h"

SGTGOCPlayerInput::SGTGOCPlayerInput(void)
{
	SGTMessageSystem::Instance().JoinNewsgroup(this, "KEY_DOWN");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "KEY_UP");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "MOUSE_MOVE");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");

	mCharacterMovementState = 0;
}

SGTGOCPlayerInput::~SGTGOCPlayerInput(void)
{
}

void SGTGOCPlayerInput::ReceiveMessage(SGTMsg &msg)
{
	if (msg.mNewsgroup == "KEY_DOWN")
	{
		OIS::KeyCode kc = (OIS::KeyCode)msg.mData.GetInt("KEY_ID_OIS");
		if (kc == OIS::KC_W)
		{
			mCharacterMovementState = mCharacterMovementState | SGTCharacterMovement::FORWARD;
		}
		if (kc == OIS::KC_S)
		{
			mCharacterMovementState = mCharacterMovementState | SGTCharacterMovement::BACKWARD;
		}
		if (kc == OIS::KC_A)
		{
			mCharacterMovementState = mCharacterMovementState | SGTCharacterMovement::LEFT;
		}
		if (kc == OIS::KC_D)
		{
			mCharacterMovementState = mCharacterMovementState | SGTCharacterMovement::RIGHT;
		}
		if (kc == OIS::KC_SPACE)
		{
			mCharacterMovementState = mCharacterMovementState | SGTCharacterMovement::JUMP;
		}
	}
	if (msg.mNewsgroup == "KEY_UP")
	{
		OIS::KeyCode kc = (OIS::KeyCode)msg.mData.GetInt("KEY_ID_OIS");
		if (kc == OIS::KC_W)
		{
			mCharacterMovementState = mCharacterMovementState & ~SGTCharacterMovement::FORWARD;
		}
		if (kc == OIS::KC_S)
		{
			mCharacterMovementState = mCharacterMovementState & ~SGTCharacterMovement::BACKWARD;
		}
		if (kc == OIS::KC_A)
		{
			mCharacterMovementState = mCharacterMovementState & ~SGTCharacterMovement::LEFT;
		}
		if (kc == OIS::KC_D)
		{
			mCharacterMovementState = mCharacterMovementState & ~SGTCharacterMovement::RIGHT;
		}
	}
	BroadcastMovementState();

	if (msg.mNewsgroup == "MOUSE_MOVE")
	{
		mOwnerGO->Rotate(Ogre::Vector3(0,1,0), Ogre::Radian((Ogre::Degree(-msg.mData.GetInt("ROT_X_REL") * 0.2f))));
	}
	if (msg.mNewsgroup == "UPDATE_PER_FRAME")
	{
	}
}

#include "NxController.h"

void SGTGOCPlayerInput::ReceiveObjectMessage(Ogre::SharedPtr<SGTObjectMsg> msg)
{
	if (msg->mName == "CharacterJumpEnded")
	{
		mCharacterMovementState = mCharacterMovementState & ~SGTCharacterMovement::JUMP;
		BroadcastMovementState();
	}
	if (msg->mName == "CharacterCollisionReport")
	{
		NxU32 collisionFlags = msg->mData.GetFloat("collisionFlags");
	}
}


//CameraController

SGTGOCCameraController::SGTGOCCameraController(Ogre::Camera *camera)
{
	mCamera = camera;
	mCharacterCenterNode = SGTMain::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode();
	mCameraCenterNode = mCharacterCenterNode->createChildSceneNode();
	mTargetNode = mCameraCenterNode->createChildSceneNode(Ogre::Vector3(0,2.5f,10));
	mCameraNode = mCameraCenterNode->createChildSceneNode(Ogre::Vector3(0,2.0f,-6));
	mCameraNode->setAutoTracking(true, mTargetNode);
	mCameraNode->attachObject(mCamera);
	mCameraNode->setFixedYawAxis (true);

	mTightness = 0.04f;
	mfCameraAngle = 0;
	mfLastCharacterAngle = 0;
	mfCharacterAngle = 0;

	SGTMain::Instance().GetCameraController()->mMove = false;
	SGTMain::Instance().GetCameraController()->mXRot = false;
	SGTMain::Instance().GetCameraController()->mYRot = false;

	SGTMessageSystem::Instance().JoinNewsgroup(this, "MOUSE_MOVE");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
}

SGTGOCCameraController::~SGTGOCCameraController()
{
	SGTMain::Instance().GetOgreSceneMgr()->destroySceneNode(mCharacterCenterNode);
	SGTMain::Instance().GetOgreSceneMgr()->destroySceneNode(mCameraCenterNode);
	SGTMain::Instance().GetOgreSceneMgr()->destroySceneNode(mCameraNode);
	SGTMain::Instance().GetOgreSceneMgr()->destroySceneNode(mTargetNode);
}

void SGTGOCCameraController::ReceiveMessage(SGTMsg &msg)
{
	if (msg.mNewsgroup == "MOUSE_MOVE")
	{
		Ogre::Radian pitch = Ogre::Radian((Ogre::Degree(-msg.mData.GetInt("ROT_Y_REL"))));
		Ogre::Radian newpitch =(mCameraCenterNode->getOrientation().getPitch() - pitch);
		if (mCameraCenterNode->getOrientation().getPitch().valueDegrees() > -30.0f && pitch.valueDegrees() < 0) mCameraCenterNode->rotate(Ogre::Vector3(1,0,0), Ogre::Radian(pitch * mTightness));
		else if (mCameraCenterNode->getOrientation().getPitch().valueDegrees() < 40.0f && pitch.valueDegrees() > 0) mCameraCenterNode->rotate(Ogre::Vector3(1,0,0), Ogre::Radian(pitch * mTightness));
	}
	if (msg.mNewsgroup == "UPDATE_PER_FRAME")
	{
		float time = msg.mData.GetFloat("TIME");
		float fCharacterAngle=mCharacterOrientation.getYaw().valueRadians();
		float fAngleDelta=mfLastCharacterAngle-fCharacterAngle;
		if(fAngleDelta>Ogre::Math::PI)
			fAngleDelta-=2*Ogre::Math::PI;
		if(fAngleDelta<-Ogre::Math::PI)
			fAngleDelta+=2*Ogre::Math::PI;
		mfCharacterAngle+=fAngleDelta;
		mCameraCenterNode->rotate(Ogre::Vector3(0,1,0), Ogre::Radian((mfCameraAngle-mfCharacterAngle)*1.0f*time));
		mfCameraAngle-=(mfCameraAngle-mfCharacterAngle)*1.0f*time;
		mfLastCharacterAngle=fCharacterAngle;
	}

}

void SGTGOCCameraController::ReceiveObjectMessage(Ogre::SharedPtr<SGTObjectMsg> msg)
{
}

void SGTGOCCameraController::UpdatePosition(Ogre::Vector3 position)
{
	mCharacterCenterNode->setPosition(position);
	//mCamera->lookAt(mCenterNode->getPosition());
}

void SGTGOCCameraController::UpdateOrientation(Ogre::Quaternion orientation)
{
	mCharacterOrientation = orientation;
	//mCharacterCenterNode->setOrientation(orientation);
}
