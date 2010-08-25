#pragma once

#include "IceIncludes.h"
#include "OgrePhysXRagdoll.h"
#include "IceGOCOgreNode.h"
#include "IceMessageSystem.h"

namespace Ice
{

class GOCAnimatedCharacter;
struct RagBoneRef
{
	Ogre::Bone *mBone;
	Ogre::SceneNode *mMeshNode;
	GOCAnimatedCharacter *mGOCRagdoll;
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
	static LoadSave::Saveable* NewInstance() { return ICE_NEW GOCAnimatedCharacterBone; };
	GOCEditorInterface* New() { return ICE_NEW GOCAnimatedCharacterBone(); }
	Ogre::String GetLabel() { return ""; }
	GOComponent* GetGOComponent() { return this; }
};

class DllExport GOCAnimatedCharacter : public GOCEditorInterface, public GOCOgreNodeUser, public MessageListener
{
	enum AnimationID
	{
		IDLE,
		JUMP,
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		CROUCH,
		RUN
	};
private:
	Ogre::Entity *mEntity;
	Ogre::AnimationState *mAnimationState;
	OgrePhysX::Ragdoll *mRagdoll;

	void ResetMovementAnis();
	void GetMovementAnis(Ogre::String configfile);

	bool mSetControlToActorsTemp;
	bool mEditorMode;
	Ogre::String mAnimationStateStr;
	Ogre::String mMeshName;
	bool mShadowCaster;
	std::map<AnimationID, Ogre::String> mMovementAnimations;
	int mMovementState;

	void _clear();

	void Create(Ogre::String meshname, Ogre::Vector3 scale);

public:
	GOCAnimatedCharacter();
	GOCAnimatedCharacter(Ogre::String meshname, Ogre::Vector3 scale = Ogre::Vector3(1,1,1));
	~GOCAnimatedCharacter(void);

	OgrePhysX::Ragdoll* GetRagdoll() { return mRagdoll; }

	void SetAnimationState(Ogre::String statename);

	std::list<GameObject*> mBoneObjects;

	void CreateBoneObjects();
	void SerialiseBoneObjects(Ogre::String filename);

	void Kill();

	goc_id_family& GetFamilyID() const { static std::string name = "View"; return name; }
	GOComponent::goc_id_type& GetComponentID() const { static std::string name = "Skeleton"; return name; }

	void ReceiveMessage(Msg &msg);
	void ReceiveObjectMessage(Msg &msg);

	void SetOwner(GameObject *go);
	void SetParameters(DataMap *parameters);
	void GetParameters(DataMap *parameters);
	void GetDefaultParameters(DataMap *parameters);
	void Save(LoadSave::SaveSystem& mgr);
	void Load(LoadSave::LoadSystem& mgr);
	virtual std::string& TellName() { static std::string name = "Skeleton"; return name; };
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "Skeleton"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
	static LoadSave::Saveable* NewInstance() { return ICE_NEW GOCAnimatedCharacter(); };
	GOCEditorInterface* New() { return ICE_NEW GOCAnimatedCharacter(); }
	Ogre::String GetLabel() { return "Skeleton"; }
	GOComponent* GetGOComponent() { return this; }
};

};