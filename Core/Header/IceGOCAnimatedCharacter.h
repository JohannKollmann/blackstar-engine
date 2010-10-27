#pragma once

#include "IceIncludes.h"
#include "OgrePhysXRagdoll.h"
#include "IceGOCOgreNode.h"
#include "IceMessageSystem.h"
#include "IceGOCScriptMakros.h"
#include "IceAnimationStateSystem.h"

namespace Ice
{
	class GOCAnimatedCharacter;
	struct DllExport RagBoneRef
	{
		Ogre::Bone *mBone;
		Ogre::SceneNode *mMeshNode;
		GOCAnimatedCharacter *mGOCRagdoll;
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