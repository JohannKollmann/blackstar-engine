
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

class SGTGOCAnimatedCharacterBone;

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

class SGTDllExport SGTRagdoll
{
private:
	Ogre::SceneManager *mOgreSceneMgr;
	NxOgre::Scene *mNxOgreScene;
	std::vector<sD6Joint> mJoints;
	Ogre::Entity *mEntity;
	Ogre::SceneNode *mNode;
	Ogre::AnimationState *mAnimationState;
	bool mControlledByActors;

	/*
	CreateBoneActors and CreateJoints create a ragdoll skeleton from scratch.
	They are only used the first time the ragdoll is created, after that a config file will be exported.
	*/
	std::vector<sBoneActorBindConfig> GenerateAutoSkeleton();
	void CreateJoints();

	/*
	Creates a skeleton from a config file.
	*/
	void CreateSkeleton(std::vector<sBoneActorBindConfig> &config);

	Ogre::Bone* GetRealParent(Ogre::Bone *bone);

	void UpdateBoneActors();
	void UpdateVisualBones();
	void SetAllBonesToManualControl(bool manual);


public:
	SGTRagdoll(Ogre::SceneManager *ogre_scenemanager, NxOgre::Scene *nx_scene, Ogre::String meshname, Ogre::SceneNode *node);
	~SGTRagdoll(void);

	std::vector<sBoneActorBind> mSkeleton;

	void SetControlToActors();
	void SetControlToBones();
	bool ControlledByActors() { return mControlledByActors; }
	void ResetBones();

	void Serialise(std::vector<sBoneActorBindConfig> config);

	Ogre::Entity* GetEntity() { return mEntity; }

	void SetAnimationState(Ogre::String statename);

	void Update(float _time);
};