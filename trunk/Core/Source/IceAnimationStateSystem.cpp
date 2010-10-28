
#include "IceAnimationStateSystem.h"

namespace Ice
{

	float Blendable::computeWeight(float timeDelta)
	{
		if (mBlendingIn && mInBlendDuration > 0)
		{
			mWeight += timeDelta / mInBlendDuration;
			if (mWeight >= 1)
			{
				mWeight = 1;
				mBlendingIn = false;
			}
		}
		else if (mBlendingOut && mOutBlendDuration > 0)
		{
			mWeight -= (timeDelta / mOutBlendDuration);
			if (mWeight <= 0)
			{
				mWeight = 0;
				mBlendingOut = false;
			}
		}
		return mWeight;
	}
	void Blendable::BlendIn(Blendable *source)
	{
		if (mInBlendDuration > GetMaxBlendTime()) mInBlendDuration = GetMaxBlendTime();
		if (source)
		{
			if (source->mOutBlendDuration > source->GetMaxBlendTime()) source->mOutBlendDuration = source->GetMaxBlendTime();
			if (mInBlendDuration > source->mOutBlendDuration) mInBlendDuration = source->mOutBlendDuration;
			source->mOutBlendDuration = mInBlendDuration;
		}
		BlendIn(mInBlendDuration);
	}
	void Blendable::BlendIn(float blendDuration)
	{
		mInBlendDuration = blendDuration;
		mBlendingOut = false;
		if (mInBlendDuration > 0)
		{
			mWeight = 0;
			mBlendingIn = true;
		}
		else 
		{
			mWeight = 1;
			mBlendingIn = false;
		}
		OnBlendIn();
	}
	void Blendable::BlendOut(Blendable *target)
	{
		BlendOut(mOutBlendDuration);
	}
	void Blendable::BlendOut(float blendDuration)
	{
		mOutBlendDuration = blendDuration;
		mBlendingIn = false;
		if (mOutBlendDuration > 0)
		{
			mWeight = 1;
			mBlendingOut = true;
		}
		else 
		{
			mWeight = 0;
			mBlendingOut = false;
		}
		OnBlendOut();
	}


	void BlendStateQueue::_outblendCurrent()
	{
		if (mCurrentState.get())
		{
			if (mOutBlendState) mOutBlendState->BlendOut(0.0f);
			mOutBlendState = mCurrentState;
			//IceNote("reset current (" + mCurrentState->GetName() + "), use count: " + Ogre::StringConverter::toString(mCurrentState.use_count()))
			mCurrentState.reset();
		}
	}
	void BlendStateQueue::_selectCurrent()
	{
		if (mQueue.size() > 0)
		{
			if (mCurrentState.get() == nullptr)
			{
				mCurrentState = mQueue.front();
				mCurrentState->BlendIn(mOutBlendState.get());
				if (mOutBlendState.get())
					mOutBlendState->BlendOut(mCurrentState.get());
				mQueue.pop_front();
			}
		}
	}
	BlendStateQueue::BlendStateQueue(const BlendStateQueue &rhs)			//deep copy
	{
		ITERATE(i, rhs.mQueue)
		{
			mQueue.push_back(std::shared_ptr<Blendable>((*i)->Clone()));
		}
	}
	bool BlendStateQueue::IsEmpty()
	{
		return (mCurrentState.get() == nullptr && mQueue.empty());
	}
	float BlendStateQueue::GetMaxBlendTime()
	{
		_selectCurrent();
		if (mCurrentState.get()) return mCurrentState->GetMaxBlendTime();
		return 10;
	}
	bool BlendStateQueue::RequestBlendOut(float targetBlendInTime)
	{
		_selectCurrent();
		if (mQueue.empty())
		{
			if (mCurrentState.get()) return mCurrentState->RequestBlendOut(targetBlendInTime);
			else if (mOutBlendState.get()) return mOutBlendState->RequestBlendOut(targetBlendInTime);
			return true;
		}
		else return false;
	}
	void BlendStateQueue::OnBlendIn()
	{
		if (mQueue.size() > 0 || mCurrentState.get())
		{
			if (!mCurrentState.get()) _selectCurrent();
			mCurrentState->BlendIn(mInBlendDuration);
		}
	}
	void BlendStateQueue::OnBlendOut()
	{
		if (mCurrentState.get())
		{
			mCurrentState->BlendOut(mOutBlendDuration);
		}
	}
	void BlendStateQueue::Clear()
	{
		mQueue.clear();
		mCurrentState.reset();
		mOutBlendState.reset();
	}
	void BlendStateQueue::PushBack(std::shared_ptr<Blendable> state)
	{
		mQueue.push_back(state);
	}
	void BlendStateQueue::PushFront(std::shared_ptr<Blendable> state)
	{
		if (mCurrentState.get())
		{
			mQueue.push_front(mCurrentState);	//write back to the queue
			_outblendCurrent();
		}
		mQueue.push_front(state);
	}
	void BlendStateQueue::Pop(Ogre::String animationName)
	{
		if (mCurrentState.get() && mCurrentState->GetName() == animationName) _outblendCurrent();
		else
		{
			ITERATE(i, mQueue)
			{
				if ((*i)->GetName() == animationName)
				{
					mQueue.erase(i);
					break;
				}
			}
		}
	}
	void BlendStateQueue::Update(float time)
	{
		_selectCurrent();
		if (mOutBlendState.get())
		{
			if (!mOutBlendState->IsBlendingOut())
			{
				//IceNote("reset blending out (" + mOutBlendState->GetName() + "), use count: " + Ogre::StringConverter::toString(mOutBlendState.use_count()))
				mOutBlendState.reset();
			}
			else mOutBlendState->Update(time);
		}
		if (mCurrentState.get())
		{
			mCurrentState->Update(time);
			float targetInBlendTime = 0;
			if (!mQueue.empty()) targetInBlendTime = mQueue.front()->mInBlendDuration;
			if (mCurrentState->RequestBlendOut(targetInBlendTime)) _outblendCurrent();
		}
	}


