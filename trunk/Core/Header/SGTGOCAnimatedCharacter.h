#pragma once

#include "SGTIncludes.h"
#include "OgrePhysXRagdoll.h"
#include "SGTGOCView.h"
#include "SGTMessageSystem.h"

class SGTGOCAnimatedCharacter;
struct RagBoneRef
{
	Ogre::Bone *mBone;
	Ogre::SceneNode *mMeshNode;
	SGTGOCAnimatedCharacter *mGOCRagdoll;
};

class SGTDllExport SGTGOCAnimatedCharacterBone : public SGTGOCNodeRenderable, public SGTGOCEditorInterface
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
	SGTGOCAnimatedCharacterBone(void);
	~SGTGOCAnimatedCharacterBone(void);

	goc_id_family& GetFamilyID() const { static std::string name = "AnimatedCharacterBone"; return name; }
	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "AnimatedCharacterBone"; return name; }

	void CreateFromDataMap(SGTDataMap *parameters);
	void GetParameters(SGTDataMap *parameters);
	static void GetDefaultParameters(SGTDataMap *parameters);
	void* GetUserData();
	void InjectUserData(void* data);

	void CreateJointAxis();

	bool GetTestAnimation();

	OgrePhysX::sBoneActorBindConfig GetBoneConfig() { return mBoneConfig; }

	void UpdatePosition(Ogre::Vector3 position);
	void UpdateScale(Ogre::Vector3 scale);
	void UpdateOrientation(Ogre::Quaternion orientation);

	void SetBone(Ogre::SceneNode *meshnode, SGTGOCAnimatedCharacter* ragdoll, OgrePhysX::sBoneActorBind &bone_config, bool controlBone);

	void SetOwner(SGTGameObject *go);

	void Save(SGTSaveSystem& mgr) {};
	void Load(SGTLoadSystem& mgr) {};
	std::string& TellName() { static std::string name = "AnimatedCharacterBone"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "AnimatedCharacterBone"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTSaveable* NewInstance() { return new SGTGOCAnimatedCharacterBone; };
	static SGTGOCEditorInterface* NewEditorInterfaceInstance() { return new SGTGOCAnimatedCharacterBone(); }
	void AttachToGO(SGTGameObject *go);
};

class SGTDllExport SGTGOCAnimatedCharacter : public SGTGOCEditorInterface, public SGTGOCNodeRenderable, public SGTMessageListener
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
	std::map<AnimationID, Ogre::String> mMovementAnimations;
	int mMovementState;

	void Create(Ogre::String meshname, Ogre::Vector3 scale);

public:
	SGTGOCAnimatedCharacter();
	SGTGOCAnimatedCharacter(Ogre::String meshname, Ogre::Vector3 scale = Ogre::Vector3(1,1,1));
	~SGTGOCAnimatedCharacter(void);

	OgrePhysX::Ragdoll* GetRagdoll() { return mRagdoll; }

	void SetAnimationState(Ogre::String statename);

	std::list<SGTGameObject*> mBoneObjects;

	void CreateBoneObjects();
	void SerialiseBoneObjects(Ogre::String filename);

	void Kill();

	goc_id_family& GetFamilyID() const { static std::string name = "Skeleton"; return name; }
	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "Skeleton"; return name; }

	void ReceiveMessage(SGTMsg &msg);
	void ReceiveObjectMessage(Ogre::SharedPtr<SGTObjectMsg> msg);

	void SetOwner(SGTGameObject *go);
	void CreateFromDataMap(SGTDataMap *parameters);
	void GetParameters(SGTDataMap *parameters);
	static void GetDefaultParameters(SGTDataMap *parameters);
	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	virtual std::string& TellName() { static std::string name = "Skeleton"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "Skeleton"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTSaveable* NewInstance() { return new SGTGOCAnimatedCharacter(); };
	static SGTGOCEditorInterface* NewEditorInterfaceInstance() { return new SGTGOCAnimatedCharacter(); }
	void AttachToGO(SGTGameObject *go);
};