
#pragma once

#include "Ogre.h"
#include "NxOgre.h"
#include <vector>
#include "SGTIncludes.h"
#include "SGTMessageListener.h"	//Temp
#include "SGTMessageSystem.h"
#include "SGTGOCView.h"
#include "SGTGOCEditorInterface.h"

struct sD6Joint
{
	NxD6Joint *mJoint;
	NxD6JointDesc mDescription;
};

class SGTGOCRagdollBone;

struct sJointLimitParams
{
	float mValue;
	float mDamping;
	float mRestitution;
	float mSpring;
};


struct sBoneActorBind
{
	Ogre::Bone *mBone;
	NxOgre::Actor *mActor;
	sBoneActorBind *mParent;
	SGTGameObject *mVisualBone;
	Ogre::String mParentBoneName;
	sD6Joint mJoint;
	bool mNeedsJointOrientation;
	Ogre::Quaternion mJointOrientation;
	Ogre::Vector3 mOffset;
	float mBoneLength;
	float mBoneRadius;
	bool mHinge;
	sJointLimitParams mSwing1;
	sJointLimitParams mSwing2;
	sJointLimitParams mTwistMax;
	sJointLimitParams mTwistMin;
	Ogre::Vector3 mBoneGlobalBindPosition;
	Ogre::Quaternion mBoneActorGlobalBindOrientation;
	Ogre::Quaternion mBoneGlobalBindOrientation;
	Ogre::Quaternion mBoneActorGlobalBindOrientationInverse;
};

struct sBoneActorBindConfig
{
	Ogre::String mBoneName;
	Ogre::String mParentName;
	float mBoneLength;
	Ogre::Vector3 mBoneOffset;
	Ogre::Quaternion mBoneOrientation;
	float mRadius;
	Ogre::Quaternion mJointOrientation;
	bool mNeedsJointOrientation;
	bool mHinge;
	sJointLimitParams mSwing1;
	sJointLimitParams mSwing2;
	sJointLimitParams mTwistMax;
	sJointLimitParams mTwistMin;
};

class SGTRagdoll;
struct RagBoneRef
{
	Ogre::Bone *mBone;
	Ogre::SceneNode *mMeshNode;
	SGTRagdoll *mRagdoll;
};

class SGTDllExport SGTGOCRagdollBone : public SGTGOCNodeRenderable, public SGTGOCEditorInterface
{
private:
	Ogre::Entity *mEntity;
	Ogre::Bone *mBone;
	Ogre::SceneNode *mMeshNode;
	Ogre::SceneNode *mOffsetNode;
	Ogre::SceneNode *mJointAxisNode;
	Ogre::ManualObject *mJointAxis;
	sBoneActorBindConfig mBoneConfig;
	bool mDebugAnimation;
	Ogre::Quaternion mBoneActorGlobalBindOrientationInverse;
	Ogre::Quaternion mBoneGlobalBindOrientation;
	Ogre::Vector3 mGlobalBindPosition;
	RagBoneRef* mRagBoneRef;

	void ScaleNode();

public:
	SGTGOCRagdollBone(void);
	~SGTGOCRagdollBone(void);

	goc_id_family& GetFamilyID() const { static std::string name = "RagdollBone"; return name; }
	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "RagdollBone"; return name; }

	void CreateFromDataMap(SGTDataMap *parameters);
	void GetParameters(SGTDataMap *parameters);
	static void GetDefaultParameters(SGTDataMap *parameters);
	bool IsViewComponent() { return false; }
	void* GetUserData();
	void InjectUserData(void* data);

	void CreateJointAxis();

	bool GetTestAnimation();

	sBoneActorBindConfig GetBoneConfig() { return mBoneConfig; }

	void UpdatePosition(Ogre::Vector3 position);
	void UpdateScale(Ogre::Vector3 scale);
	void UpdateOrientation(Ogre::Quaternion orientation);

	void SetBone(Ogre::SceneNode *meshnode, SGTRagdoll* ragdoll, sBoneActorBind &bone_config, bool controlBone);

	void SetOwner(SGTGameObject *go);

	void Save(SGTSaveSystem& mgr) {};
	void Load(SGTLoadSystem& mgr) {};
	std::string& TellName() { static std::string name = "RagdollBone"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "RagdollBone"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTSaveable* NewInstance() { return new SGTGOCRagdollBone; };
	static SGTGOCEditorInterface* NewEditorInterfaceInstance() { return new SGTGOCRagdollBone(); }
};

class SGTDllExport SGTRagdoll : public SGTGOCEditorInterface, public SGTGOCNodeRenderable, public SGTMessageListener
{
private:
	Ogre::SceneManager *mOgreSceneMgr;
	NxOgre::Scene *mNxOgreScene;
	std::vector<sD6Joint> mJoints;
	std::vector<sBoneActorBind> mSkeleton;
	Ogre::Entity *mEntity;
	Ogre::AnimationState *mAnimationState;
	bool mControlledByActors;
	Ogre::String mAnimationStateStr;

	bool mSetControlToActorsTemp;

	void Create(Ogre::String meshname, Ogre::Vector3 scale);

	/*
	CreateBoneActors and CreateJoints create a ragdoll skeleton from scratch.
	They are only used the first time the ragdoll is created, after that a config file will be exported.
	*/
	std::vector<sBoneActorBindConfig> CreateDummySkeleton();
	void CreateJoints();

	/*
	Creates a skeleton from a config file.
	*/
	void CreateSkeleton(std::vector<sBoneActorBindConfig> &config);

	Ogre::Bone* GetRealParent(Ogre::Bone *bone);

	void UpdateBoneActors();
	void UpdateVisualBones();
	void SetAllBonesToManualControl(bool manual);
	void ResetBones();

	void Serialise(std::vector<sBoneActorBindConfig> config);

public:
	SGTRagdoll();
	SGTRagdoll(Ogre::String meshname, Ogre::Vector3 scale = Ogre::Vector3(1,1,1));
	~SGTRagdoll(void);

	std::list<SGTGameObject*> mBoneObjects;

	void CreateBoneObjects();
	void SerialiseBoneObjects();

	void SetControlToActors();
	void SetControlToBones();

	goc_id_family& GetFamilyID() const { static std::string name = "GOCRagdoll"; return name; }
	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "Ragdoll"; return name; }

	void SetAnimationState(Ogre::String statename);

	void Update(float _time);

	void ReceiveMessage(SGTMsg &msg);

	void SetOwner(SGTGameObject *go);
	void CreateFromDataMap(SGTDataMap *parameters);
	void GetParameters(SGTDataMap *parameters);
	static void GetDefaultParameters(SGTDataMap *parameters);
	bool IsViewComponent() { return false; }
	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	virtual std::string& TellName() { static std::string name = "Ragdoll"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "Ragdoll"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTSaveable* NewInstance() { return new SGTRagdoll; };
	static SGTGOCEditorInterface* NewEditorInterfaceInstance() { return new SGTRagdoll(); }
};