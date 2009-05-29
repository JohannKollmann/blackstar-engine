
#include "SGTRagdoll.h"
#include "SGTMain.h"
#include <fstream>
#include "SGTSceneManager.h"

SGTGOCRagdollBone::SGTGOCRagdollBone(void)
{
	mNode = SGTMain::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode();
	mEntity = SGTMain::Instance().GetOgreSceneMgr()->createEntity(Ogre::StringConverter::toString(SGTSceneManager::Instance().RequestID()), "capsule.mesh");
	mEntity->setCastShadows(false);
	mEntity->setUserObject(mOwnerGO);
	mNode->attachObject(mEntity);
	mDebugAnimation = false;
	mMeshNode = 0;
	mBone = 0;
}
SGTGOCRagdollBone::~SGTGOCRagdollBone(void)
{
	SGTMain::Instance().GetOgreSceneMgr()->destroyEntity(mEntity);
	SGTMain::Instance().GetOgreSceneMgr()->destroySceneNode(mNode);
	if (mBone) mBone->setManuallyControlled(false);
}

void SGTGOCRagdollBone::CreateFromDataMap(SGTDataMap *parameters)
{
	mBoneConfig.mBoneName = parameters->GetOgreString("BoneName");
	mBoneConfig.mParentName = parameters->GetOgreString("mParentName");
	mBoneConfig.mBoneLength = parameters->GetFloat("BoneLength");
	mBoneConfig.mRadius = parameters->GetFloat("Radius");
	mBoneConfig.mSwing1 = parameters->GetFloat("Swing1");
	mBoneConfig.mSwing2 = parameters->GetFloat("Swing2");
	mBoneConfig.mTwistMax = parameters->GetFloat("TwistMax");
	mBoneConfig.mTwistMin = parameters->GetFloat("TwistMin");
}
void SGTGOCRagdollBone::GetParameters(SGTDataMap *parameters)
{
	parameters->AddOgreString("BoneName", mBoneConfig.mBoneName);
	parameters->AddOgreString("mParentName", mBoneConfig.mParentName);
	parameters->AddFloat("BoneLength", mBoneConfig.mBoneLength);
	parameters->AddFloat("Radius", mBoneConfig.mRadius);
	parameters->AddFloat("Swing1", mBoneConfig.mSwing1);
	parameters->AddFloat("Swing2", mBoneConfig.mSwing2);
	parameters->AddFloat("TwistMax", mBoneConfig.mTwistMax);
	parameters->AddFloat("TwistMin", mBoneConfig.mTwistMin);
}
void SGTGOCRagdollBone::GetDefaultParameters(SGTDataMap *parameters)
{
}
void SGTGOCRagdollBone::UpdatePosition(Ogre::Vector3 position)
{
	if (mNode && mBone)
	{
		mNode->setPosition(position);
		if (mDebugAnimation)
		{
			Ogre::Vector3 parentpos = mMeshNode->_getDerivedPosition();
			if (mBone->getParent()) parentpos = parentpos + mBone->getParent()->_getDerivedPosition();
			mBone->setPosition(position - parentpos);
		}
	}
}

void SGTGOCRagdollBone::UpdateOrientation(Ogre::Quaternion orientation)
{
	if (mNode && mBone)
	{
		mNode->setOrientation(orientation);
		if (mDebugAnimation)
		{
			Ogre::Quaternion parentquat = mMeshNode->_getDerivedOrientation();
			if (mBone->getParent()) parentquat = parentquat * mBone->getParent()->_getDerivedOrientation();
			mBone->setOrientation(parentquat.Inverse() * orientation);
		}
	}
}
void SGTGOCRagdollBone::SetBone(Ogre::Bone* bone, Ogre::SceneNode *meshnode, float length, float radius)
{
	mMeshNode = meshnode;
	mBone = bone;
	mBoneConfig.mBoneName = bone->getName();
	if (mBone->getParent()) mBoneConfig.mParentName = mBone->getParent()->getName();
	mBoneConfig.mRadius = radius;
	mBoneConfig.mBoneLength = length;
	mBoneConfig.mTwistMax = 20.0f;
	mBoneConfig.mTwistMin = -20.0f;
	mBoneConfig.mSwing1 = 30.0f;
	mBoneConfig.mSwing2 = 30.0f;
}