	OgreAnimationState::OgreAnimationState(Ogre::AnimationState *state, float blendDuration, float timeScale) : mOgreAnimState(state), mTimeScale(timeScale)
	{
		mOgreAnimState->setTimePosition(0);
		mOutBlendDuration = blendDuration;
		mInBlendDuration = blendDuration;
	}
	OgreAnimationState::~OgreAnimationState()
	{
		mOgreAnimState->setEnabled(false);
	}
	void OgreAnimationState::AddCallback(float timePos, ScriptParam callback)
	{
		Callback c; c.timePos = timePos; c.callback = callback;
		mScriptCallbacks.push_back(c);
	}
	void OgreAnimationState::Update(float time)
	{
		time *= mTimeScale;
		ITERATE(i, mScriptCallbacks)
			if (mOgreAnimState->getTimePosition()/mTimeScale <= i->timePos && (mOgreAnimState->getTimePosition()/mTimeScale)+time > i->timePos)
				ScriptSystem::GetInstance().RunCallbackFunction(i->callback, std::vector<ScriptParam>());

		float weight = computeWeight(time);
		if (weight <= 0)
			mOgreAnimState->setEnabled(false);
		else mOgreAnimState->setEnabled(true);
		mOgreAnimState->setWeight(weight);
		mOgreAnimState->addTime(time);
	}
	bool OgreAnimationState::RequestBlendOut(float targetBlendInTime)
	{
		if (mOgreAnimState->getLoop()) return false;
		float remaining = (mOgreAnimState->getLength() - mOgreAnimState->getTimePosition())/mTimeScale;
		return (remaining <= targetBlendInTime || remaining <= mOutBlendDuration);
	}
	void OgreAnimationState::OnBlendIn()
	{
		mOgreAnimState->setEnabled(true);
	}
	void OgreAnimationState::OnBlendOut()
	{
		if (mWeight <= 0)
			mOgreAnimState->setEnabled(false);
	}
	float OgreAnimationState::GetMaxBlendTime()
	{
		return (mOgreAnimState->getLength()/mTimeScale);
	}
	Ogre::String OgreAnimationState::GetName()
	{
		return mOgreAnimState->getAnimationName();
	}


	bool AnimState::_isFinished()
	{
		ITERATE(i, mAnimationQueues)
		{
			if (!i->second.IsEmpty()) return false;
		}
		return true;
	}
	AnimState::~AnimState()
	{
		mAnimationQueues.clear();
	}
	AnimState::AnimState(Ogre::String name, float blendDuration) : mName(name)
	{
		mOutBlendDuration = blendDuration;
		mInBlendDuration = blendDuration;
	}
	AnimState::AnimState(const AnimState &rhs)
	{
		mName = rhs.mName;
		mAnimationQueues = rhs.mAnimationQueues;
		mOutBlendDuration = rhs.mOutBlendDuration;
		mInBlendDuration = rhs.mInBlendDuration;
	}
	void AnimState::Update(float time)
	{
		computeWeight(time);
		ITERATE(i, mAnimationQueues)
		{
			i->second.Update(time);
		}
	}
	bool AnimState::RequestBlendOut(float targetBlendInTime)
	{
		ITERATE(i, mAnimationQueues)
		{
			if (!i->second.RequestBlendOut(targetBlendInTime)) return false;
		}
		return true;
	}
	float AnimState::GetMaxBlendTime()
	{
		if (_isFinished()) return 0;
		float maxBlendTime = 10;
		ITERATE(i, mAnimationQueues)
		{
			if (i->second.GetMaxBlendTime() < maxBlendTime) maxBlendTime = i->second.GetMaxBlendTime();
		}
		return maxBlendTime;
	}
	void AnimState::OnBlendIn()
	{
		ITERATE(i, mAnimationQueues) i->second.BlendIn(mInBlendDuration);
	}
	void AnimState::OnBlendOut()
	{
		ITERATE(i, mAnimationQueues) i->second.BlendOut(mOutBlendDuration);
	}

	void AnimState::AnimPushFront(int queueID, std::shared_ptr<OgreAnimationState> state)
	{
		if (mAnimationQueues.find(queueID) == mAnimationQueues.end()) mAnimationQueues.insert(std::make_pair(queueID, BlendStateQueue()));
		mAnimationQueues[queueID].PushFront(state);
	}
	void AnimState::AnimPushBack(int queueID, std::shared_ptr<OgreAnimationState> state)
	{
		if (mAnimationQueues.find(queueID) == mAnimationQueues.end()) mAnimationQueues.insert(std::make_pair(queueID, BlendStateQueue()));
		mAnimationQueues[queueID].PushBack(state);
	}
	void AnimState::AnimPop(int queueID, Ogre::String animName)
	{
		if (mAnimationQueues.find(queueID) != mAnimationQueues.end()) mAnimationQueues[queueID].Pop(animName);
	}
}