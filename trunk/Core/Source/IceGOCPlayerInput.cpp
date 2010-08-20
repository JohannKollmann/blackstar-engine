
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
	mCharacterMovementState = 0;
	mActive = false;
}

GOCPlayerInput::~GOCPlayerInput(void)
{
}

void GOCPlayerInput::SetActive(bool active)
{
	if (active)
	{
		MessageSystem::Instance().QuitAllNewsgroups(this);
		//MessageSystem::Instance().JoinNewsgroup(this, "KEY_DOWN");
		//MessageSystem::Instance().JoinNewsgroup(this, "KEY_UP");
		MessageSystem::Instance().JoinNewsgroup(this, "MOUSE_MOVE");
		MessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
	}

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
	if (msg.type == "KEY_DOWN")
	{
		OIS::KeyCode kc = (OIS::KeyCode)msg.params.GetInt("KEY_ID_OIS");
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
	if (msg.type == "KEY_UP")
	{
		OIS::KeyCode kc = (OIS::KeyCode)msg.params.GetInt("KEY_ID_OIS");
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

	if (msg.type == "MOUSE_MOVE")
	{
		mOwnerGO->Rotate(Ogre::Vector3(0,1,0), Ogre::Radian((Ogre::Degree(-msg.params.GetInt("ROT_X_REL") * 0.25f))));
	}
	if (msg.type == "UPDATE_PER_FRAME")
	{
		mCharacterMovementState=0;
		if (Main::Instance().GetInputManager()->isKeyDown(OIS::KC_W))
			mCharacterMovementState |= CharacterMovement::FORWARD;

		if (Main::Instance().GetInputManager()->isKeyDown(OIS::KC_S))
			mCharacterMovementState |= CharacterMovement::BACKWARD;

		if (Main::Instance().GetInputManager()->isKeyDown(OIS::KC_A))
			mCharacterMovementState |= CharacterMovement::LEFT;

		if (Main::Instance().GetInputManager()->isKeyDown(OIS::KC_D))
			mCharacterMovementState |= CharacterMovement::RIGHT;

		if (Main::Instance().GetInputManager()->isKeyDown(OIS::KC_SPACE))
			mCharacterMovementState |= CharacterMovement::JUMP;
	}
}

#include "NxController.h"

void GOCPlayerInput::ReceiveObjectMessage(Msg &msg)
{
	if (msg.type == "CharacterJumpEnded")
	{
		mCharacterMovementState = mCharacterMovementState & ~CharacterMovement::JUMP;
		BroadcastMovementState();
	}
	if (msg.type == "CharacterCollisionReport")
	{
		NxU32 collisionFlags = msg.params.GetInt("collisionFlags");
	}
}

};