SGTRagdoll::~SGTRagdoll(void)
{
	SGTMessageSystem::Instance().QuitNewsgroup(this, "UPDATE_PER_FRAME");
	mOgreSceneMgr->destroySceneNode(mNode);
	mOgreSceneMgr->destroyEntity(mEntity);
}

SGTRagdoll::SGTRagdoll()
{
	SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
	mOgreSceneMgr = SGTMain::Instance().GetOgreSceneMgr();
	mNxOgreScene = SGTMain::Instance().GetNxScene();
}

SGTRagdoll::SGTRagdoll(Ogre::String meshname, Ogre::Vector3 scale)
{
	SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
	mOgreSceneMgr = SGTMain::Instance().GetOgreSceneMgr();
	mNxOgreScene = SGTMain::Instance().GetNxScene();
	Create(meshname, scale);
}

void SGTRagdoll::Create(Ogre::String meshname, Ogre::Vector3 scale)
{
	mNode = mOgreSceneMgr->getRootSceneNode()->createChildSceneNode();
	mNode->scale(scale);
	mEntity = mOgreSceneMgr->createEntity(SGTSceneManager::Instance().RequestIDStr(), meshname);
	mNode->attachObject(mEntity);
	ResetBones();
	if (Ogre::ResourceGroupManager::getSingleton().resourceExists("General", mEntity->getMesh()->getName() + ".bones"))
	{
		Ogre::LogManager::getSingleton().logMessage("UIADSFHLSUDIDUHGLUSGHLSUDGHA<LÖIDBOOONE");
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
				Ogre::String bonename, boneparentname, internbonename, shaperadius, shapelength, shapeoffset, shapeorientation, hinge, swing1, swing2, twistmax, twistmin, ragdollcontrolbyparent;
				Ogre::ConfigFile::SettingsMultiMap::iterator i;
				for (i = Settings->begin(); i != Settings->end(); ++i)
				{
					if (i->first == "BoneName") bonename = i->second;
					if (i->first == "mParentName") boneparentname = i->second;
					if (i->first == "InternBoneName") internbonename = i->second;
					if (i->first == "BoneLength") shapelength = i->second;
					if (i->first == "BoneOffset") shapeoffset = i->second;
					if (i->first == "BoneOrientation") shapeorientation = i->second;
					if (i->first == "Radius") shaperadius = i->second;
					if (i->first == "Hinge") hinge = i->second;
					if (i->first == "Swing1") swing1 = i->second;
					if (i->first == "Swing2") swing2 = i->second;
					if (i->first == "TwistMax") twistmax = i->second;
					if (i->first == "TwistMin") twistmin = i->second;
					if (i->first == "RagdollControlByParent") ragdollcontrolbyparent = i->second;	//Fix for jaiqua
				}
				if (bonename == "" || boneparentname == "" || internbonename == "" || shaperadius == "" || shapeoffset == "" || hinge == "" || swing1 == "" || swing2 == "" || twistmax == "" || twistmin == "")
				{
					Ogre::LogManager::getSingleton().logMessage("Ragdoll::Ragdoll: Error in Section " + secName + "! -> continue");
					continue;
				}
				sBoneActorBindConfig boneconfig;
				boneconfig.mBoneName = bonename;
				boneconfig.mParentName = boneparentname;
				boneconfig.mInternBoneName = internbonename;
				boneconfig.mBoneLength = Ogre::StringConverter::parseReal(shapelength);
				boneconfig.mBoneOffsetLength = Ogre::StringConverter::parseReal(shapeoffset);
				boneconfig.mBoneOrientation = Ogre::StringConverter::parseQuaternion(shapeorientation);
				boneconfig.mRadius = Ogre::StringConverter::parseReal(shaperadius);
				boneconfig.mHinge = Ogre::StringConverter::parseBool(hinge);
				boneconfig.mSwing1 = Ogre::StringConverter::parseReal(swing1);
				boneconfig.mSwing2 = Ogre::StringConverter::parseReal(swing2);
				boneconfig.mTwistMax = Ogre::StringConverter::parseReal(twistmax);
				boneconfig.mTwistMin = Ogre::StringConverter::parseReal(twistmin);
				boneconfig.mRagdollByParent = false;
				if (ragdollcontrolbyparent != "") boneconfig.mRagdollByParent = Ogre::StringConverter::parseBool(ragdollcontrolbyparent);
				bones.push_back(boneconfig);
			}
		}	
		CreateSkeleton(bones);
	}
	else
	{
		Ogre::LogManager::getSingleton().logMessage("Could not find " + mEntity->getMesh()->getName() + ".bones, generating new...");
		std::vector<sBoneActorBindConfig> boneconfig = CreateDummySkeleton();
		std::fstream f;
		f.open((mEntity->getMesh()->getName() + ".bones").c_str(), std::ios::out | std::ios::trunc);
		for (std::vector<sBoneActorBindConfig>::iterator i = boneconfig.begin(); i != boneconfig.end(); i++)
		{
			f << "[Bone_" << (*i).mBoneName.c_str() << "]" << std::endl;
			f << "BoneName = " << (*i).mBoneName.c_str() << std::endl;
			f << "InternBoneName = " << (*i).mInternBoneName.c_str() << std::endl;
			if ((*i).mParentName != "")
			{
				f << "mParentName = " << (*i).mParentName.c_str() << std::endl;
			}
			else
			{
				f << "mParentName = None" << std::endl;
			}
			f << "BoneLength = " << (*i).mBoneLength << std::endl;
			f << "BoneOffset = " << (*i).mBoneOffsetLength << std::endl;
			f << "BoneOrientation = " << Ogre::StringConverter::toString((*i).mBoneOrientation) << std::endl;
			f << "Radius = " + Ogre::StringConverter::toString((*i).mRadius) << std::endl;
			//f << "LocalAxis = " << Ogre::StringConverter::toString((*i).mLocalAxis) << std::endl;
			f << "Hinge = False" << std::endl;
			f << "Swing1 = " << (*i).mSwing1 << std::endl;
			f << "Swing2 = " << (*i).mSwing2 << std::endl;
			f << "TwistMax = " << (*i).mTwistMax << std::endl;
			f << "TwistMin = " << (*i).mTwistMin << std::endl;
			f << std::endl;
		}
		f.close();
		CreateSkeleton(boneconfig);
	}
	mAnimationState = 0;
	SetControlToBones();
}

