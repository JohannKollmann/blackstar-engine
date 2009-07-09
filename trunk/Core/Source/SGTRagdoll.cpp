
#include "SGTRagdoll.h"
#include "SGTMain.h"
#include <fstream>
#include "SGTSceneManager.h"

SGTRagdoll::~SGTRagdoll(void)
{
	for (std::vector<sBoneActorBind>::iterator i = mSkeleton.begin(); i != mSkeleton.end(); i++)
	{
		if ((*i).mParent) SGTMain::Instance().GetNxScene()->getNxScene()->releaseJoint(*(*i).mJoint.mJoint);
		SGTMain::Instance().GetNxScene()->destroyActor((*i).mActor->getName());
	}
	mOgreSceneMgr->destroyEntity(mEntity);
}

SGTRagdoll::SGTRagdoll(Ogre::SceneManager *ogre_scenemanager, NxOgre::Scene *nx_scene, Ogre::String meshname, Ogre::SceneNode *node)
{
	mOgreSceneMgr = ogre_scenemanager;
	mNxOgreScene = nx_scene;
	mNode = node;
	Ogre::Vector3 scale = mNode->getScale();
	mEntity = mOgreSceneMgr->createEntity(mNode->getName() + "_mesh", meshname);
	mNode->attachObject(mEntity);
	ResetBones();
	if (Ogre::ResourceGroupManager::getSingleton().resourceExists("General", mEntity->getMesh()->getName() + ".bones"))
	{
		std::vector<sBoneActorBindConfig> bones;
		Ogre::ConfigFile cf;
		cf.loadFromResourceSystem(mEntity->getMesh()->getName() + ".bones", "General");
		Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
		Ogre::String secName;
		while (seci.hasMoreElements())
		{
			secName = seci.peekNextKey();
			Ogre::ConfigFile::SettingsMultiMap *Settings = seci.getNext();
			if (secName.find("Bone") != Ogre::String::npos)
			{
				Ogre::String bonename, boneparentname, shaperadius, shapelength, shapeoffset, shapeorientation, jointorientation, hinge, swing1_value,swing1_damping,swing1_restitution,swing1_spring, swing2_value,swing2_damping,swing2_restitution,swing2_spring, twistmax_value,twistmax_damping,twistmax_restitution,twistmax_spring, twistmin_value,twistmin_damping,twistmin_restitution,twistmin_spring;
				Ogre::ConfigFile::SettingsMultiMap::iterator i;
				for (i = Settings->begin(); i != Settings->end(); ++i)
				{
					if (i->first == "BoneName") bonename = i->second;
					if (i->first == "ParentName") boneparentname = i->second;
					if (i->first == "BoneLength") shapelength = i->second;
					if (i->first == "BoneOffset") shapeoffset = i->second;
					if (i->first == "BoneOrientation") shapeorientation = i->second;
					if (i->first == "Radius") shaperadius = i->second;
					if (i->first == "JointOrientation") jointorientation = i->second;
					if (i->first == "Hinge") hinge = i->second;

					if (i->first == "Swing1_Value") swing1_value = i->second;
					if (i->first == "Swing1_Damping") swing1_damping = i->second;
					if (i->first == "Swing1_Restitution") swing1_restitution = i->second;
					if (i->first == "Swing1_Spring") swing1_spring = i->second;

					if (i->first == "Swing2_Value") swing2_value = i->second;
					if (i->first == "Swing2_Damping") swing2_damping = i->second;
					if (i->first == "Swing2_Restitution") swing2_restitution = i->second;
					if (i->first == "Swing2_Spring") swing2_spring = i->second;

					if (i->first == "TwistMax_Value") twistmax_value = i->second;
					if (i->first == "TwistMax_Damping") twistmax_damping = i->second;
					if (i->first == "TwistMax_Restitution") twistmax_restitution = i->second;
					if (i->first == "TwistMax_Spring") twistmax_spring = i->second;

					if (i->first == "TwistMin_Value") twistmin_value = i->second;
					if (i->first == "TwistMin_Damping") twistmin_damping = i->second;
					if (i->first == "TwistMin_Restitution") twistmin_restitution = i->second;
					if (i->first == "TwistMin_Spring") twistmin_spring = i->second;
				}
				if (bonename == "" || boneparentname == "")
				{
					Ogre::LogManager::getSingleton().logMessage("Ragdoll::Ragdoll: Error in Section " + secName + "! -> continue");
					continue;
				}
				sBoneActorBindConfig boneconfig;
				if (jointorientation == "")
				{
					boneconfig.mJointOrientation = Ogre::Quaternion();
					boneconfig.mNeedsJointOrientation = true;
				}
				else
				{
					boneconfig.mJointOrientation = Ogre::StringConverter::parseQuaternion(jointorientation);
					boneconfig.mNeedsJointOrientation = false;
				}
				boneconfig.mBoneName = bonename;
				boneconfig.mParentName = boneparentname;
				boneconfig.mBoneLength = Ogre::StringConverter::parseReal(shapelength);
				boneconfig.mBoneOffset = Ogre::StringConverter::parseVector3(shapeoffset);
				boneconfig.mBoneOrientation = Ogre::StringConverter::parseQuaternion(shapeorientation);
				boneconfig.mRadius = Ogre::StringConverter::parseReal(shaperadius);
				boneconfig.mHinge = Ogre::StringConverter::parseBool(hinge);
				boneconfig.mSwing1.mValue = Ogre::StringConverter::parseReal(swing1_value);
				boneconfig.mSwing1.mDamping = Ogre::StringConverter::parseReal(swing1_damping);
				boneconfig.mSwing1.mRestitution = Ogre::StringConverter::parseReal(swing1_restitution);
				boneconfig.mSwing1.mSpring = Ogre::StringConverter::parseReal(swing1_spring);

				boneconfig.mSwing2.mValue = Ogre::StringConverter::parseReal(swing2_value);
				boneconfig.mSwing2.mDamping = Ogre::StringConverter::parseReal(swing2_damping);
				boneconfig.mSwing2.mRestitution = Ogre::StringConverter::parseReal(swing2_restitution);
				boneconfig.mSwing2.mSpring = Ogre::StringConverter::parseReal(swing2_spring);

				boneconfig.mTwistMax.mValue = Ogre::StringConverter::parseReal(twistmax_value);
				boneconfig.mTwistMax.mDamping = Ogre::StringConverter::parseReal(twistmax_damping);
				boneconfig.mTwistMax.mRestitution = Ogre::StringConverter::parseReal(twistmax_restitution);
				boneconfig.mTwistMax.mSpring = Ogre::StringConverter::parseReal(twistmax_spring);

				boneconfig.mTwistMin.mValue = Ogre::StringConverter::parseReal(twistmin_value);
				boneconfig.mTwistMin.mDamping = Ogre::StringConverter::parseReal(twistmin_damping);
				boneconfig.mTwistMin.mRestitution = Ogre::StringConverter::parseReal(twistmin_restitution);
				boneconfig.mTwistMin.mSpring = Ogre::StringConverter::parseReal(twistmin_spring);

				bones.push_back(boneconfig);
			}
		}	
		CreateSkeleton(bones);
	}
	else
	{
		Ogre::LogManager::getSingleton().logMessage("Could not find " + mEntity->getMesh()->getName() + ".bones, generating new...");
		std::vector<sBoneActorBindConfig> boneconfig = GenerateAutoSkeleton();
		CreateSkeleton(boneconfig);
	}
	mAnimationState = 0;
	SetControlToBones();
}

