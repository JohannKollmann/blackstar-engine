
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
		mActionQueue.push_front(mActiveState);
		mActiveState = state;
		return;
	}
	mActionQueue.push_back(state);
}

void SGTGOCAI::ClearActionQueue()
{
	for (std::list<SGTAIState*>::iterator i = mActionQueue.begin(); i != mActionQueue.end(); i++)
	{
		delete (*i);
	}
	mActionQueue.clear();
	delete mActiveState;
	mActiveState = 0;
}

void SGTGOCAI::ClearIdleQueue()
{
	for (std::list<SGTScriptedAIState*>::iterator i = mIdleQueue.begin(); i != mIdleQueue.end(); i++)
	{
		delete (*i);
	}
	mIdleQueue.clear();
}

void SGTGOCAI::SelectState()
{
	if (mActionQueue.size() > 0)
	{
		std::list<SGTAIState*>::iterator highest = mActionQueue.begin();
		for (std::list<SGTAIState*>::iterator i = mActionQueue.begin(); i != mActionQueue.end(); i++)
		{
			if ((*i)->GetPriority() > (*highest)->GetPriority())
			{
				highest = i;
			}
		}
		mActiveState = (*highest);
		mActionQueue.erase(highest);
		mActiveState->OnEnter();
	}
	else mActiveState = 0;
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
		else if (mIdleQueue.size() > 0)
		{
			SGTScriptedAIState *state = *mIdleQueue.begin();
			if (state->OnUpdate())
			{
				mIdleQueue.pop_front();
				mIdleQueue.push_back(state);
				(*mIdleQueue.begin())->OnEnter();
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