void SGTRagdoll::CreateBoneObjects()
{
	for (std::vector<sBoneActorBind>::iterator i = mSkeleton.begin(); i != mSkeleton.end(); i++)
	{
		SGTGameObject *go = new SGTGameObject();
		SGTGOCRagdollBone *bone = new SGTGOCRagdollBone();
		bone->SetBone((*i).mBone, mNode, (*i).mBoneLength, 1);
		go->AddComponent(bone);
		go->SetGlobalPosition((*i).mActor->getGlobalPositionAsOgreVector3());
		go->SetGlobalOrientation((*i).mActor->getGlobalOrientationAsOgreQuaternion());
	}
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

void SGTRagdoll::CreateSkeleton(std::vector<sBoneActorBindConfig> config)
{
	Ogre::Vector3 scale = mEntity->getParentSceneNode()->_getDerivedScale();
	for (std::vector<sBoneActorBindConfig>::iterator i = config.begin(); i != config.end(); i++)
	{
		Ogre::Bone *bone = mEntity->getSkeleton()->getBone((*i).mBoneName);
		Ogre::Bone *internbone = mEntity->getSkeleton()->getBone((*i).mInternBoneName);
		Ogre::Vector3 mypos = bone->_getDerivedPosition();
		Ogre::Vector3 parentpos = mEntity->getParentSceneNode()->_getDerivedPosition();
		if ((*i).mParentName != "None") parentpos = mEntity->getSkeleton()->getBone((*i).mParentName)->_getDerivedPosition();
		if ((*i).mBoneLength == 0.0f)
		{
			Ogre::LogManager::getSingleton().logMessage("Error in Ragdoll::CreateSkeleton: Bone length for " + (*i).mBoneName + " is 0.");
			continue;
		}
		sBoneActorBind bone_actor_bind;
		bone_actor_bind.mBone = internbone;
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
		if (capsule_height <= 0.0f) capsule_height = 0.1f;
		NxFindRotationMatrix(NxVec3(0.0f,1.0f,0.0f), NxVec3(1.0,0.0f,0.0f), sp.mLocalPose.M);
		sp.mLocalPose.t = NxVec3(scaled_bonelength/2.0f,0.0f,0.0f);
		Ogre::Quaternion q;
		q.FromAngleAxis(Ogre::Radian(Ogre::Degree(90)), Ogre::Vector3(0,0,1));
		//bone_actor_bind.mActor = mNxOgreScene->createBody<NxOgre::Body>(mEntity->getName() + "_Bone_" + bone->getName(), new NxOgre::Capsule(capsule_radius, capsule_height, sp), NxOgre::Pose(bone->_getDerivedPosition() + mEntity->getParentSceneNode()->_getDerivedPosition(), mEntity->getParentSceneNode()->_getDerivedOrientation() * bone->_getDerivedOrientation()), nrp, ap);
		Ogre::Vector3 offset = ((*i).mBoneOrientation * Ogre::Vector3(1,0,0)) * (*i).mBoneOffsetLength * (*i).mBoneLength;
		//(*i).mBoneOrientation = Ogre::Quaternion();
		if ((*i).mParentName != "None") bone_actor_bind.mActor = mNxOgreScene->createActor(mEntity->getName() + "_Bone_" + bone->getName(), new NxOgre::Capsule(capsule_radius, capsule_height, sp), NxOgre::Pose(((bone->_getDerivedPosition() + offset) * scale) + mEntity->getParentSceneNode()->_getDerivedPosition(), (*i).mBoneOrientation), ap);//mEntity->getParentSceneNode()->_getDerivedOrientation() * bone->_getDerivedOrientation()), ap);
		else bone_actor_bind.mActor = mNxOgreScene->createActor(mEntity->getName() + "_Bone_" + bone->getName(), new NxOgre::Capsule(capsule_radius, capsule_height, sp), NxOgre::Pose((bone->_getDerivedPosition() * scale) + mEntity->getParentSceneNode()->_getDerivedPosition(), (*i).mBoneOrientation), ap);
		bone_actor_bind.mActor->setSolverIterationCount(12);
		bone_actor_bind.mActor->setName(bone->getName().c_str());
		bone_actor_bind.mParent = 0;
		bone_actor_bind.mParentBoneName = (*i).mParentName;
		bone_actor_bind.mSwing1 = (*i).mSwing1;
		bone_actor_bind.mSwing2 = (*i).mSwing2;
		bone_actor_bind.mTwistMin = (*i).mTwistMin;
		bone_actor_bind.mTwistMax = (*i).mTwistMax;
		bone_actor_bind.mRagdollByParent = (*i).mRagdollByParent;
		//bone_actor_bind.mBoneGlobalBindPosition = bone_actor_bind.mActor->getGlobalPositionAsOgreVector3();
		bone_actor_bind.mOffset = offset;
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
			if ((*i).mParentBoneName == (*x).mBone->getName()) (*i).mParent = &(*x);
		}
	}
	CreateJoints();
}

