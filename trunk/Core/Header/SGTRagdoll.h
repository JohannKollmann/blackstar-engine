
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


struct sBoneActorBind
{
	Ogre::Bone *mBone;
	NxOgre::Actor *mActor;
	sBoneActorBind *mParent;
	Ogre::String mParentBoneName;
	sD6Joint mJoint;
	Ogre::Vector3 mLocalAxis;
	Ogre::Vector3 mOffset;
	float mBoneLength;
	float mBoneRadius;
	float mSwing1;
	float mSwing2;
	float mTwistMax;
	float mTwistMin;
	Ogre::Quaternion mBoneActorGlobalBindOrientation;
	Ogre::Quaternion mBoneGlobalBindOrientation;
	Ogre::Quaternion mBoneActorGlobalBindOrientationInverse;
};

struct sBoneActorBindConfig
{
	Ogre::String mBoneName;
	Ogre::String mBoneParentName;
	Ogre::Vector3 mBoneOffset;
	Ogre::Quaternion mBoneOrientation;
	float mBoneLength;
	float mBoneRadius;
	float mSwing1;
	float mSwing2;
	float mTwistMax;
	float mTwistMin;
};

class SGTDllExport SGTGOCRagdollBone : public SGTGOCNodeRenderable, public SGTGOCEditorInterface
{
private:
	Ogre::Entity *mEntity;
	Ogre::Bone *mBone;
	Ogre::SceneNode *mMeshNode;
	sBoneActorBindConfig mBoneConfig;
	bool mDebugAnimation;

public:
	SGTGOCRagdollBone(void);
	~SGTGOCRagdollBone(void);

	goc_id_family& GetFamilyID() const { static std::string name = "RagdollBone"; return name; }
	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "RagdollBone"; return name; }

	void CreateFromDataMap(SGTDataMap *parameters);
	void GetParameters(SGTDataMap *parameters);
	static void GetDefaultParameters(SGTDataMap *parameters);

	void UpdatePosition(Ogre::Vector3 position);
	void UpdateOrientation(Ogre::Quaternion orientation);

	void SetBone(Ogre::Bone* bone, Ogre::SceneNode *meshnode, float length, float radius);

	void Save(SGTSaveSystem& mgr) {};
	void Load(SGTLoadSystem& mgr) {};
	std::string& TellName() { static std::string name = "RagdollBone"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "RagdollBone"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTGOCRagdollBone* NewInstance() { return new SGTGOCRagdollBone; };
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
	void CreateSkeleton(std::vector<sBoneActorBindConfig> config);

	Ogre::Bone* GetRealParent(Ogre::Bone *bone);

	void UpdateBoneActors();
	void UpdateVisualBones();
	void SetAllBonesToManualControl(bool manual);
	void ResetBones();

public:
	SGTRagdoll();
	SGTRagdoll(Ogre::String meshname, Ogre::Vector3 scale = Ogre::Vector3(1,1,1));
	~SGTRagdoll(void);

	void CreateBoneObjects();

	void SetControlToActors();
	void SetControlToBones();

	goc_id_family& GetFamilyID() const { static std::string name = "GOCRagdoll"; return name; }
	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "Ragdoll"; return name; }

	void SetAnimationState(Ogre::String statename);

	void Update(float _time);

	void ReceiveMessage(SGTMsg &msg);

	void CreateFromDataMap(SGTDataMap *parameters);
	void GetParameters(SGTDataMap *parameters);
	static void GetDefaultParameters(SGTDataMap *parameters);
	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	virtual std::string& TellName() { static std::string name = "Ragdoll"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "Ragdoll"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTRagdoll* NewInstance() { return new SGTRagdoll; };
};