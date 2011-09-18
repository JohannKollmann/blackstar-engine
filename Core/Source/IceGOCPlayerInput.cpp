
#include "IceGOCPlayerInput.h"
#include "OIS/OIS.h"
#include "IceMessageSystem.h"
#include "IceGameObject.h"
#include "IceObjectMessageIDs.h"

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
		QuitAllNewsgroups();
		JoinNewsgroup(GlobalMessageIDs::KEY_DOWN);
		//JoinNewsgroup(GlobalMessageIDs::KEY_UP);
		JoinNewsgroup(GlobalMessageIDs::MOUSE_MOVE);
		JoinNewsgroup(GlobalMessageIDs::UPDATE_PER_FRAME);
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

	if (msg.typeID == GlobalMessageIDs::MOUSE_MOVE)
	{
		mOwnerGO.lock()->Rotate(Ogre::Vector3(0,1,0), Ogre::Radian((Ogre::Degree(-msg.params.GetInt("ROT_X_REL") * 0.2f))));
	}
	if (msg.typeID == GlobalMessageIDs::UPDATE_PER_FRAME)
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
	if (msg.typeID == GlobalMessageIDs::KEY_DOWN)
	{
		if (msg.params.GetInt("KEY_ID_OIS") == OIS::KC_SPACE)
		{
			Msg startJumpMsg;
			startJumpMsg.typeID = ObjectMessageIDs::INPUT_START_JUMP;
			BroadcastObjectMessage(startJumpMsg);
		}
	}

	if (msg.typeID == ObjectMessageIDs::CHARACTER_COLLISION)
	{
		int collisionFlags = msg.params.GetInt("collisionFlags");
	}
}

};