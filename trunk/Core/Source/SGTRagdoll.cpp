
#include "SGTRagdoll.h"
#include "SGTMain.h"
#include <fstream>
#include "SGTSceneManager.h"
#include "SGTGameState.h"

SGTGOCRagdollBone::SGTGOCRagdollBone(void)
{
	mNode = SGTMain::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode();
	mOffsetNode = mNode->createChildSceneNode(Ogre::Vector3(0,0,0), Ogre::Quaternion());
	mEntity = SGTMain::Instance().GetOgreSceneMgr()->createEntity(Ogre::StringConverter::toString(SGTSceneManager::Instance().RequestID()), "capsule.mesh");
	mEntity->setCastShadows(false);
	mEntity->setUserObject(mOwnerGO);
	mOffsetNode->attachObject(mEntity);
	mDebugAnimation = true;
	mMeshNode = 0;
	mJointAxisNode = 0;
	mJointAxis = 0;
	mBone = 0;
	mRagBoneRef = 0;
	mBoneConfig.mBoneOffset = Ogre::Vector3(0,0,0);
	mGlobalBindPosition = Ogre::Vector3(0,0,0);
	mBoneConfig.mBoneOrientation = Ogre::Quaternion();
	mBoneConfig.mTwistMax = 20.0f;
	mBoneConfig.mTwistMin = -20.0f;
	mBoneConfig.mSwing1 = 30.0f;
	mBoneConfig.mSwing2 = 30.0f;
}
SGTGOCRagdollBone::~SGTGOCRagdollBone(void)
{
	SGTMain::Instance().GetOgreSceneMgr()->destroyEntity(mEntity);
	if (mJointAxis) SGTMain::Instance().GetOgreSceneMgr()->destroyManualObject(mJointAxis);
	if (mJointAxisNode) SGTMain::Instance().GetOgreSceneMgr()->destroySceneNode(mJointAxisNode);
	SGTMain::Instance().GetOgreSceneMgr()->destroySceneNode(mOffsetNode);
	SGTMain::Instance().GetOgreSceneMgr()->destroySceneNode(mNode);
	if (mBone) mBone->setManuallyControlled(false);
	if (mRagBoneRef) mRagBoneRef->mRagdoll->mBoneObjects.remove(mOwnerGO);
}

void* SGTGOCRagdollBone::GetUserData()
{
	return mRagBoneRef;
}
void SGTGOCRagdollBone::InjectUserData(void* data)
{
	mRagBoneRef = (RagBoneRef*)data;
	mBone = mRagBoneRef->mBone;
	mMeshNode = mRagBoneRef->mMeshNode;
	mBone->setManuallyControlled(mDebugAnimation);
	mBoneConfig.mBoneName = mBone->getName();
	mBoneGlobalBindOrientation = mBone->_getDerivedOrientation();
	mRagBoneRef->mRagdoll->mBoneObjects.push_back(mOwnerGO);
	float scale_factor = ((mMeshNode->_getDerivedScale().x + mMeshNode->_getDerivedScale().y + mMeshNode->_getDerivedScale().z) / 3);
	mBoneConfig.mBoneLength = mBoneConfig.mBoneLength / scale_factor;
	mBoneConfig.mBoneOffset = mBoneConfig.mBoneOffset / scale_factor;
	mBoneConfig.mRadius = mBoneConfig.mRadius / scale_factor;
	ScaleNode();
	mGlobalBindPosition = mOwnerGO->GetGlobalPosition() - (mBoneConfig.mBoneOffset*scale_factor);
	if (mBoneConfig.mParentName != "None") CreateJointAxis();
}

void SGTGOCRagdollBone::CreateFromDataMap(SGTDataMap *parameters)
{
	mBoneConfig.mParentName = parameters->GetOgreString("mParentName");
	mBoneConfig.mBoneLength = parameters->GetFloat("BoneLength");
	mBoneConfig.mBoneOffset = parameters->GetOgreVec3("BoneOffset");
	mBoneConfig.mRadius = parameters->GetFloat("Radius");
	mBoneConfig.mJointOrientation = parameters->GetOgreQuat("JointOrientation");
	mBoneConfig.mSwing1 = parameters->GetFloat("Swing1");
	mBoneConfig.mSwing2 = parameters->GetFloat("Swing2");
	mBoneConfig.mTwistMax = parameters->GetFloat("TwistMax");
	mBoneConfig.mTwistMin = parameters->GetFloat("TwistMin");
	mDebugAnimation = parameters->GetBool("TestAnimation");
}

