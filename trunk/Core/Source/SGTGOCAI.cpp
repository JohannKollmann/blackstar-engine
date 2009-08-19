
#include "SGTGOCAI.h"

SGTGOCAI::SGTGOCAI(void)
{
	mCharacterMovementState = 0;
	mActiveState = 0;
}

SGTGOCAI::~SGTGOCAI(void)
{
}

void SGTGOCAI::AddState(SGTAIState *state)
{
	if (!mActiveState)
	{
		mActiveState = state;
		return;
	}
	if (state->GetPriority() > mActiveState->GetPriority())
	{
		mQueue.push_front(mActiveState);
		mActiveState = state;
		return;
	}
	mQueue.push_back(state);
}

void SGTGOCAI::ClearStates()
{
	for (std::list<SGTAIState*>::iterator i = mQueue.begin(); i != mQueue.end(); i++)
	{
		delete (*i);
	}
	mQueue.clear();
	delete mActiveState;
	mActiveState = 0;
}

void SGTGOCAI::SelectState()
{
	if (mQueue.size() > 0)
	{
		std::list<SGTAIState*>::iterator highest = mQueue.begin();
		for (std::list<SGTAIState*>::iterator i = mQueue.begin(); i != mQueue.end(); i++)
		{
			if ((*i)->GetPriority() > (*highest)->GetPriority())
			{
				highest = i;
			}
		}
		mActiveState = (*highest);
		mQueue.erase(highest);
		mActiveState->OnEnter();
	}
}

void SGTGOCAI::ReceiveMessage(SGTMsg &msg)
{
	if (msg.mNewsgroup == "UPDATE_PER_FRAME")
	{
		float time = msg.mData.GetFloat("TIME");
		if (mActiveState)
		{
			bool finished = mActiveState->OnUpdate(time);
			if (finished)
			{
				mActiveState->OnLeave();
				delete mActiveState;
				mActiveState = 0;
				SelectState();
			}
		}
	}
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