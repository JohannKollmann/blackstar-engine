#pragma once

#include "IceIncludes.h"
#include "IceScriptSystem.h"
#include "Ogre.h"

namespace Ice
{

	class DllExport Blendable
	{
	public:
		bool mBlendingIn;
		bool mBlendingOut;
		float mInBlendDuration;
		float mOutBlendDuration;
		float mWeight;

		float computeWeight(float timeDelta);

	public:
		virtual float GetMaxBlendTime() = 0;
		virtual bool RequestBlendOut(float targetBlendInTime) { return false; }
		virtual Ogre::String GetName() { return ""; }
		bool IsBlendingIn() { return mBlendingIn; }
		bool IsBlendingOut() { return mBlendingOut; }
		virtual void Update(float time) = 0;
		virtual Blendable* Clone() = 0;

		Blendable() : mBlendingIn(false), mBlendingOut(false), mWeight(1) {}
		virtual ~Blendable() {}

		virtual void OnBlendIn() {}
		virtual void OnBlendOut() {}
		void BlendIn(Blendable *source);
		void BlendIn(float blendDuration);
		void BlendOut(Blendable *target);
		void BlendOut(float blendDuration);
	
	};
	class DllExport BlendStateQueue : public Blendable
	{
	private:
		std::shared_ptr<Blendable> mCurrentState;
		std::list<std::shared_ptr<Blendable>> mQueue;
		std::shared_ptr<Blendable> mOutBlendState;
		
		void _outblendCurrent();
		void _selectCurrent();

	public:
		~BlendStateQueue() {}
		BlendStateQueue() : mOutBlendState(nullptr), mCurrentState(nullptr) {}
		BlendStateQueue(const BlendStateQueue &rhs);			//deep copy

		bool IsEmpty();
		float GetMaxBlendTime();
		bool RequestBlendOut(float targetBlendInTime);
		void OnBlendIn();
		void OnBlendOut();
		void Clear();
		void PushBack(std::shared_ptr<Blendable> state);
		void PushFront(std::shared_ptr<Blendable> state);
		void Pop(Ogre::String animationName);
		void Update(float time);

		Blendable* Clone() { IceAssert(false); return nullptr; }		//Never-ever-never
	};

	class DllExport OgreAnimationState : public Blendable
	{
	private:
		Ogre::AnimationState *mOgreAnimState;
		float mTimeScale;
		struct Callback { float timePos; ScriptParam callback; };
		std::vector<Callback> mScriptCallbacks;
	public:
		OgreAnimationState(Ogre::AnimationState *state, float blendDuration, float timeScale);
		~OgreAnimationState();

		void AddCallback(float timePos, ScriptParam callback);
		void Update(float time);
		bool RequestBlendOut(float targetBlendInTime);
		void OnBlendIn();
		void OnBlendOut();
		float GetMaxBlendTime();
		Ogre::String GetName();

		Blendable* Clone() { OgreAnimationState *anim =  new OgreAnimationState(mOgreAnimState, mInBlendDuration, mTimeScale); anim->mScriptCallbacks = mScriptCallbacks; return anim; }
	};

	class DllExport AnimState : public Blendable
	{
	private:
		Ogre::String mName;
		std::map<int, BlendStateQueue> mAnimationQueues;
		bool _isFinished();

	public:
		~AnimState();
		AnimState(Ogre::String name, float blendDuration);
		AnimState(const AnimState &rhs);

		Ogre::String GetName() { return mName; }
		void Update(float time);
		bool RequestBlendOut(float targetBlendInTime);
		float GetMaxBlendTime();
		void OnBlendIn();
		void OnBlendOut();

		void AnimPushFront(int queueID, std::shared_ptr<OgreAnimationState> state);
		void AnimPushBack(int queueID, std::shared_ptr<OgreAnimationState> state);
		void AnimPop(int queueID, Ogre::String animName);

		Blendable* Clone() { return new AnimState(*this); }

	};
}