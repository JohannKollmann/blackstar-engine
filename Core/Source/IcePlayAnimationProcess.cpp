
#include "IcePlayAnimationProcess.h"

namespace Ice
{


	PlayAnimationProcess::PlayAnimationProcess(Ogre::AnimationState *state)
	{
		mAnimationState = state;
		mLooped = false;
		mInBlendDuration = 0.1f;
		mOutBlendDuration = 0.1f;
		mBlendingIn = true;
		mBlendingOut = false;
		mWeight = 0;
		mTimeScale = 1;
	}
	PlayAnimationProcess::~PlayAnimationProcess()
	{
	}
	void PlayAnimationProcess::OnSetActive(bool active)
	{
		if (active)
		{
			mBlendingIn = true;
			mBlendingOut = false;
			mAnimationState->setEnabled(true);
			MessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
		}
		else
		{
			mAnimationState->setEnabled(false);
			MessageSystem::Instance().QuitAllNewsgroups(this);
			mBlendingIn = false;
			mBlendingOut = false;
		}
	}
	void PlayAnimationProcess::AddCallback(float timePos, ScriptParam callback)
	{
		Callback c; c.timePos = timePos; c.callback = callback;
		mScriptCallbacks.push_back(c);
	}
	void PlayAnimationProcess::ReceiveMessage(Msg &msg)
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
				Terminate();
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
				mBlendingIn = false;
				mBlendingOut = true;
			}
		}
	}
}