std::vector<sBoneActorBindConfig> SGTRagdoll::CreateDummySkeleton()
{
	std::vector<sBoneActorBindConfig> returner;
    Ogre::SkeletonInstance* skeletonInst = mEntity->getSkeleton();
	Ogre::Skeleton::BoneIterator boneI=skeletonInst->getBoneIterator();
	Ogre::Vector3 scale = mEntity->getParentSceneNode()->_getDerivedScale();
	while(boneI.hasMoreElements())
	{
		Ogre::Bone* bone=boneI.getNext();
		Ogre::String bName=bone->getName();

		Ogre::Vector3 difference = Ogre::Vector3(0,0,0);
		Ogre::Vector3 mypos = bone->_getDerivedPosition();
		Ogre::Quaternion estimated_orientation = bone->_getDerivedOrientation();
		/*if (bone->getParent())
		{
			Ogre::Vector3 parentpos = GetRealParent(bone)->_getDerivedPosition();
			difference = mypos - parentpos;
		}*/
		if (bone->getChild(0))
		{
			Ogre::Vector3 childpos = bone->getChild(0)->_getDerivedPosition();
			if (childpos == mypos) continue;
			difference = childpos - mypos;
			estimated_orientation = Ogre::Vector3::UNIT_X.getRotationTo(difference.normalisedCopy());
		}
		/*if (bone->numChildren() > 1)
		{
			estimated_orientation = Ogre::Quaternion();
			difference = Ogre::Vector3(0,1,0);
		}*/
		if (difference.length() > 0)
		{
				sBoneActorBindConfig bone_actor_bind;
				bone_actor_bind.mBoneName = bone->getName();
				if (!bone->getParent()) bone_actor_bind.mParentName = "None";
				else bone_actor_bind.mParentName = GetRealParent(bone)->getName();
				bone_actor_bind.mInternBoneName = bone->getName();
				bone_actor_bind.mSwing1 = 30.0f;
				bone_actor_bind.mSwing2 = 30.0f;
				bone_actor_bind.mTwistMin = -20.0f;
				bone_actor_bind.mTwistMax = 20.0f;
				bone_actor_bind.mHinge = false;
				bone_actor_bind.mBoneOffsetLength = 0.0f;
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
				bone_actor_bind.mBoneDirection = difference;
				bone_actor_bind.mRagdollByParent = false;
				returner.push_back(bone_actor_bind);
			}
		}
		/*Ogre::Node::ChildNodeIterator childI = bone->getChildIterator();
		while (childI.hasMoreElements())
		{
			Ogre::Node *child = childI.getNext();
			if (bone == child->getParent())
			{
				Ogre::Vector3 childpos = child->_getDerivedPosition();
				difference = child->getPosition();
				if (compute_boneorientations) estimated_orientation = Ogre::Vector3::UNIT_X.getRotationTo(difference.normalisedCopy());
				float boneLength = difference.length();
				if (boneLength == 0.0f) continue;
				else
				{	
					sBoneActorBindConfig bone_actor_bind;
					bone_actor_bind.mBoneName = child->getName();
					bone_actor_bind.mInternBoneName = child->getName();
					bone_actor_bind.mParentName = bone->getName();
					bone_actor_bind.mSwing1 = 20.0f;
					bone_actor_bind.mSwing2 = 20.0f;
					bone_actor_bind.mTwistMin = -40.0f;
					bone_actor_bind.mTwistMax = 40.0f;
					bone_actor_bind.mHinge = false;
					bone_actor_bind.mBoneOffsetLength = 0.0f;
					if (!bone->getParent())
					{
						bone_actor_bind.mBoneLength = boneLength * 0.5f;
						bone_actor_bind.mBoneOffsetLength = 0.5f;
					}
					else bone_actor_bind.mBoneLength = boneLength * 0.9f;
					float bradius = mEntity->getBoundingRadius() / ((scale.x + scale.y + scale.z) / 3.0f);
					float minRadius = bradius * 0.02f;
					float maxRadius = bradius * 0.035f;
					float estimatedRadius = bone_actor_bind.mBoneLength * 0.25f;
					if (estimatedRadius < minRadius) bone_actor_bind.mRadius = minRadius;
					else if (estimatedRadius > maxRadius) bone_actor_bind.mRadius = maxRadius;
					else bone_actor_bind.mRadius = estimatedRadius;
					bone_actor_bind.mBoneOrientation = estimated_orientation;
					bone_actor_bind.mBoneDirection = difference;
					bone_actor_bind.mRagdollByParent = false;
					returner.push_back(bone_actor_bind);
				}*/
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
		Ogre::Vector3 position = (*i).mBone->getPosition();//(*i).mBone->_getDerivedPosition() - (*i).mActor->getGlobalPositionAsOgreVector3();//(*i).mActor->getGlobalPositionAsOgreVector3();//
		Ogre::Quaternion orientation = (*i).mParent->mActor->getGlobalOrientationAsOgreQuaternion().Inverse() * (*i).mActor->getGlobalOrientationAsOgreQuaternion();//(*i).mBone->_getDerivedOrientation();
		/*if ((*i).mParent)
		{
			position = position - (*i).mParent->mActor->getGlobalPositionAsOgreVector3();//(*i).mParent->mBone->_getDerivedPosition();
			//position  = (*i).mParent->mActor->getGlobalOrientationAsOgreQuaternion() * position;//(*i).mParent->mBone->getInitialOrientation().Inverse() * position;
			orientation = (*i).mParent->mActor->getGlobalOrientationAsOgreQuaternion().Inverse() * orientation;//(*i).mParent->mBone->_getDerivedOrientation().Inverse() * orientation;
		}*/
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
			if ((*i).mTwistMax != 0.0f || (*i).mTwistMax != 0.0f)
			{
				d6Desc.twistMotion = NX_D6JOINT_MOTION_LIMITED;
				d6Desc.twistLimit.low.value=NxMath::degToRad((*i).mTwistMin);
				d6Desc.twistLimit.low.damping=0.5f;
				d6Desc.twistLimit.low.restitution=0.0f;
				d6Desc.twistLimit.low.spring=0.0f;
				d6Desc.twistLimit.high.value=NxMath::degToRad((*i).mTwistMax);
				d6Desc.twistLimit.high.damping=0.5f;
				d6Desc.twistLimit.high.restitution=0.0f;
				d6Desc.twistLimit.high.spring=0.0f;
			}
			if ((*i).mSwing1 > 0.0f)
			{
				d6Desc.swing1Motion = NX_D6JOINT_MOTION_LIMITED;
				d6Desc.swing1Limit.value=NxMath::degToRad((*i).mSwing1);
				d6Desc.swing1Limit.damping=0.5f;
				d6Desc.swing1Limit.restitution=0.0f;
			}
			if ((*i).mSwing2 > 0.0f)
			{
				d6Desc.swing2Motion = NX_D6JOINT_MOTION_LIMITED;
				d6Desc.swing2Limit.value=NxMath::degToRad((*i).mSwing2);
				d6Desc.swing2Limit.damping=0.5f;
				d6Desc.swing2Limit.restitution=0.0f;
			}

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
		//update Bone Actor
		if ((*i).mParent)
		{
			(*i).mActor->setGlobalPosition((mEntity->getParentSceneNode()->_getDerivedOrientation() * ((*i).mBone->_getDerivedPosition()+(*i).mOffset) * scale) + mEntity->getParentSceneNode()->_getDerivedPosition());  
			Ogre::Quaternion orientation = (*i).mBone->_getDerivedOrientation();
			orientation = (*i).mParent->mBone->_getDerivedOrientation().Inverse() * orientation;
			(*i).mJoint.mJoint->setDriveOrientation(NxOgre::NxConvert<NxQuat, Ogre::Quaternion>(orientation));
		}
		else
		{
			(*i).mActor->setGlobalPosition((mEntity->getParentSceneNode()->_getDerivedOrientation() * (*i).mBone->_getDerivedPosition() * scale) + mEntity->getParentSceneNode()->_getDerivedPosition());  
			(*i).mActor->setGlobalOrientation(NxOgre::NxConvert<NxQuat, Ogre::Quaternion>((*i).mBoneActorGlobalBindOrientation * (*i).mBone->_getDerivedOrientation() * mEntity->getParentSceneNode()->_getDerivedOrientation()));
		}

	} 
}

