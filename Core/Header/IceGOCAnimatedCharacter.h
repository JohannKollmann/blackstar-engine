#pragma once

#include "IceIncludes.h"
#include "OgrePhysXRagdoll.h"
#include "IceGOCOgreNode.h"
#include "IceMessageSystem.h"
#include "IceGOCScriptMakros.h"

namespace Ice
{

	class GOCAnimatedCharacter;
	struct DllExport RagBoneRef
	{
		Ogre::Bone *mBone;
		Ogre::SceneNode *mMeshNode;
		GOCAnimatedCharacter *mGOCRagdoll;
	};

	class DllExport Blendable
	{
	public:
		bool mBlendingIn;
		bool mBlendingOut;
		float mInBlendDuration;
		float mOutBlendDuration;
		float mWeight;

		float computeWeight(float timeDelta)
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

	public:
		virtual float GetMaxBlendTime() = 0;
		virtual bool RequestBlendOut(float targetBlendInTime) { return false; }
		virtual Ogre::String GetName() { return ""; }
		bool IsBlendingIn() { return mBlendingIn; }
		bool IsBlendingOut() { return mBlendingOut; }
		virtual void Update(float time) = 0;
		virtual Blendable* Clone() = 0;

		Blendable() : mBlendingIn(false), mBlendingOut(false), mWeight(1)
		{
		}
		virtual ~Blendable() {}

		virtual void OnBlendIn() {}
		virtual void OnBlendOut() {}
		void BlendIn(Blendable *source)
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
		void BlendIn(float blendDuration)
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
		void BlendOut(Blendable *target)
		{
			BlendOut(mOutBlendDuration);
		}
		void BlendOut(float blendDuration)
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
	
	};
	class DllExport BlendStateQueue : public Blendable
	{
	private:
		std::shared_ptr<Blendable> mCurrentState;
		std::list<std::shared_ptr<Blendable>> mQueue;
		std::shared_ptr<Blendable> mOutBlendState;
		
		void _outblendCurrent()
		{
			if (mCurrentState.get())
			{
				mOutBlendState = mCurrentState;
				//IceNote("reset current (" + mCurrentState->GetName() + "), use count: " + Ogre::StringConverter::toString(mCurrentState.use_count()))
				mCurrentState.reset();
			}
		}
		void _selectCurrent()
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

	public:
		~BlendStateQueue() {}
		BlendStateQueue() : mOutBlendState(nullptr), mCurrentState(nullptr) {}
		BlendStateQueue(const BlendStateQueue &rhs)			//deep copy
		{
			ITERATE(i, rhs.mQueue)
			{
				mQueue.push_back(std::shared_ptr<Blendable>((*i)->Clone()));
			}
		}

		bool IsEmpty()
		{
			return (mCurrentState.get() == nullptr && mQueue.empty());
		}
		float GetMaxBlendTime()
		{
			_selectCurrent();
			if (mCurrentState.get()) return mCurrentState->GetMaxBlendTime();
			return 10;
		}
		bool RequestBlendOut(float targetBlendInTime)
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
		void OnBlendIn()
		{
			if (mQueue.size() > 0 || mCurrentState.get())
			{
				if (!mCurrentState.get()) _selectCurrent();
				mCurrentState->BlendIn(mInBlendDuration);
			}
		}
		void OnBlendOut()
		{
			if (mCurrentState.get())
			{
				mCurrentState->BlendOut(mOutBlendDuration);
			}
		}
		void Clear()
		{
			mQueue.clear();
			mCurrentState.reset();
			mOutBlendState.reset();
		}
		void PushBack(std::shared_ptr<Blendable> state)
		{
			mQueue.push_back(state);
		}
		void PushFront(std::shared_ptr<Blendable> state)
		{
			if (mCurrentState.get())
			{
				mQueue.push_front(mCurrentState);	//write back to the queue
				_outblendCurrent();
			}
			mQueue.push_front(state);
		}
		void Pop(Ogre::String animationName)
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
		void Update(float time)
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
		OgreAnimationState(Ogre::AnimationState *state, float blendDuration, float timeScale) : mOgreAnimState(state), mTimeScale(timeScale)
		{
			mOgreAnimState->setTimePosition(0);
			mOutBlendDuration = blendDuration;
			mInBlendDuration = blendDuration;
		}
		~OgreAnimationState()
		{
			mOgreAnimState->setEnabled(false);
		}
		void AddCallback(float timePos, ScriptParam callback)
		{
			Callback c; c.timePos = timePos; c.callback = callback;
			mScriptCallbacks.push_back(c);
		}