void SGTRagdoll::Serialise(std::vector<sBoneActorBindConfig> boneconfig)
{
	std::fstream f;
	f.open((mEntity->getMesh()->getName() + ".bones").c_str(), std::ios::out | std::ios::trunc);
	for (std::vector<sBoneActorBindConfig>::iterator i = boneconfig.begin(); i != boneconfig.end(); i++)
	{
		f << "[Bone_" << (*i).mBoneName.c_str() << "]" << std::endl;
		f << "BoneName = " << (*i).mBoneName.c_str() << std::endl;
		if ((*i).mParentName != "")
		{
			f << "ParentName = " << (*i).mParentName.c_str() << std::endl;
		}
		else
		{
			f << "ParentName = None" << std::endl;
		}
		f << "BoneLength = " << (*i).mBoneLength << std::endl;
		f << "BoneOffset = " << Ogre::StringConverter::toString((*i).mBoneOffset) << std::endl;
		f << "BoneOrientation = " << Ogre::StringConverter::toString((*i).mBoneOrientation) << std::endl;
		f << "Radius = " + Ogre::StringConverter::toString((*i).mRadius) << std::endl;
		f << "JointOrientation = " << Ogre::StringConverter::toString((*i).mJointOrientation) << std::endl;
		f << "Hinge = False" << std::endl;

		f << "Swing1_Value = " << (*i).mSwing1.mValue << std::endl;
		f << "Swing1_Damping = " << (*i).mSwing1.mDamping << std::endl;
		f << "Swing1_Restitution = " << (*i).mSwing1.mRestitution << std::endl;
		f << "Swing1_Spring = " << (*i).mSwing1.mSpring << std::endl;

		f << "Swing2_Value = " << (*i).mSwing2.mValue << std::endl;
		f << "Swing2_Damping = " << (*i).mSwing2.mDamping << std::endl;
		f << "Swing2_Restitution = " << (*i).mSwing2.mRestitution << std::endl;
		f << "Swing2_Spring = " << (*i).mSwing2.mSpring << std::endl;

		f << "TwistMax_Value = " << (*i).mTwistMax.mValue << std::endl;
		f << "TwistMax_Damping = " << (*i).mTwistMax.mDamping << std::endl;
		f << "TwistMax_Restitution = " << (*i).mTwistMax.mRestitution << std::endl;
		f << "TwistMax_Spring = " << (*i).mTwistMax.mSpring << std::endl;

		f << "TwistMin_Value = " << (*i).mTwistMin.mValue << std::endl;
		f << "TwistMin_Damping = " << (*i).mTwistMin.mDamping << std::endl;
		f << "TwistMin_Restitution = " << (*i).mTwistMin.mRestitution << std::endl;
		f << "TwistMin_Spring = " << (*i).mTwistMin.mSpring << std::endl;

		f << std::endl;
	}
	f.close();
}

