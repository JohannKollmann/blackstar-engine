
#pragma once

#include "Ogre.h"
#include "OgrePhysX.h"
#include "NxD6Joint.h"
#include "NxD6JointDesc.h"
#include "OgrePhysXRenderableBinding.h"
#include "OgrePhysXActor.h"

namespace OgrePhysX
{
	struct OgrePhysXClass sD6Joint
	{
		NxD6Joint *mJoint;
		NxD6JointDesc mDescription;
	};

	struct OgrePhysXClass sJointLimitParams
	{
		float mValue;
		float mDamping;
		float mRestitution;
		float mSpring;
	};

	struct OgrePhysXClass sBoneActorBindConfig
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

	struct OgrePhysXClass sBoneActorBind
	{
		Ogre::Bone *mBone;
		Actor *mActor;
		sBoneActorBind *mParent;
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

	class OgrePhysXClass Ragdoll : public RenderableBinding
	{
		friend class Scene;
	private:
		NxScene *mNxScene;
		std::vector<sD6Joint> mJoints;
		Ogre::Entity *mEntity;
		Ogre::SceneNode *mNode;
		bool mControlledByActors;
		std::vector<sBoneActorBind> mSkeleton;

		Ragdoll(NxScene* scene, Ogre::Entity *ent, Ogre::SceneNode *node, NxCollisionGroup boneCollisionGroup = 0);
		~Ragdoll(void);

		/*
		CreateBoneActors and CreateJoints create a ragdoll skeleton from scratch.
		They are only used the first time the ragdoll is created, after that a config file will be exported.
		*/
		std::vector<sBoneActorBindConfig> generateAutoSkeleton();
		void createJoints();

		/*
		Creates a skeleton from a config file.
		*/
		void createSkeleton(std::vector<sBoneActorBindConfig> &config, NxCollisionGroup boneCollisionGroup);

		void updateBoneActors();
		void updateVisualBones();
		void setAllBonesToManualControl(bool manual);


	public:

		void setControlToActors();
		void setControlToBones();
		bool isControlledByActors() { return mControlledByActors; }
		void resetBones();

		void setActorUserData(void *data);

		std::vector<sBoneActorBind>& getSkeleton();
		void serialise(std::vector<sBoneActorBindConfig> config, Ogre::String filename);

		Ogre::Entity* getEntity() { return mEntity; }

		void sync();
	};

}