void SGTRagdoll::UpdateVisualBones()
{
	Ogre::Quaternion PhysxRotation, OgreGlobalQuat, NodeRotation = mEntity->getParentSceneNode()->_getDerivedOrientation();
	for (std::vector<sBoneActorBind>::iterator i = mSkeleton.begin(); i != mSkeleton.end(); i++)
	{
		Ogre::Bone *bone = (*i).mBone;
		/*if ((*i).mRagdollByParent == true)
		{
			bone = (Ogre::Bone*)(*i).mBone->getParent();
			(*i).mBone->setOrientation(Ogre::Quaternion());
		}*/
		PhysxRotation = (*i).mActor->getGlobalOrientationAsOgreQuaternion() * (*i).mBoneActorGlobalBindOrientationInverse;
		OgreGlobalQuat = PhysxRotation * (*i).mBoneGlobalBindOrientation;
		Ogre::Quaternion ParentInverse = NodeRotation.Inverse();
		if (bone->getParent())
		{
			ParentInverse = (NodeRotation * bone->getParent()->_getDerivedOrientation()).Inverse();
		}
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
	ResetBones();
	for (std::vector<sBoneActorBind>::iterator i = mSkeleton.begin(); i != mSkeleton.end(); i++)
	{
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
			if ((*i).mTwistMax != 0.0f || (*i).mTwistMax != 0.0f)
			{
				(*i).mJoint.mDescription.twistMotion = NX_D6JOINT_MOTION_LIMITED;
				(*i).mJoint.mDescription.twistLimit.low.value=NxMath::degToRad((*i).mTwistMin);
				(*i).mJoint.mDescription.twistLimit.low.damping=0.5f;
				(*i).mJoint.mDescription.twistLimit.low.restitution=0.0f;
				(*i).mJoint.mDescription.twistLimit.low.spring=0.0f;
				(*i).mJoint.mDescription.twistLimit.high.value=NxMath::degToRad((*i).mTwistMax);
				(*i).mJoint.mDescription.twistLimit.high.damping=0.5f;
				(*i).mJoint.mDescription.twistLimit.high.restitution=0.0f;
				(*i).mJoint.mDescription.twistLimit.high.spring=0.0f;
			}
			if ((*i).mSwing1 > 0.0f)
			{
				(*i).mJoint.mDescription.swing1Motion = NX_D6JOINT_MOTION_LIMITED;
				(*i).mJoint.mDescription.swing1Limit.value=NxMath::degToRad((*i).mSwing1);
				(*i).mJoint.mDescription.swing1Limit.damping=0.5f;
				(*i).mJoint.mDescription.swing1Limit.restitution=0.0f;
			}
			if ((*i).mSwing2 > 0.0f)
			{
				(*i).mJoint.mDescription.swing2Motion = NX_D6JOINT_MOTION_LIMITED;
				(*i).mJoint.mDescription.swing2Limit.value=NxMath::degToRad((*i).mSwing2);
				(*i).mJoint.mDescription.swing2Limit.damping=0.5f;
				(*i).mJoint.mDescription.swing2Limit.restitution=0.0f;
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
	mAnimationStateStr = statename;
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

void SGTRagdoll::ReceiveMessage(SGTMsg &msg)
{
	if (msg.mNewsgroup == "UPDATE_PER_FRAME")
	{
		Update(msg.mData.GetFloat("TIME"));
	}
}

void SGTRagdoll::CreateFromDataMap(SGTDataMap *parameters)
{
	Ogre::String meshname = parameters->GetOgreString("MeshName");
	Ogre::String animstate = parameters->GetOgreString("AnimState");
	bool shadowcaster = parameters->GetBool("ShadowCaster");
	bool ragdoll = parameters->GetBool("Ragdoll");
	Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
	scale = parameters->GetOgreVec3("Scale");
	Create(meshname, scale);
	SetAnimationState(animstate);
	mEntity->setCastShadows(shadowcaster);
	CreateBoneObjects();
	if (ragdoll) SetControlToActors();
}
void SGTRagdoll::GetParameters(SGTDataMap *parameters)
{
	parameters->AddOgreString("MeshName", mEntity->getMesh()->getName());
	parameters->AddOgreString("AnimState", mAnimationStateStr);
	parameters->AddBool("Ragdoll", false);
	parameters->AddBool("ShadowCaster", mEntity->getCastShadows());
}
void SGTRagdoll::GetDefaultParameters(SGTDataMap *parameters)
{
	parameters->AddOgreString("MeshName", "");
	parameters->AddOgreString("AnimState", "Idle");
	parameters->AddBool("Ragdoll", false);
	parameters->AddBool("ShadowCaster", true);
}

void SGTRagdoll::Save(SGTSaveSystem& mgr)
{
	mgr.SaveAtom("Ogre::String", (void*)&mEntity->getMesh()->getName(), "MeshName");
	mgr.SaveAtom("Ogre::Vector3", &mOwnerGO->GetGlobalScale(), "Scale");
	mgr.SaveAtom("Ogre::String", &mAnimationStateStr, "AnimState");
	mgr.SaveAtom("bool", &mControlledByActors, "Ragdoll");
	bool shadow = mEntity->getCastShadows();
	mgr.SaveAtom("bool", &shadow, "ShadowCaster");
}
void SGTRagdoll::Load(SGTLoadSystem& mgr)
{
	Ogre::String meshname;
	Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
	Ogre::String animstate;
	bool shadowcaster = true;
	bool ragdoll = false;
	mgr.LoadAtom("Ogre::String", &meshname);
	mgr.LoadAtom("Ogre::Vector3", &scale);
	mgr.LoadAtom("Ogre::String", &animstate);
	mgr.LoadAtom("ShadowCaster", &shadowcaster);
	mgr.LoadAtom("Ragdoll", &ragdoll);
	Create(meshname, scale);
	SetAnimationState(animstate);
	mEntity->setCastShadows(shadowcaster);
	if (ragdoll) SetControlToActors();
}