Ogre::Bone* SGTRagdoll::GetRealParent(Ogre::Bone *bone)
{
	Ogre::Bone *node = bone;
	bool found = false;
	while (node->getParent())
	{
		node = (Ogre::Bone*)node->getParent();
		if (node->_getDerivedPosition() != bone->_getDerivedPosition()) return node;
	}
	return node;
}

void SGTRagdoll::CreateSkeleton(std::vector<sBoneActorBindConfig> &config)
{
	Ogre::Vector3 scale = mEntity->getParentSceneNode()->_getDerivedScale();
	for (std::vector<sBoneActorBindConfig>::iterator i = config.begin(); i != config.end(); i++)
	{
		Ogre::Bone *bone = mEntity->getSkeleton()->getBone((*i).mBoneName);
		Ogre::Vector3 mypos = bone->_getDerivedPosition();
		Ogre::Vector3 parentpos = mEntity->getParentSceneNode()->_getDerivedPosition();
		if ((*i).mParentName != "None") parentpos = mEntity->getSkeleton()->getBone((*i).mParentName)->_getDerivedPosition();
		if ((*i).mBoneLength == 0.0f)
		{
			Ogre::LogManager::getSingleton().logMessage("Error in Ragdoll::CreateSkeleton: Bone length for " + (*i).mBoneName + " is 0.");
			continue;
		}
		sBoneActorBind bone_actor_bind;
		bone_actor_bind.mBone = bone;
		NxOgre::ActorParams ap;
		ap.setToDefault();
		ap.mMass = 10.0f;
		ap.mDensity = 0.0f;
		NxOgre::ShapeParams sp;
		sp.setToDefault();
		sp.mMass = 10.0f;
		sp.mDensity = 0.0f;
		//(*i).mBoneLength = 0.5f;
		//(*i).mRadius = 0.5f;
		//sp.mGroup = "BoneActor";
		float scale_factor = ((scale.x + scale.y + scale.z) / 3);
		float capsule_radius = (*i).mRadius * scale_factor;
		if (capsule_radius <= 0.05f) capsule_radius = 0.05f;
		float scaled_bonelength = (*i).mBoneLength * scale_factor;
		float capsule_height = scaled_bonelength - capsule_radius * 2.0f;
		if (capsule_height <= 0.0f) capsule_height = 0.01f;
		//NxFindRotationMatrix(NxVec3(0.0f,1.0f,0.0f), NxVec3(1.0,0.0f,0.0f), sp.mLocalPose.M);
		sp.mLocalPose.t = NxVec3(0.0f,scaled_bonelength/2.0f,0.0f);
		bone_actor_bind.mOffset = (*i).mBoneOffset;
		Ogre::Vector3 localBonePos = (bone->_getDerivedPosition() + (*i).mBoneOffset) * scale;
		Ogre::String strID = bone->getName() + "_" + SGTSceneManager::Instance().RequestIDStr();
		if ((*i).mParentName != "None")
		{
			bone_actor_bind.mActor = mNxOgreScene->createActor(strID, new NxOgre::Capsule(capsule_radius, capsule_height, sp), NxOgre::Pose(localBonePos + mEntity->getParentSceneNode()->_getDerivedPosition(), (*i).mBoneOrientation), ap);//mEntity->getParentSceneNode()->_getDerivedOrientation() * bone->_getDerivedOrientation()), ap);
		}
		else
		{
			bone_actor_bind.mActor = mNxOgreScene->createActor(strID, new NxOgre::Capsule(capsule_radius, capsule_height, sp), NxOgre::Pose(localBonePos + mEntity->getParentSceneNode()->_getDerivedPosition(), (*i).mBoneOrientation), ap);
		}
		bone_actor_bind.mActor->setSolverIterationCount(12);
		bone_actor_bind.mParent = 0;
		bone_actor_bind.mParentBoneName = (*i).mParentName;
		bone_actor_bind.mNeedsJointOrientation = (*i).mNeedsJointOrientation;
		bone_actor_bind.mJointOrientation = (*i).mJointOrientation;
		bone_actor_bind.mSwing1 = (*i).mSwing1;
		bone_actor_bind.mSwing2 = (*i).mSwing2;
		bone_actor_bind.mTwistMin = (*i).mTwistMin;
		bone_actor_bind.mTwistMax = (*i).mTwistMax;

		bone_actor_bind.mBoneLength = (*i).mBoneLength;
		bone_actor_bind.mBoneRadius = (*i).mRadius;
		bone_actor_bind.mVisualBone = 0;
		//bone_actor_bind.mBoneGlobalBindPosition = bone_actor_bind.mActor->getGlobalPositionAsOgreVector3();
		bone_actor_bind.mBoneActorGlobalBindOrientation = bone_actor_bind.mActor->getGlobalOrientationAsOgreQuaternion();
		bone_actor_bind.mBoneGlobalBindOrientation = bone_actor_bind.mBone->_getDerivedOrientation();
		bone_actor_bind.mBoneActorGlobalBindOrientationInverse = bone_actor_bind.mActor->getGlobalOrientationAsOgreQuaternion().Inverse();
		mSkeleton.push_back(bone_actor_bind);
	}
	if (mSkeleton.size() != config.size())
	{
		Ogre::LogManager::getSingleton().logMessage("Error in Ragdoll::CreateSkeleton: Invalid bone configuration, canceling skeleton generation...");
		return;
	}
	for (std::vector<sBoneActorBind>::iterator i = mSkeleton.begin(); i != mSkeleton.end(); i++)
	{
		for (std::vector<sBoneActorBind>::iterator x = mSkeleton.begin(); x != mSkeleton.end(); x++)
		{
			if ((*i).mParentBoneName == (*x).mBone->getName())
			{
				(*i).mParent = &(*x);

				for (std::vector<sBoneActorBindConfig>::iterator j = config.begin(); j != config.end(); j++)
				{
					if ((*j).mNeedsJointOrientation && (*j).mBoneName == (*i).mBone->getName())
					{
						(*j).mJointOrientation = (*i).mParent->mActor->getGlobalOrientationAsOgreQuaternion().Inverse() * (*i).mActor->getGlobalOrientationAsOgreQuaternion();
						(*j).mNeedsJointOrientation = false;
						(*i).mJointOrientation = (*j).mJointOrientation;
					}
				}
			}
		}
	}
	CreateJoints();
}

