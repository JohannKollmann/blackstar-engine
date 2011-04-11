
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
		MessageSystem::Instance().JoinNewsgroup(this, "KEY_DOWN");
		//MessageSystem::Instance().JoinNewsgroup(this, "KEY_UP");
		MessageSystem::Instance().JoinNewsgroup(this, "MOUSE_MOVE");
		MessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
	}

	mActive = active;
	GameObjectPtr owner = mOwnerGO.lock();
	if (mActive && owner.get())
		SceneManager::Instance().RegisterPlayer(owner);
	else SceneManager::Instance().RegisterPlayer(GameObjectPtr());
}

void GOCPlayerInput::Pause(bool pause)
{
	mActive = !pause;
	if (pause)
	{
		mCharacterMovementState = 0;
		BroadcastMovementState();
	}
}

void GOCPlayerInput::ReceiveMessage(Msg &msg)
{
	if (!mActive) return;

	if (msg.type == "MOUSE_MOVE")
	{
		mOwnerGO.lock()->Rotate(Ogre::Vector3(0,1,0), Ogre::Radian((Ogre::Degree(-msg.params.GetInt("ROT_X_REL") * 0.2f))));
	}
	if (msg.type == "UPDATE_PER_FRAME")
	{
		int newState = 0;
		if (Main::Instance().GetInputManager()->isKeyDown(OIS::KC_W))
			newState |= CharacterMovement::FORWARD;

		if (Main::Instance().GetInputManager()->isKeyDown(OIS::KC_S))
			newState |= CharacterMovement::BACKWARD;

		if (Main::Instance().GetInputManager()->isKeyDown(OIS::KC_A))
			newState |= CharacterMovement::LEFT;

		if (Main::Instance().GetInputManager()->isKeyDown(OIS::KC_D))
			newState |= CharacterMovement::RIGHT;

		BroadcastMovementState(newState);
	}
	if (msg.type == "KEY_DOWN")
	{
		if (msg.params.GetInt("KEY_ID_OIS") == OIS::KC_SPACE)
		{
			Msg msg;
			msg.type = "INPUT_START_JUMP";
			mOwnerGO.lock()->SendInstantMessage(msg);
		}
	}
}

#include "NxController.h"

void GOCPlayerInput::ReceiveObjectMessage(Msg &msg)
{
	if (msg.type == "CharacterCollisionReport")
	{
		NxU32 collisionFlags = msg.params.GetInt("collisionFlags");
	}
}

};