void SGTGOCRagdollBone::GetParameters(SGTDataMap *parameters)
{
	parameters->AddOgreString("mParentName", mBoneConfig.mParentName);
	float scale_factor = ((mMeshNode->_getDerivedScale().x + mMeshNode->_getDerivedScale().y + mMeshNode->_getDerivedScale().z) / 3);
	parameters->AddFloat("BoneLength", mBoneConfig.mBoneLength * scale_factor);
	parameters->AddOgreVec3("BoneOffset", mBoneConfig.mBoneOffset * scale_factor);
	parameters->AddFloat("Radius", mBoneConfig.mRadius * scale_factor);
	parameters->AddOgreQuat("JointOrientation", mBoneConfig.mJointOrientation);
	parameters->AddFloat("Swing1", mBoneConfig.mSwing1);
	parameters->AddFloat("Swing2", mBoneConfig.mSwing2);
	parameters->AddFloat("TwistMax", mBoneConfig.mTwistMax);
	parameters->AddFloat("TwistMin", mBoneConfig.mTwistMin);
	parameters->AddBool("TestAnimation", mDebugAnimation);
}
void SGTGOCRagdollBone::GetDefaultParameters(SGTDataMap *parameters)
{
}
void SGTGOCRagdollBone::UpdatePosition(Ogre::Vector3 position)
{
	if (mNode)
	{
		mNode->setPosition(position);
		if (mBone)
		{
			float scale_factor = ((mMeshNode->_getDerivedScale().x + mMeshNode->_getDerivedScale().y + mMeshNode->_getDerivedScale().z) / 3);
			if (mOwnerGO->GetUpdatingFromParent())
			{
				//if root bone moves, don't update BoneOffset
				SGTGameObject *superparent = mOwnerGO->GetParent();
				while (superparent->GetParent()) superparent = superparent->GetParent(); 
				if (superparent->GetTranformingChildren()) mGlobalBindPosition = position - (mBoneConfig.mBoneOffset*scale_factor);
				else
				{
					float scale_factor = ((mMeshNode->_getDerivedScale().x + mMeshNode->_getDerivedScale().y + mMeshNode->_getDerivedScale().z) / 3);
					mBoneConfig.mBoneOffset = position - mGlobalBindPosition;
					mBoneConfig.mBoneOffset = mBoneConfig.mBoneOffset / scale_factor;
				}
			}
			else if (mOwnerGO->GetParent() || !mOwnerGO->GetTranformingChildren())
			{
				mBoneConfig.mBoneOffset = position - mGlobalBindPosition;
				mBoneConfig.mBoneOffset = mBoneConfig.mBoneOffset / scale_factor;
			}
		}
	}
}

void SGTGOCRagdollBone::UpdateScale(Ogre::Vector3 scale)
{
	/*if (mNode)
	{
		mBoneConfig.mRadius = mBoneConfig.mBoneLength * ((scale.x + scale.z) * 0.5f);
		mBoneConfig.mBoneLength = mBoneConfig.mBoneLength*scale.y;
		ScaleNode();
	}*/
}

void SGTGOCRagdollBone::UpdateOrientation(Ogre::Quaternion orientation)
{
	if (mNode)
	{
		mNode->setOrientation(orientation);
		Ogre::Quaternion q = Ogre::Vector3(1,0,0).getRotationTo(Ogre::Vector3(0,1,0));
		mBoneConfig.mBoneOrientation = orientation * q.Inverse();
		if (GetTestAnimation() && mBone)
		{
			/*Ogre::Quaternion parentquat = mMeshNode->_getDerivedOrientation();
			if (mBone->getParent()) parentquat = parentquat * mBone->getParent()->_getDerivedOrientation();
			mBone->setOrientation(parentquat.Inverse() * orientation);*/
			Ogre::Quaternion PhysxRotation, OgreGlobalQuat, NodeRotation = mMeshNode->_getDerivedOrientation();
			PhysxRotation = orientation * mBoneActorGlobalBindOrientationInverse;
			OgreGlobalQuat = PhysxRotation * mBoneGlobalBindOrientation;
			Ogre::Quaternion ParentInverse = NodeRotation.Inverse();
			if (mBone->getParent())
			{
				ParentInverse = (NodeRotation * mBone->getParent()->_getDerivedOrientation()).Inverse();
			}
			mBone->setOrientation(ParentInverse * OgreGlobalQuat);
		}
		else
		{
			mBoneActorGlobalBindOrientationInverse = orientation.Inverse();
		}
	}
}