std::vector<sBoneActorBindConfig> SGTRagdoll::GenerateAutoSkeleton()
{
	std::vector<sBoneActorBindConfig> returner;
    Ogre::SkeletonInstance* skeletonInst = mEntity->getSkeleton();
	Ogre::Skeleton::BoneIterator boneI=skeletonInst->getBoneIterator();
	Ogre::Vector3 scale = mEntity->getParentSceneNode()->_getDerivedScale();
	while(boneI.hasMoreElements())
	{
		Ogre::Bone* bone=boneI.getNext();
		Ogre::String bName=bone->getName();

		Ogre::Vector3 difference = Ogre::Vector3(0,1,0);
		Ogre::Vector3 mypos = bone->_getDerivedPosition();
		Ogre::Quaternion estimated_orientation = Ogre::Quaternion();//0.707107, 0, 0, 0.707107);
		if (bone->numChildren() == 1 && bone->getChild(0))
		{
			//points towards child
			Ogre::Vector3 childpos = bone->getChild(0)->_getDerivedPosition();
			if (childpos == mypos) continue;
			difference = childpos - mypos;
			estimated_orientation = Ogre::Vector3::UNIT_Y.getRotationTo(difference.normalisedCopy());
		}
		else if (bone->getParent())
		{
			//points away from parent
			difference = Ogre::Vector3(1,0,0);
			Ogre::Vector3 parentpos = bone->getParent()->_getDerivedPosition();
			if (parentpos == mypos) continue;
			difference = parentpos - mypos;
			estimated_orientation = Ogre::Vector3::UNIT_Y.getRotationTo(-difference.normalisedCopy());
			difference.normalise();
			difference = difference * 0.1f * (mEntity->getBoundingRadius() / ((scale.x + scale.y + scale.z) / 3.0f));
		}
		if (difference.length() > 0)
		{
				sBoneActorBindConfig bone_actor_bind;
				bone_actor_bind.mBoneName = bone->getName();
				if (!bone->getParent()) bone_actor_bind.mParentName = "None";
				else bone_actor_bind.mParentName = bone->getParent()->getName();//GetRealParent(bone)->getName();

				bone_actor_bind.mBoneOffset = Ogre::Vector3(0,0,0);
				bone_actor_bind.mJointOrientation = Ogre::Quaternion();
				bone_actor_bind.mNeedsJointOrientation = true;
				bone_actor_bind.mTwistMax.mValue = 20.0f;
				bone_actor_bind.mTwistMax.mDamping = 0.5f;
				bone_actor_bind.mTwistMax.mRestitution = 0.0f;
				bone_actor_bind.mTwistMax.mSpring = 0.0f;

				bone_actor_bind.mTwistMin.mValue = -20.0f;
				bone_actor_bind.mTwistMin.mDamping = 0.5f;
				bone_actor_bind.mTwistMin.mRestitution = 0.0f;
				bone_actor_bind.mTwistMin.mSpring = 0.0f;

				bone_actor_bind.mSwing1.mValue = 30.0f;
				bone_actor_bind.mSwing1.mDamping = 0.5f;
				bone_actor_bind.mSwing1.mRestitution = 0.0f;
				bone_actor_bind.mSwing1.mSpring = 0.0f;

				bone_actor_bind.mSwing2.mValue = 30.0f;
				bone_actor_bind.mSwing2.mDamping = 0.5f;
				bone_actor_bind.mSwing2.mRestitution = 0.0f;
				bone_actor_bind.mSwing2.mSpring = 0.0f;

				bone_actor_bind.mHinge = false;
				float boneLength = difference.length();
				bone_actor_bind.mBoneLength = boneLength * 0.98f;
				float bradius = mEntity->getBoundingRadius() / ((scale.x + scale.y + scale.z) / 3.0f);
				float minRadius = bradius * 0.02f;
				float maxRadius = bradius * 0.035f;
				float estimatedRadius = bone_actor_bind.mBoneLength * 0.25f;
				if (estimatedRadius < minRadius) bone_actor_bind.mRadius = minRadius;
				else if (estimatedRadius > maxRadius) bone_actor_bind.mRadius = maxRadius;
				else bone_actor_bind.mRadius = estimatedRadius;
				bone_actor_bind.mBoneOrientation = estimated_orientation;
				returner.push_back(bone_actor_bind);
			}
		}
	return returner;
}

