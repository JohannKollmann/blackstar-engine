
#include "IceGOCPlayerInput.h"
#include "OIS/OIS.h"
#include "IceMessageSystem.h"
#include "IceGameObject.h"
#include "IceMain.h"
#include "IceCameraController.h"
#include "IceSceneManager.h"

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
	if (mActive && mOwnerGO)
		SceneManager::Instance().RegisterPlayer(mOwnerGO);
	else SceneManager::Instance().RegisterPlayer(0);
}

void GOCPlayerInput::Pause(bool pause)
{
	mActive = !pause;
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
	//mCharacterMovementState = mCharacterMovementState & ~CharacterMovement::JUMP;	//Jumping is always a one-time event

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

};