bool SGTGOCRagdollBone::GetTestAnimation()
{
	if (!mDebugAnimation) return false;
	if (mOwnerGO->GetParent())
	{
		if (mOwnerGO->GetParent()->GetComponent("RagdollBone"))
		{
			return ((SGTGOCRagdollBone*)(mOwnerGO->GetParent()->GetComponent("RagdollBone")))->GetTestAnimation();
		}
	}
	return mDebugAnimation;
}

void SGTGOCRagdollBone::CreateJointAxis()
{
	float scale_factor = ((mMeshNode->_getDerivedScale().x + mMeshNode->_getDerivedScale().y + mMeshNode->_getDerivedScale().z) / 3);
	mJointAxis = SGTMain::Instance().GetOgreSceneMgr()->createManualObject("AxisLine_" + mOwnerGO->GetName());
	mJointAxis->begin("WPLine", Ogre::RenderOperation::OT_TRIANGLE_LIST);
	float width = 0.05;
	float height = mBoneConfig.mBoneLength*scale_factor;
	mJointAxis->position(Ogre::Vector3(0, -height*0.5, 0)); //0
	mJointAxis->colour(1,0,0);
	mJointAxis->position(Ogre::Vector3(width, -height*0.5, 0)); //1
	mJointAxis->colour(1,0,0);
	mJointAxis->position(Ogre::Vector3(0, -height*0.5, width)); //2
	mJointAxis->colour(1,0,0);
	mJointAxis->position(Ogre::Vector3(width, -height*0.5, width)); //3
	mJointAxis->colour(1,0,0);
	mJointAxis->position(Ogre::Vector3(0, 0, 0)); //4
	mJointAxis->colour(1,0,0);
	mJointAxis->position(Ogre::Vector3(width, 0, 0)); //5
	mJointAxis->colour(1,0,0);
	mJointAxis->position(Ogre::Vector3(0, 0, width)); //6
	mJointAxis->colour(1,0,0);
	mJointAxis->position(Ogre::Vector3(width, 0, width)); //7

	mJointAxis->quad(0, 1, 3, 2); //away
	mJointAxis->quad(6, 7, 5, 4); //forward

	mJointAxis->quad(4,5,1,0);  //forwad
	mJointAxis->quad(7,6,2,3);  //away

	mJointAxis->quad(0,2,6,4);  //away	
	mJointAxis->quad(3, 1, 5, 7); //forward

	mJointAxis->end();
	mJointAxisNode = mNode->createChildSceneNode("AxisLine_" + mOwnerGO->GetName());
	mJointAxisNode->setInheritOrientation(false);
	mJointAxisNode->setOrientation(mBoneConfig.mJointOrientation);
	mJointAxisNode->attachObject(mJointAxis);
}

void SGTGOCRagdollBone::ScaleNode()
{
	float scale_factor = ((mMeshNode->_getDerivedScale().x + mMeshNode->_getDerivedScale().y + mMeshNode->_getDerivedScale().z) / 3);
	float capsule_height = (scale_factor*mBoneConfig.mBoneLength) - (scale_factor*mBoneConfig.mBoneLength * 0.4f);
	if (capsule_height <= 0.0f) capsule_height = 0.1f;
	Ogre::Vector3 offset = Ogre::Vector3(0.0f,-mBoneConfig.mBoneLength*0.5f*scale_factor,0.0f);
	mOffsetNode->setScale(Ogre::Vector3(mBoneConfig.mRadius*scale_factor, capsule_height, mBoneConfig.mRadius*scale_factor));
	mOffsetNode->setPosition(offset);
}