void SGTRagdoll::CreateJoints()
{
	Ogre::Vector3 scale = mEntity->getParentSceneNode()->_getDerivedScale();
	int counter = 0;
	for (std::vector<sBoneActorBind>::iterator i = mSkeleton.begin(); i != mSkeleton.end(); i++)
	{
		if ((*i).mParent == 0) continue;
		mNxOgreScene->getNxScene()->setActorPairFlags(*(*i).mParent->mActor->getNxActor(), *(*i).mActor->getNxActor(), NX_IGNORE_PAIR);
		NxD6JointDesc d6Desc;
		d6Desc.actor[0] = (*i).mParent->mActor->getNxActor();
		d6Desc.actor[1] = (*i).mActor->getNxActor();
		Ogre::Vector3 position = (*i).mActor->getGlobalPositionAsOgreVector3() - (*i).mParent->mActor->getGlobalPositionAsOgreVector3();//(*i).mBone->getPosition();
		Ogre::Quaternion orientation = (*i).mJointOrientation;//(*i).mParent->mActor->getGlobalOrientationAsOgreQuaternion().Inverse() * (*i).mActor->getGlobalOrientationAsOgreQuaternion();//(*i).mBone->_getDerivedOrientation();
		position = position * scale;

		Ogre::LogManager::getSingleton().logMessage("Creating joint: " + (*i).mParent->mBone->getName() + " + " + (*i).mBone->getName());

		
		//NxMat34 basePose = NxOgre::Pose((*i).mParent->mActor->getGlobalPositionAsOgreVector3(), (*i).mParent->mActor->getGlobalOrientationAsOgreQuaternion());
		NxMat34 basePose = NxOgre::Pose((*i).mActor->getGlobalPositionAsOgreVector3(), (*i).mActor->getGlobalOrientationAsOgreQuaternion());
		NxMat34 localPose = NxOgre::Pose(position, orientation);
		d6Desc.localAxis[0] = localPose.M*NxVec3(1.0f,0.0f,0.0f);
		d6Desc.localAxis[1]=NxVec3(1.0f,0.0f,0.0f);
		d6Desc.localNormal[0]=localPose.M*NxVec3(0.0f,1.0f,0.0f);
		d6Desc.localNormal[1]=NxVec3(0.0f,1.0f,0.0f);
		d6Desc.setGlobalAnchor(basePose*NxVec3(0.0f,0.0f,0.0f));
		d6Desc.xMotion = NX_D6JOINT_MOTION_LOCKED;
		d6Desc.yMotion = NX_D6JOINT_MOTION_LOCKED;
		d6Desc.zMotion = NX_D6JOINT_MOTION_LOCKED;
		d6Desc.twistMotion = NX_D6JOINT_MOTION_FREE;
		d6Desc.swing1Motion = NX_D6JOINT_MOTION_FREE;
		d6Desc.swing2Motion = NX_D6JOINT_MOTION_FREE;
		/*if ((*i).mTwistMax.mValue != 0.0f || (*i).mTwistMax.mValue != 0.0f)
			{
				d6Desc.twistMotion = NX_D6JOINT_MOTION_LIMITED;
				d6Desc.twistLimit.low.value=NxMath::degToRad((*i).mTwistMin.mValue);
				d6Desc.twistLimit.low.damping=(*i).mTwistMin.mDamping;
				d6Desc.twistLimit.low.restitution=(*i).mTwistMin.mRestitution;
				d6Desc.twistLimit.low.spring=(*i).mTwistMin.mSpring;
				d6Desc.twistLimit.high.value=NxMath::degToRad((*i).mTwistMax.mValue);
				d6Desc.twistLimit.high.damping=(*i).mTwistMax.mDamping;
				d6Desc.twistLimit.high.restitution=(*i).mTwistMax.mRestitution;
				d6Desc.twistLimit.high.spring=(*i).mTwistMax.mSpring;
			}
			if ((*i).mSwing1.mValue > 0.0f)
			{
				d6Desc.swing1Motion = NX_D6JOINT_MOTION_LIMITED;
				d6Desc.swing1Limit.value=NxMath::degToRad((*i).mSwing1.mValue);
				d6Desc.swing1Limit.damping=(*i).mSwing1.mDamping;
				d6Desc.swing1Limit.restitution=(*i).mSwing1.mRestitution;
				d6Desc.swing1Limit.spring=(*i).mSwing1.mSpring;
			}
			if ((*i).mSwing2.mValue > 0.0f)
			{
				d6Desc.swing2Motion = NX_D6JOINT_MOTION_LIMITED;
				d6Desc.swing2Limit.value=NxMath::degToRad((*i).mSwing2.mValue);
				d6Desc.swing2Limit.damping=(*i).mSwing2.mDamping;
				d6Desc.swing2Limit.restitution=(*i).mSwing2.mRestitution;
				d6Desc.swing2Limit.spring=(*i).mSwing2.mSpring;
			}*/

		d6Desc.flags|=NX_D6JOINT_SLERP_DRIVE;
		d6Desc.slerpDrive.driveType=NX_D6JOINT_DRIVE_POSITION;// | NX_D6JOINT_DRIVE_VELOCITY;
		d6Desc.slerpDrive.forceLimit=0.0f; //Not used with the current drive model.
		d6Desc.slerpDrive.spring=250.0f;
		d6Desc.slerpDrive.damping=0.01f;

		NxD6Joint *d6Joint=(NxD6Joint*)mNxOgreScene->getNxScene()->createJoint(d6Desc);
		sD6Joint joint;
		joint.mJoint = d6Joint;
		joint.mDescription = d6Desc;
		(*i).mJoint = joint;
		counter++;
	}
	Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(counter) + " Joints wurden erstellt.");
}

