
#include "IcePlayAnimationProcess.h"
#include "IceProcessNodeManager.h"

namespace Ice
{


	PlayAnimationProcess::PlayAnimationProcess(Ogre::AnimationState *state)
	{
		mAnimationState = state;
		mAnimationState->setEnabled(true);
		mAnimationState->setWeight(0);
		mAnimationState->setTimePosition(0);
		mLooped = false;
		mInBlendDuration = 0.1f;
		mOutBlendDuration = 0.1f;
		mBlendingIn = true;
		mBlendingOut = false;
		mFinished = false;
		mWeight = 0;
		mTimeScale = 1;
		JoinNewsgroup(GlobalMessageIDs::REPARSE_SCRIPTS_PRE);
		JoinNewsgroup(GlobalMessageIDs::UPDATE_PER_FRAME);
	}
	PlayAnimationProcess::~PlayAnimationProcess()
	{
		mAnimationState->setEnabled(false);
	}
	void PlayAnimationProcess::OnSetActive(bool active)
	{
		if (active)
		{
			mBlendingIn = true;
			mBlendingOut = false;
			mAnimationState->setEnabled(true);
		}
		else
		{
			mBlendingIn = false;
			mBlendingOut = true;
		}
	}
	void PlayAnimationProcess::AddCallback(float timePos, ScriptParam callback)
	{
		Callback c; c.timePos = timePos; c.callback = callback;
		mScriptCallbacks.push_back(c);
	}
	void PlayAnimationProcess::ReceiveMessage(Msg &msg)
	{
		if (mAnimationState->getEnabled() && msg.typeID == GlobalMessageIDs::UPDATE_PER_FRAME)
		{
			float timeDelta = msg.params.GetFloat("TIME");
			timeDelta *= mTimeScale;

			if (mBlendingIn)
			{
				if (mInBlendDuration > 0) mWeight += timeDelta / mInBlendDuration;
				else mWeight = 1;
				if (mWeight >= 1)
				{
					mWeight = 1;
					mBlendingIn = false;
				}
			}
			else if (mBlendingOut)
			{
				if (mOutBlendDuration > 0)  mWeight -= (timeDelta / mOutBlendDuration);
				else mWeight = 0;
				if (mWeight <= 0)
				{
					mWeight = 0;
					mBlendingOut = false;
					mAnimationState->setEnabled(false);
					if (mFinished) ProcessNodeManager::Instance().RemoveProcessNode(GetProcessID());
					return;
				}
			}

			ITERATE(i, mScriptCallbacks)
				if (mAnimationState->getTimePosition()/mTimeScale <= i->timePos && (mAnimationState->getTimePosition()/mTimeScale)+timeDelta > i->timePos)
					ScriptSystem::GetInstance().RunCallbackFunction(i->callback, std::vector<ScriptParam>());

			mAnimationState->setWeight(mWeight);
			mAnimationState->addTime(timeDelta);
			if (!mLooped)
			{
				float remaining = (mAnimationState->getLength() - mAnimationState->getTimePosition())/mTimeScale;
				if (remaining < mOutBlendDuration)
				{
					mFinished = true;
					mBlendingIn = false;
					mBlendingOut = true;
				}
				else ProcessNodeManager::Instance().RemoveProcessNode(GetProcessID());
			}
		}
		else if (msg.typeID == GlobalMessageIDs::REPARSE_SCRIPTS_PRE)
		{
			mScriptCallbacks.clear();
		}
	}

	void PlayAnimationProcess::TerminateProcess()
	{
		//mFinished = true;
		mBlendingIn = false;
		mBlendingOut = true;
		TriggerWaitingProcesses();
	}
}