void SGTGOCRagdollBone::SetBone(Ogre::SceneNode *meshnode, SGTRagdoll* ragdoll, sBoneActorBind bone_config, bool controlBone)
{
	mMeshNode = meshnode;
	mBone = bone_config.mBone;
	mDebugAnimation = controlBone;
	mBone->setManuallyControlled(mDebugAnimation);
	mBoneConfig.mBoneName = mBone->getName();
	mBoneConfig.mJointOrientation = bone_config.mJointOrientation;
	mBoneConfig.mRadius = bone_config.mBoneRadius;
	mBoneConfig.mBoneLength = bone_config.mBoneLength;
	mBoneConfig.mBoneOffset = bone_config.mOffset;
	mBoneConfig.mSwing1 = bone_config.mSwing1;
	mBoneConfig.mSwing2 = bone_config.mSwing2;
	mBoneConfig.mTwistMax = bone_config.mTwistMax;
	mBoneConfig.mTwistMin = bone_config.mTwistMin;
	if (bone_config.mParent)
	{
		mBoneConfig.mParentName = bone_config.mParent->mBone->getName();
		CreateJointAxis();
	}
	else mBoneConfig.mParentName = "None";

	mBoneGlobalBindOrientation = mBone->_getDerivedOrientation();
	ScaleNode();
	float scale_factor = ((mMeshNode->_getDerivedScale().x + mMeshNode->_getDerivedScale().y + mMeshNode->_getDerivedScale().z) / 3);
	mGlobalBindPosition = mOwnerGO->GetGlobalPosition() - (mBoneConfig.mBoneOffset*scale_factor);

	mRagBoneRef = new RagBoneRef();
	mRagBoneRef->mBone = mBone;
	mRagBoneRef->mMeshNode = mMeshNode;
	mRagBoneRef->mRagdoll = ragdoll;
	mRagBoneRef->mRagdoll->mBoneObjects.push_back(mOwnerGO);
}

void SGTGOCRagdollBone::SetOwner(SGTGameObject *go)
{
	mOwnerGO = go;
	mEntity->setUserObject(mOwnerGO);
}

SGTRagdoll::~SGTRagdoll(void)
{
	std::list<SGTGameObject*>::iterator i = mBoneObjects.begin();
	while (i != mBoneObjects.end())
	{
		delete (*i);
		i = mBoneObjects.begin();
	}

	for (std::vector<sBoneActorBind>::iterator i = mSkeleton.begin(); i != mSkeleton.end(); i++)
	{
		if ((*i).mParent) SGTMain::Instance().GetNxScene()->getNxScene()->releaseJoint(*(*i).mJoint.mJoint);
		SGTMain::Instance().GetNxScene()->destroyActor((*i).mActor->getName());
	}
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
				Ogre::String bonename, boneparentname, shaperadius, shapelength, shapeoffset, shapeorientation, jointorientation, hinge, swing1, swing2, twistmax, twistmin, ragdollcontrolbyparent;
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
					if (i->first == "Swing1") swing1 = i->second;
					if (i->first == "Swing2") swing2 = i->second;
					if (i->first == "TwistMax") twistmax = i->second;
					if (i->first == "TwistMin") twistmin = i->second;
					if (i->first == "RagdollControlByParent") ragdollcontrolbyparent = i->second;	//Fix for jaiqua
				}
				if (bonename == "" || boneparentname == "" || shaperadius == "" || shapeoffset == "" || hinge == "" || swing1 == "" || swing2 == "" || twistmax == "" || twistmin == "")
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
				boneconfig.mSwing1 = Ogre::StringConverter::parseReal(swing1);
				boneconfig.mSwing2 = Ogre::StringConverter::parseReal(swing2);
				boneconfig.mTwistMax = Ogre::StringConverter::parseReal(twistmax);
				boneconfig.mTwistMin = Ogre::StringConverter::parseReal(twistmin);
				bones.push_back(boneconfig);
			}
		}	
		CreateSkeleton(bones);
	}
	else
	{
		Ogre::LogManager::getSingleton().logMessage("Could not find " + mEntity->getMesh()->getName() + ".bones, generating new...");
		std::vector<sBoneActorBindConfig> boneconfig = CreateDummySkeleton();
		CreateSkeleton(boneconfig);
		Serialise(boneconfig);
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
		f << "Swing1 = " << (*i).mSwing1 << std::endl;
		f << "Swing2 = " << (*i).mSwing2 << std::endl;
		f << "TwistMax = " << (*i).mTwistMax << std::endl;
		f << "TwistMin = " << (*i).mTwistMin << std::endl;
		f << std::endl;
	}
	f.close();
}

void SGTRagdoll::SerialiseBoneObjects()
{
	std::vector<sBoneActorBindConfig> boneconfig;
	for (std::list<SGTGameObject*>::iterator i = mBoneObjects.begin(); i != mBoneObjects.end(); i++)
	{
		SGTGOCRagdollBone *bone = (SGTGOCRagdollBone*)(*i)->GetComponent("RagdollBone");
		boneconfig.push_back(bone->GetBoneConfig());
	}
	Serialise(boneconfig);
}