void SGTRagdoll::UpdateBoneActors()
{
	Ogre::Vector3 scale = mEntity->getParentSceneNode()->_getDerivedScale();
	for (std::vector<sBoneActorBind>::iterator i = mSkeleton.begin(); i != mSkeleton.end(); i++)
	{
		Ogre::Vector3 position;
		//update Bone Actor
		if ((*i).mParent)
		{
			position = (mEntity->getParentSceneNode()->_getDerivedOrientation() * ((*i).mBone->_getDerivedPosition()+(*i).mOffset) * scale) + mEntity->getParentSceneNode()->_getDerivedPosition();
			(*i).mActor->setGlobalPosition(position);  
			Ogre::Quaternion drive_orientation = (*i).mBone->_getDerivedOrientation();
			drive_orientation = (*i).mParent->mBone->_getDerivedOrientation().Inverse() * drive_orientation;
			(*i).mJoint.mJoint->setDriveOrientation(NxOgre::NxConvert<NxQuat, Ogre::Quaternion>(drive_orientation));
		}
		else
		{
			position = (mEntity->getParentSceneNode()->_getDerivedOrientation() * ((*i).mBone->_getDerivedPosition() + (*i).mOffset) * scale) + mEntity->getParentSceneNode()->_getDerivedPosition();
			(*i).mActor->setGlobalPosition(position);  
			//(*i).mActor->setGlobalOrientation(NxOgre::NxConvert<NxQuat, Ogre::Quaternion>((*i).mBoneActorGlobalBindOrientation * (*i).mBone->_getDerivedOrientation() * mEntity->getParentSceneNode()->_getDerivedOrientation()));
		}
		/*if ((*i).mVisualBone)
		{
			(*i).mVisualBone->SetGlobalPosition(position);
			Ogre::Quaternion q = Ogre::Vector3(1,0,0).getRotationTo(Ogre::Vector3(0,1,0));
			(*i).mVisualBone->SetGlobalOrientation(((*i).mBoneActorGlobalBindOrientation * (*i).mBone->_getDerivedOrientation() * mEntity->getParentSceneNode()->_getDerivedOrientation()) * q);
		}*/
	} 
}

