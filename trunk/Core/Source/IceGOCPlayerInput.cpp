
#include "IceGOCPlayerInput.h"
#include "OIS/OIS.h"
#include "IceMessageSystem.h"
#include "IceGameObject.h"
#include "IceMain.h"
#include "IceCameraController.h"

namespace Ice
{

GOCPlayerInput::GOCPlayerInput(void)
{
	MessageSystem::Instance().JoinNewsgroup(this, "KEY_DOWN");
	MessageSystem::Instance().JoinNewsgroup(this, "KEY_UP");
	MessageSystem::Instance().JoinNewsgroup(this, "MOUSE_MOVE");
	MessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");

	mCharacterMovementState = 0;
	mActive = false;
}

GOCPlayerInput::~GOCPlayerInput(void)
{
	MessageSystem::Instance().QuitNewsgroup(this, "KEY_DOWN");
	MessageSystem::Instance().QuitNewsgroup(this, "KEY_UP");
	MessageSystem::Instance().QuitNewsgroup(this, "MOUSE_MOVE");
	MessageSystem::Instance().QuitNewsgroup(this, "UPDATE_PER_FRAME");
}

void GOCPlayerInput::AttachToGO(GameObject *go)
{
	go->RemoveComponent(GetFamilyID());
	go->AddComponent(this);
}

void GOCPlayerInput::SetActive(bool active)
{
	mActive = active;
}

void GOCPlayerInput::ReceiveMessage(Msg &msg)
{
	if (!mActive) return;
	if (msg.mNewsgroup == "KEY_DOWN")
	{
		OIS::KeyCode kc = (OIS::KeyCode)msg.mData.GetInt("KEY_ID_OIS");
		if (kc == OIS::KC_W)
		{
			mCharacterMovementState = mCharacterMovementState | CharacterMovement::FORWARD;
		}
		if (kc == OIS::KC_S)
		{
			mCharacterMovementState = mCharacterMovementState | CharacterMovement::BACKWARD;
		}
		if (kc == OIS::KC_A)
		{
			mCharacterMovementState = mCharacterMovementState | CharacterMovement::LEFT;
		}
		if (kc == OIS::KC_D)
		{
			mCharacterMovementState = mCharacterMovementState | CharacterMovement::RIGHT;
		}
		if (kc == OIS::KC_SPACE)
		{
			mCharacterMovementState = mCharacterMovementState | CharacterMovement::JUMP;
		}
	}
	if (msg.mNewsgroup == "KEY_UP")
	{
		OIS::KeyCode kc = (OIS::KeyCode)msg.mData.GetInt("KEY_ID_OIS");
		if (kc == OIS::KC_W)
		{
			mCharacterMovementState = mCharacterMovementState & ~CharacterMovement::FORWARD;
		}
		if (kc == OIS::KC_S)
		{
			mCharacterMovementState = mCharacterMovementState & ~CharacterMovement::BACKWARD;
		}
		if (kc == OIS::KC_A)
		{
			mCharacterMovementState = mCharacterMovementState & ~CharacterMovement::LEFT;
		}
		if (kc == OIS::KC_D)
		{
			mCharacterMovementState = mCharacterMovementState & ~CharacterMovement::RIGHT;
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

void GOCPlayerInput::ReceiveObjectMessage(Ogre::SharedPtr<ObjectMsg> msg)
{
	if (msg->mName == "CharacterJumpEnded")
	{
		mCharacterMovementState = mCharacterMovementState & ~CharacterMovement::JUMP;
		BroadcastMovementState();
	}
	if (msg->mName == "CharacterCollisionReport")
	{
		NxU32 collisionFlags = msg->mData.GetFloat("collisionFlags");
	}
}


//CameraController

GOCCameraController::GOCCameraController()
{
	CreateNodes();
	mCamera = 0;
}
GOCCameraController::GOCCameraController(Ogre::Camera *camera)
{
	CreateNodes();
	AttachCamera(camera);
}

GOCCameraController::~GOCCameraController()
{
	if (mCamera) DetachCamera();
	Main::Instance().GetOgreSceneMgr()->destroySceneNode(mCharacterCenterNode);
	Main::Instance().GetOgreSceneMgr()->destroySceneNode(mCameraCenterNode);
	Main::Instance().GetOgreSceneMgr()->destroySceneNode(mCameraNode);
	Main::Instance().GetOgreSceneMgr()->destroySceneNode(mTargetNode);
}

void GOCCameraController::CreateNodes()
{
	/*mCharacterCenterNode = Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode();
	mCameraCenterNode = mCharacterCenterNode->createChildSceneNode();
	mTargetNode = mCameraCenterNode->createChildSceneNode(Ogre::Vector3(0,2.5f,10));
	mCameraNode = mCameraCenterNode->createChildSceneNode(Ogre::Vector3(0,2.0f,-6));
	mCameraNode->setAutoTracking(true, mTargetNode);
	mCameraNode->setFixedYawAxis (true);

	mTightness = 0.04f;
	mfCameraAngle = 0;
	mfLastCharacterAngle = 0;
	mfCharacterAngle = 0;*/

	//Simple first person setup
	mCharacterCenterNode = Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode();
	mCameraCenterNode = mCharacterCenterNode->createChildSceneNode(Ogre::Vector3(0, 2, 0));
	mCameraNode = mCameraCenterNode->createChildSceneNode();
	mTargetNode = mCameraCenterNode->createChildSceneNode(Ogre::Vector3(0,2.5f,10));		//unused

	mTightness = 0.04f;
}

void GOCCameraController::AttachCamera(Ogre::Camera *camera)
{
	mCameraCenterNode->resetOrientation();
	mCamera = camera;
	mCameraNode->attachObject(mCamera);
	mCamera->setPosition(0,0,0);
	mCamera->setOrientation(Ogre::Vector3::UNIT_Z.getRotationTo(Ogre::Vector3(0,0,-1)));

	MessageSystem::Instance().JoinNewsgroup(this, "MOUSE_MOVE");
	MessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
}
void GOCCameraController::DetachCamera()
{
	mCameraNode->detachObject(mCamera);
	MessageSystem::Instance().QuitNewsgroup(this, "MOUSE_MOVE");
	MessageSystem::Instance().QuitNewsgroup(this, "UPDATE_PER_FRAME");
	mCamera = 0;
}

void GOCCameraController::ReceiveMessage(Msg &msg)
{
	if (msg.mNewsgroup == "MOUSE_MOVE")
	{
		Ogre::Radian pitch = Ogre::Radian((Ogre::Degree(msg.mData.GetInt("ROT_Y_REL"))));
		Ogre::Radian newpitch =(mCameraCenterNode->getOrientation().getPitch() - pitch);
		if (mCameraCenterNode->getOrientation().getPitch().valueDegrees() > -30.0f && pitch.valueDegrees() < 0) mCameraCenterNode->rotate(Ogre::Vector3(1,0,0), Ogre::Radian(pitch * mTightness));
		else if (mCameraCenterNode->getOrientation().getPitch().valueDegrees() < 40.0f && pitch.valueDegrees() > 0) mCameraCenterNode->rotate(Ogre::Vector3(1,0,0), Ogre::Radian(pitch * mTightness));
	}
	if (msg.mNewsgroup == "UPDATE_PER_FRAME")
	{
		/*float time = msg.mData.GetFloat("TIME");
		float fCharacterAngle=mCharacterOrientation.getYaw().valueRadians();
		float fAngleDelta=mfLastCharacterAngle-fCharacterAngle;
		if(fAngleDelta>Ogre::Math::PI)
			fAngleDelta-=2*Ogre::Math::PI;
		if(fAngleDelta<-Ogre::Math::PI)
			fAngleDelta+=2*Ogre::Math::PI;
		mfCharacterAngle+=fAngleDelta;
		mCameraCenterNode->rotate(Ogre::Vector3(0,1,0), Ogre::Radian((mfCameraAngle-mfCharacterAngle)*1.0f*time));
		mfCameraAngle-=(mfCameraAngle-mfCharacterAngle)*1.0f*time;
		mfLastCharacterAngle=fCharacterAngle;*/
	}

}

void GOCCameraController::AttachToGO(GameObject *go)
{
	go->RemoveComponent(GetFamilyID());
	go->AddComponent(this);
}

void GOCCameraController::ReceiveObjectMessage(Ogre::SharedPtr<ObjectMsg> msg)
{
}

void GOCCameraController::UpdatePosition(Ogre::Vector3 position)
{
	mCharacterCenterNode->setPosition(position);
}

void GOCCameraController::UpdateOrientation(Ogre::Quaternion orientation)
{
	//mCharacterOrientation = orientation;
	mCharacterCenterNode->setOrientation(orientation);
}

};