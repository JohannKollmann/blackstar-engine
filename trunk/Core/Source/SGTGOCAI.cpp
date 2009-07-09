
#include "SGTGOCAI.h"

SGTGOCAI::SGTGOCAI(void)
{
	mCharacterMovementState = 0;
}

SGTGOCAI::~SGTGOCAI(void)
{
}

void SGTGOCAI::ReceiveMessage(SGTMsg &msg)
{
}

void SGTGOCAI::ReceiveObjectMessage(Ogre::SharedPtr<SGTObjectMsg> msg)
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