void SGTRagdoll::UpdateVisualBones()
{
	Ogre::Quaternion PhysxRotation, OgreGlobalQuat, NodeRotation = mEntity->getParentSceneNode()->_getDerivedOrientation();
	for (std::vector<sBoneActorBind>::iterator i = mSkeleton.begin(); i != mSkeleton.end(); i++)
	{
		Ogre::Bone *bone = (*i).mBone;
		PhysxRotation = (*i).mActor->getGlobalOrientationAsOgreQuaternion() * (*i).mBoneActorGlobalBindOrientationInverse;
		OgreGlobalQuat = PhysxRotation * (*i).mBoneGlobalBindOrientation;
		Ogre::Quaternion ParentInverse = NodeRotation.Inverse();
		/*if (bone->getParent())
		{
			ParentInverse = (NodeRotation * bone->getParent()->_getDerivedOrientation()).Inverse();
		}*/
		if ((*i).mParent == 0)
		{
			Ogre::Vector3 newPos = (*i).mActor->getGlobalPositionAsOgreVector3() - (mEntity->getParentSceneNode()->_getDerivedOrientation() * (bone->_getDerivedPosition() * mEntity->getParentSceneNode()->getScale()));
			mEntity->getParentSceneNode()->setPosition(newPos);
		}
		bone->setOrientation(ParentInverse * OgreGlobalQuat);
	}
}

void SGTRagdoll::SetAllBonesToManualControl(bool manual)
{
   Ogre::SkeletonInstance* skeletonInst = mEntity->getSkeleton();
   Ogre::Skeleton::BoneIterator boneI=skeletonInst->getBoneIterator();

   while(boneI.hasMoreElements())
   {
	   Ogre::Bone *bone = boneI.getNext();
	   bone->setManuallyControlled(manual); 
   }

}

void SGTRagdoll::ResetBones()
{
   Ogre::SkeletonInstance* skeletonInst = mEntity->getSkeleton();
   Ogre::Skeleton::BoneIterator boneI=skeletonInst->getBoneIterator();

   while(boneI.hasMoreElements())
	   boneI.getNext()->reset();
}

