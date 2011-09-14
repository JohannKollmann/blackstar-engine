#pragma once

#include "IceIncludes.h"
#include "OgrePhysXRagdoll.h"
#include "IceGOCOgreNode.h"
#include "IceMessageSystem.h"
#include "IceGOCScriptMakros.h"
#include "IcePlayAnimationProcess.h"
#include "IceProcessOwner.h"

namespace Ice
{
	class GOCAnimatedCharacter;
	struct DllExport RagBoneRef
	{
		Ogre::Bone *mBone;
		Ogre::SceneNode *mMeshNode;
		GOCAnimatedCharacter *mGOCRagdoll;
	};

	class DllExport GOCAnimatedCharacter : public GOCEditorInterface, public GOCOgreNodeUser, public ProcessOwner
	{
	private:
		Ogre::Entity *mEntity;
		OgrePhysX::Ragdoll *mRagdoll;

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

		AccessPermissionID GetAccessPermissionID() { return AccessPermissions::ACCESS_PHYSICS; }

		OgrePhysX::Ragdoll* GetRagdoll() { return mRagdoll; }

		std::list<GameObject*> mBoneObjects;

		void CreateBoneObjects();
		void SerialiseBoneObjects(Ogre::String filename);

		void Kill();

		GOComponent::FamilyID& GetFamilyID() const { static std::string name = "View"; return name; }
		GOComponent::TypeID& GetComponentID() const { static std::string name = "Skeleton"; return name; }

		void ReceiveMessage(Msg &msg);

		void SetOwner(std::weak_ptr<GameObject> go);
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

		std::vector<ScriptParam> AnimProcess_Create(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> AnimProcess_GetBinding(Script& caller, std::vector<ScriptParam> &vParams);

		//Methods to setup states the time
		DEFINE_TYPEDGOCLUAMETHOD(GOCAnimatedCharacter, AnimProcess_Create, "string")	//anim state name, looped, blendTime, timeScale, callbacks
	};
};