		void Update(float time)
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
		bool RequestBlendOut(float targetBlendInTime)
		{
			if (mOgreAnimState->getLoop()) return false;
			float remaining = (mOgreAnimState->getLength() - mOgreAnimState->getTimePosition())/mTimeScale;
			return (remaining <= targetBlendInTime || remaining <= mOutBlendDuration);
		}
		void OnBlendIn()
		{
			mOgreAnimState->setEnabled(true);
		}
		void OnBlendOut()
		{
			if (mWeight <= 0)
				mOgreAnimState->setEnabled(false);
		}
		float GetMaxBlendTime()
		{
			return (mOgreAnimState->getLength()/mTimeScale);
		}
		Ogre::String GetName()
		{
			return mOgreAnimState->getAnimationName();
		}
		Blendable* Clone() { OgreAnimationState *anim =  new OgreAnimationState(mOgreAnimState, mInBlendDuration, mTimeScale); anim->mScriptCallbacks = mScriptCallbacks; return anim; }
	};

	class DllExport AnimState : public Blendable
	{
	private:
		Ogre::String mName;
		std::map<int, BlendStateQueue> mAnimationQueues;
		bool _isFinished()
		{
			ITERATE(i, mAnimationQueues)
			{
				if (!i->second.IsEmpty()) return false;
			}
			return true;
		}
	public:
		~AnimState()
		{
			mAnimationQueues.clear();
		}
		AnimState(Ogre::String name, float blendDuration) : mName(name)
		{
			mOutBlendDuration = blendDuration;
			mInBlendDuration = blendDuration;
		}
		AnimState(const AnimState &rhs)
		{
			mName = rhs.mName;
			mAnimationQueues = rhs.mAnimationQueues;
			mOutBlendDuration = rhs.mOutBlendDuration;
			mInBlendDuration = rhs.mInBlendDuration;
		}
		Ogre::String GetName() { return mName; }
		void Update(float time)
		{
			computeWeight(time);
			ITERATE(i, mAnimationQueues)
			{
				i->second.Update(time);
			}
		}
		bool RequestBlendOut(float targetBlendInTime)
		{
			ITERATE(i, mAnimationQueues)
			{
				if (!i->second.RequestBlendOut(targetBlendInTime)) return false;
			}
			return true;
		}
		float GetMaxBlendTime()
		{
			if (_isFinished()) return 0;
			float maxBlendTime = 10;
			ITERATE(i, mAnimationQueues)
			{
				if (i->second.GetMaxBlendTime() < maxBlendTime) maxBlendTime = i->second.GetMaxBlendTime();
			}
			return maxBlendTime;
		}
		void OnBlendIn()
		{
			ITERATE(i, mAnimationQueues) i->second.BlendIn(mInBlendDuration);
		}
		void OnBlendOut()
		{
			ITERATE(i, mAnimationQueues) i->second.BlendOut(mOutBlendDuration);
		}

		void AnimPushFront(int queueID, std::shared_ptr<OgreAnimationState> state)
		{
			if (mAnimationQueues.find(queueID) == mAnimationQueues.end()) mAnimationQueues.insert(std::make_pair(queueID, BlendStateQueue()));
			mAnimationQueues[queueID].PushFront(state);
		}
		void AnimPushBack(int queueID, std::shared_ptr<OgreAnimationState> state)
		{
			if (mAnimationQueues.find(queueID) == mAnimationQueues.end()) mAnimationQueues.insert(std::make_pair(queueID, BlendStateQueue()));
			mAnimationQueues[queueID].PushBack(state);
		}
		void AnimPop(int queueID, Ogre::String animName)
		{
			if (mAnimationQueues.find(queueID) != mAnimationQueues.end()) mAnimationQueues[queueID].Pop(animName);
		}
		Blendable* Clone() { return new AnimState(*this); }

	};

	class DllExport GOCAnimatedCharacter : public GOCEditorInterface, public GOCOgreNodeUser, public ScriptUser
	{
	private:
		Ogre::Entity *mEntity;
		OgrePhysX::Ragdoll *mRagdoll;

		BlendStateQueue mAnimationQueue;
		std::map<Ogre::String, std::shared_ptr<AnimState>> mHighLevelAnimationStates;

		bool mSetControlToActorsTemp;
		bool mEditorMode;
		Ogre::String mMeshName;
		bool mShadowCaster;

		void _clear();

		void Create(Ogre::String meshname, Ogre::Vector3 scale);

	public:
		GOCAnimatedCharacter();
		GOCAnimatedCharacter(Ogre::String meshname, Ogre::Vector3 scale = Ogre::Vector3(1,1,1));
		~GOCAnimatedCharacter(void);