void SGTRagdoll::SetControlToActors()
{
	mControlledByActors = true;
	Ogre::AnimationStateSet* set = mEntity->getAllAnimationStates();
	Ogre::AnimationStateIterator it = set->getAnimationStateIterator();
	Ogre::AnimationState *anim = 0;   
	while(it.hasMoreElements())
	{
		anim = it.getNext();
		anim->setLoop(false);
        anim->setEnabled(false);
        anim->setWeight(0);
     }

	SetAllBonesToManualControl(true);
	for (std::vector<sBoneActorBind>::iterator i = mSkeleton.begin(); i != mSkeleton.end(); i++)
	{
		//(*i).mBoneActorGlobalBindOrientation = (*i).mActor->getGlobalOrientationAsOgreQuaternion();
		//(*i).mBoneGlobalBindOrientation = mEntity->getParentSceneNode()->_getDerivedOrientation() * (*i).mBone->_getDerivedOrientation();
		//(*i).mBoneActorGlobalBindOrientationInverse = (*i).mActor->getGlobalOrientationAsOgreQuaternion().Inverse();
		(*i).mBone->setInheritOrientation(false); //!!!

		(*i).mActor->clearBodyFlag(NxBodyFlag::NX_BF_FROZEN_POS);
		if ((*i).mParent)
		{
			(*i).mJoint.mDescription.slerpDrive.driveType=0;
			(*i).mJoint.mDescription.slerpDrive.forceLimit=0.0f;
			(*i).mJoint.mDescription.slerpDrive.spring=0.0f;
			(*i).mJoint.mDescription.slerpDrive.damping=0.0f;

			(*i).mJoint.mDescription.twistMotion = NX_D6JOINT_MOTION_LOCKED;
			(*i).mJoint.mDescription.swing1Motion = NX_D6JOINT_MOTION_LOCKED;
			(*i).mJoint.mDescription.swing2Motion = NX_D6JOINT_MOTION_LOCKED;

			if ((*i).mTwistMax.mValue != 0.0f || (*i).mTwistMax.mValue != 0.0f)
			{
				(*i).mJoint.mDescription.twistMotion = NX_D6JOINT_MOTION_LIMITED;
				(*i).mJoint.mDescription.twistLimit.low.value=NxMath::degToRad((*i).mTwistMin.mValue);
				(*i).mJoint.mDescription.twistLimit.low.damping=(*i).mTwistMin.mDamping;
				(*i).mJoint.mDescription.twistLimit.low.restitution=(*i).mTwistMin.mRestitution;
				(*i).mJoint.mDescription.twistLimit.low.spring=(*i).mTwistMin.mSpring;
				(*i).mJoint.mDescription.twistLimit.high.value=NxMath::degToRad((*i).mTwistMax.mValue);
				(*i).mJoint.mDescription.twistLimit.high.damping=(*i).mTwistMax.mDamping;
				(*i).mJoint.mDescription.twistLimit.high.restitution=(*i).mTwistMax.mRestitution;
				(*i).mJoint.mDescription.twistLimit.high.spring=(*i).mTwistMax.mSpring;
			}
			if ((*i).mSwing1.mValue > 0.0f)
			{
				(*i).mJoint.mDescription.swing1Motion = NX_D6JOINT_MOTION_LIMITED;
				(*i).mJoint.mDescription.swing1Limit.value=NxMath::degToRad((*i).mSwing1.mValue);
				(*i).mJoint.mDescription.swing1Limit.damping=(*i).mSwing1.mDamping;
				(*i).mJoint.mDescription.swing1Limit.restitution=(*i).mSwing1.mRestitution;
				(*i).mJoint.mDescription.swing1Limit.spring=(*i).mSwing1.mSpring;
			}
			if ((*i).mSwing2.mValue > 0.0f)
			{
				(*i).mJoint.mDescription.swing2Motion = NX_D6JOINT_MOTION_LIMITED;
				(*i).mJoint.mDescription.swing2Limit.value=NxMath::degToRad((*i).mSwing2.mValue);
				(*i).mJoint.mDescription.swing2Limit.damping=(*i).mSwing2.mDamping;
				(*i).mJoint.mDescription.swing2Limit.restitution=(*i).mSwing2.mRestitution;
				(*i).mJoint.mDescription.swing2Limit.spring=(*i).mSwing2.mSpring;
			}


			(*i).mJoint.mJoint->loadFromDesc((*i).mJoint.mDescription);
		}
	}
}

void SGTRagdoll::SetControlToBones()
{
	mControlledByActors = false;
	SetAllBonesToManualControl(false);
	for (std::vector<sBoneActorBind>::iterator i = mSkeleton.begin(); i != mSkeleton.end(); i++)
	{
		//(*i).mActor->raiseActorFlag(NxActorFlag::NX_AF_DISABLE_RESPONSE);
		(*i).mActor->raiseBodyFlag(NxBodyFlag::NX_BF_FROZEN_POS);
		if ((*i).mParent)
		{
			(*i).mJoint.mDescription.slerpDrive.driveType=NX_D6JOINT_DRIVE_POSITION;
			(*i).mJoint.mDescription.slerpDrive.forceLimit=0.0f;
			(*i).mJoint.mDescription.slerpDrive.spring=250.0f;
			(*i).mJoint.mDescription.slerpDrive.damping=0.0f;
			(*i).mJoint.mJoint->loadFromDesc((*i).mJoint.mDescription);
		}
	}
}

void SGTRagdoll::SetAnimationState(Ogre::String statename)
{
	mAnimationState = mEntity->getAnimationState(statename);
	mAnimationState->setLoop(true);
	mAnimationState->setEnabled(true);
}

void SGTRagdoll::Update(float _time)
{
	if (mControlledByActors)
	{
		UpdateVisualBones();
	}
	else
	{
		if (mAnimationState)
		{
			mAnimationState->addTime(_time);
		}
		UpdateBoneActors();
	}
}