void SGTRagdoll::CreateBoneObjects()
{
	mBoneObjects.clear();
	std::map<Ogre::String, SGTGameObject*> bonemap;
	for (std::vector<sBoneActorBind>::iterator i = mSkeleton.begin(); i != mSkeleton.end(); i++)
	{
		SGTGameObject *go = new SGTGameObject();
		SGTGOCRagdollBone *bone = new SGTGOCRagdollBone();
		go->AddComponent(bone);
		go->SetGlobalPosition((*i).mActor->getGlobalPositionAsOgreVector3());
		Ogre::Quaternion q = Ogre::Vector3(1,0,0).getRotationTo(Ogre::Vector3(0,1,0));
		go->SetGlobalOrientation((*i).mActor->getGlobalOrientationAsOgreQuaternion() * q);
		bone->SetBone(mNode, this, (*i), true);
		//go->SetFreezePosition(true);
		(*i).mVisualBone = go;
		bonemap.insert(std::make_pair<Ogre::String, SGTGameObject*>((*i).mBone->getName(), go));
	}
	for (std::map<Ogre::String, SGTGameObject*>::iterator i = bonemap.begin(); i != bonemap.end(); i++)
	{
		Ogre::Bone *bone = mEntity->getSkeleton()->getBone((*i).first);
		if (bone->getParent())
		{
			std::map<Ogre::String, SGTGameObject*>::iterator search = bonemap.find(bone->getParent()->getName());
			if (search != bonemap.end()) (*i).second->SetParent(search->second);
		}
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
		NxFindRotationMatrix(NxVec3(0.0f,1.0f,0.0f), NxVec3(1.0,0.0f,0.0f), sp.mLocalPose.M);
		sp.mLocalPose.t = NxVec3(scaled_bonelength/2.0f,0.0f,0.0f);
		Ogre::Quaternion q;
		q.FromAngleAxis(Ogre::Radian(Ogre::Degree(90)), Ogre::Vector3(0,0,1));
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

		Ogre::Vector3 difference = Ogre::Vector3(0,1,0);
		Ogre::Vector3 mypos = bone->_getDerivedPosition();
		Ogre::Quaternion estimated_orientation = Ogre::Quaternion(0.707107, 0, 0, 0.707107);
		if (bone->numChildren() == 1 && bone->getChild(0))
		{
			//points towards child
			Ogre::Vector3 childpos = bone->getChild(0)->_getDerivedPosition();
			if (childpos == mypos) continue;
			difference = childpos - mypos;
			estimated_orientation = Ogre::Vector3::UNIT_X.getRotationTo(difference.normalisedCopy());
		}
		else if (bone->getParent())
		{
			//points away from parent
			difference = Ogre::Vector3(1,0,0);
			Ogre::Vector3 parentpos = bone->getParent()->_getDerivedPosition();
			if (parentpos == mypos) continue;
			difference = parentpos - mypos;
			estimated_orientation = Ogre::Vector3::UNIT_X.getRotationTo(-difference.normalisedCopy());
			difference.normalise();
			difference = difference * 0.1f * (mEntity->getBoundingRadius() / ((scale.x + scale.y + scale.z) / 3.0f));
		}
		if (difference.length() > 0)
		{
				sBoneActorBindConfig bone_actor_bind;
				bone_actor_bind.mBoneName = bone->getName();
				if (!bone->getParent()) bone_actor_bind.mParentName = "None";
				else bone_actor_bind.mParentName = GetRealParent(bone)->getName();

				bone_actor_bind.mBoneOffset = Ogre::Vector3(0,0,0);
				bone_actor_bind.mJointOrientation = Ogre::Quaternion();
				bone_actor_bind.mNeedsJointOrientation = true;
				bone_actor_bind.mSwing1 = 30.0f;
				bone_actor_bind.mSwing2 = 30.0f;
				bone_actor_bind.mTwistMin = -20.0f;
				bone_actor_bind.mTwistMax = 20.0f;
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
	if (animstate != "") SetAnimationState(animstate);
	mEntity->setCastShadows(shadowcaster);
	Update(0);
	ResetBones();
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
	mgr.LoadAtom("bool", &shadowcaster);
	mgr.LoadAtom("bool", &ragdoll);
	Create(meshname, scale);
	if (animstate != "") SetAnimationState(animstate);
	mEntity->setCastShadows(shadowcaster);
	if (ragdoll) SetControlToActors();
}