		OgrePhysX::Ragdoll* GetRagdoll() { return mRagdoll; }

		std::list<GameObject*> mBoneObjects;

		void CreateBoneObjects();
		void SerialiseBoneObjects(Ogre::String filename);

		void Kill();

		goc_id_family& GetFamilyID() const { static std::string name = "View"; return name; }
		GOComponent::goc_id_type& GetComponentID() const { static std::string name = "Skeleton"; return name; }

		void OnReceiveMessage(Msg &msg);
		void ReceiveObjectMessage(Msg &msg);

		void SetOwner(GameObject *go);
		void SetParameters(DataMap *parameters);
		void GetParameters(DataMap *parameters);
		void GetDefaultParameters(DataMap *parameters);
		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		virtual std::string& TellName() { static std::string name = "Skeleton"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "Skeleton"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCAnimatedCharacter(); };
		GOCEditorInterface* New() { return new GOCAnimatedCharacter(); }
		Ogre::String GetLabel() { return "Skeleton"; }
		GOComponent* GetGOComponent() { return this; }

		//Scripting
		int GetThisID() { IceAssert(mOwnerGO); return mOwnerGO->GetID(); }

		std::vector<ScriptParam> Anim_Play(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> AnimState_Create(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> AnimState_EnqueueAnimation(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> AnimState_Push(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> AnimState_Pop(Script& caller, std::vector<ScriptParam> &vParams);

		//Methods to setup states the time
		DEFINE_TYPEDGOCLUAMETHOD(GOCAnimatedCharacter, AnimState_Create, "string")	//state name
		DEFINE_TYPEDGOCLUAMETHOD(GOCAnimatedCharacter, AnimState_EnqueueAnimation, "string string int")		//state name, animation name, queue ID

		//Methods to control state flow
		DEFINE_TYPEDGOCLUAMETHOD(GOCAnimatedCharacter, AnimState_Push, "string")
		DEFINE_TYPEDGOCLUAMETHOD(GOCAnimatedCharacter, AnimState_Pop, "string")

		//Plays an animation at runtime, useful for things like a hit feedback
		DEFINE_TYPEDGOCLUAMETHOD(GOCAnimatedCharacter, Anim_Play, "string int")	//animation name, queueID
	};

	class DllExport GOCAnimatedCharacterBone : public GOCOgreNodeUser, public GOCEditorInterface
	{
	private:
		Ogre::Entity *mEntity;
		Ogre::Bone *mBone;
		Ogre::SceneNode *mMeshNode;
		Ogre::SceneNode *mOffsetNode;
		Ogre::SceneNode *mJointAxisNode;
		Ogre::ManualObject *mJointAxis;
		OgrePhysX::sBoneActorBindConfig mBoneConfig;
		bool mDebugAnimation;
		Ogre::Quaternion mBoneActorGlobalBindOrientationInverse;
		Ogre::Quaternion mBoneGlobalBindOrientation;
		Ogre::Vector3 mGlobalBindPosition;
		RagBoneRef* mRagBoneRef;

		void ScaleNode();

	public:
		GOCAnimatedCharacterBone(void);
		void Init();
		~GOCAnimatedCharacterBone(void);

		GOComponent::goc_id_type& GetComponentID() const { static std::string name = "AnimatedCharacterBone"; return name; }

		void SetParameters(DataMap *parameters);
		void GetParameters(DataMap *parameters);
		void GetDefaultParameters(DataMap *parameters);

		void CreateJointAxis();

		bool GetTestAnimation();

		OgrePhysX::sBoneActorBindConfig GetBoneConfig() { return mBoneConfig; }

		void UpdatePosition(Ogre::Vector3 position);
		void UpdateScale(Ogre::Vector3 scale);
		void UpdateOrientation(Ogre::Quaternion orientation);

		void SetBone(Ogre::SceneNode *meshnode, GOCAnimatedCharacter* ragdoll, OgrePhysX::sBoneActorBind &bone_config, bool controlBone);

		void SetOwner(GameObject *go);

		void Save(LoadSave::SaveSystem& mgr) {};
		void Load(LoadSave::LoadSystem& mgr) {};
		std::string& TellName() { static std::string name = "AnimatedCharacterBone"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "AnimatedCharacterBone"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCAnimatedCharacterBone; };
		GOCEditorInterface* New() { return new GOCAnimatedCharacterBone(); }
		Ogre::String GetLabel() { return ""; }
		GOComponent* GetGOComponent() { return this; }
	};

};