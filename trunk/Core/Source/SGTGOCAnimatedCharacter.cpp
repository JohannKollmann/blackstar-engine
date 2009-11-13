
#include "SGTGOCAnimatedCharacter.h"
#include "SGTSceneManager.h"
#include "SGTMain.h"
#include "SGTGOCCharacterController.h"


SGTGOCAnimatedCharacterBone::SGTGOCAnimatedCharacterBone(void)
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

	mBoneConfig.mTwistMax.mValue = 20.0f;
	mBoneConfig.mTwistMax.mDamping = 0.5f;
	mBoneConfig.mTwistMax.mRestitution = 0.0f;
	mBoneConfig.mTwistMax.mSpring = 0.0f;

	mBoneConfig.mTwistMin.mValue = -20.0f;
	mBoneConfig.mTwistMin.mDamping = 0.5f;
	mBoneConfig.mTwistMin.mRestitution = 0.0f;
	mBoneConfig.mTwistMin.mSpring = 0.0f;

	mBoneConfig.mSwing1.mValue = 30.0f;
	mBoneConfig.mSwing1.mDamping = 0.5f;
	mBoneConfig.mSwing1.mRestitution = 0.0f;
	mBoneConfig.mSwing1.mSpring = 0.0f;

	mBoneConfig.mSwing2.mValue = 30.0f;
	mBoneConfig.mSwing2.mDamping = 0.5f;
	mBoneConfig.mSwing2.mRestitution = 0.0f;
	mBoneConfig.mSwing2.mSpring = 0.0f;
}
SGTGOCAnimatedCharacterBone::~SGTGOCAnimatedCharacterBone(void)
{
	SGTMain::Instance().GetOgreSceneMgr()->destroyEntity(mEntity);
	if (mJointAxis) SGTMain::Instance().GetOgreSceneMgr()->destroyManualObject(mJointAxis);
	if (mJointAxisNode) SGTMain::Instance().GetOgreSceneMgr()->destroySceneNode(mJointAxisNode);
	SGTMain::Instance().GetOgreSceneMgr()->destroySceneNode(mOffsetNode);
	SGTMain::Instance().GetOgreSceneMgr()->destroySceneNode(mNode);
	if (mBone) mBone->setManuallyControlled(false);
	if (mRagBoneRef) mRagBoneRef->mGOCRagdoll->mBoneObjects.remove(mOwnerGO);
}

void* SGTGOCAnimatedCharacterBone::GetUserData()
{
	return mRagBoneRef;
}
void SGTGOCAnimatedCharacterBone::InjectUserData(void* data)
{
	mRagBoneRef = (RagBoneRef*)data;
	mBone = mRagBoneRef->mBone;
	mMeshNode = mRagBoneRef->mMeshNode;
	mBone->setManuallyControlled(mDebugAnimation);
	mBoneConfig.mBoneName = mBone->getName();
	mBoneGlobalBindOrientation = mBone->_getDerivedOrientation();
	mRagBoneRef->mGOCRagdoll->mBoneObjects.push_back(mOwnerGO);
	float scale_factor = ((mMeshNode->_getDerivedScale().x + mMeshNode->_getDerivedScale().y + mMeshNode->_getDerivedScale().z) / 3);
	mBoneConfig.mBoneLength = mBoneConfig.mBoneLength / scale_factor;
	mBoneConfig.mBoneOffset = mBoneConfig.mBoneOffset / scale_factor;
	mBoneConfig.mRadius = mBoneConfig.mRadius / scale_factor;
	ScaleNode();
	mGlobalBindPosition = mOwnerGO->GetGlobalPosition() - (mBoneConfig.mBoneOffset*scale_factor);
	if (mBoneConfig.mParentName != "None")
	{
		mBoneConfig.mJointOrientation = mOwnerGO->GetParent()->GetGlobalOrientation().Inverse() * mBoneConfig.mJointOrientation;
		CreateJointAxis();
	}
}

void SGTGOCAnimatedCharacterBone::CreateFromDataMap(SGTDataMap *parameters)
{
	mBoneConfig.mParentName = parameters->GetOgreString("mParentName");
	mBoneConfig.mBoneLength = parameters->GetFloat("BoneLength");
	mBoneConfig.mBoneOffset = parameters->GetOgreVec3("BoneOffset");
	mBoneConfig.mRadius = parameters->GetFloat("Radius");
	mBoneConfig.mJointOrientation = parameters->GetOgreQuat("JointOrientation");
	mBoneConfig.mNeedsJointOrientation = false;
	mBoneConfig.mSwing1.mValue = parameters->GetFloat("Swing1_Value");
	mBoneConfig.mSwing1.mDamping = parameters->GetFloat("Swing1_Damping");
	mBoneConfig.mSwing1.mRestitution = parameters->GetFloat("Swing1_Restitution");
	mBoneConfig.mSwing1.mSpring = parameters->GetFloat("Swing1_Spring");

	mBoneConfig.mSwing2.mValue = parameters->GetFloat("Swing2_Value");
	mBoneConfig.mSwing2.mDamping = parameters->GetFloat("Swing2_Damping");
	mBoneConfig.mSwing2.mRestitution = parameters->GetFloat("Swing2_Restitution");
	mBoneConfig.mSwing2.mSpring = parameters->GetFloat("Swing2_Spring");

	mBoneConfig.mTwistMax.mValue = parameters->GetFloat("TwistMax_Value");
	mBoneConfig.mTwistMax.mDamping = parameters->GetFloat("TwistMax_Damping");
	mBoneConfig.mTwistMax.mRestitution = parameters->GetFloat("TwistMax_Restitution");
	mBoneConfig.mTwistMax.mSpring = parameters->GetFloat("TwistMax_Spring");

	mBoneConfig.mTwistMin.mValue = parameters->GetFloat("TwistMin_Value");
	mBoneConfig.mTwistMin.mDamping = parameters->GetFloat("TwistMin_Damping");
	mBoneConfig.mTwistMin.mRestitution = parameters->GetFloat("TwistMin_Restitution");
	mBoneConfig.mTwistMin.mSpring = parameters->GetFloat("TwistMin_Spring");

	mDebugAnimation = parameters->GetBool("TestAnimation");
}

void SGTGOCAnimatedCharacterBone::GetParameters(SGTDataMap *parameters)
{
	parameters->AddOgreString("mParentName", mBoneConfig.mParentName);
	float scale_factor = ((mMeshNode->_getDerivedScale().x + mMeshNode->_getDerivedScale().y + mMeshNode->_getDerivedScale().z) / 3);
	parameters->AddFloat("BoneLength", mBoneConfig.mBoneLength * scale_factor);
	parameters->AddOgreVec3("BoneOffset", mBoneConfig.mBoneOffset * scale_factor);
	parameters->AddFloat("Radius", mBoneConfig.mRadius * scale_factor);
	Ogre::Quaternion parentOrientation = Ogre::Quaternion();
	if (mOwnerGO->GetParent()) parentOrientation = mOwnerGO->GetParent()->GetGlobalOrientation();
	parameters->AddOgreQuat("JointOrientation", parentOrientation * mBoneConfig.mJointOrientation);

	parameters->AddFloat("Swing1_Value", mBoneConfig.mSwing1.mValue);
	parameters->AddFloat("Swing1_Damping", mBoneConfig.mSwing1.mDamping);
	parameters->AddFloat("Swing1_Restitution", mBoneConfig.mSwing1.mRestitution);
	parameters->AddFloat("Swing1_Spring", mBoneConfig.mSwing1.mSpring);

	parameters->AddFloat("Swing2_Value", mBoneConfig.mSwing2.mValue);
	parameters->AddFloat("Swing2_Damping", mBoneConfig.mSwing2.mDamping);
	parameters->AddFloat("Swing2_Restitution", mBoneConfig.mSwing2.mRestitution);
	parameters->AddFloat("Swing2_Spring", mBoneConfig.mSwing2.mSpring);

	parameters->AddFloat("TwistMax_Value", mBoneConfig.mTwistMax.mValue);
	parameters->AddFloat("TwistMax_Damping", mBoneConfig.mTwistMax.mDamping);
	parameters->AddFloat("TwistMax_Restitution", mBoneConfig.mTwistMax.mRestitution);
	parameters->AddFloat("TwistMax_Spring", mBoneConfig.mTwistMax.mSpring);

	parameters->AddFloat("TwistMin_Value", mBoneConfig.mTwistMin.mValue);
	parameters->AddFloat("TwistMin_Damping", mBoneConfig.mTwistMin.mDamping);
	parameters->AddFloat("TwistMin_Restitution", mBoneConfig.mTwistMin.mRestitution);
	parameters->AddFloat("TwistMin_Spring", mBoneConfig.mTwistMin.mSpring);

	parameters->AddBool("TestAnimation", mDebugAnimation);
}
void SGTGOCAnimatedCharacterBone::GetDefaultParameters(SGTDataMap *parameters)
{
}
void SGTGOCAnimatedCharacterBone::UpdatePosition(Ogre::Vector3 position)
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

void SGTGOCAnimatedCharacterBone::UpdateScale(Ogre::Vector3 scale)
{
	/*if (mNode)
	{
		mBoneConfig.mRadius = mBoneConfig.mBoneLength * ((scale.x + scale.z) * 0.5f);
		mBoneConfig.mBoneLength = mBoneConfig.mBoneLength*scale.y;
		ScaleNode();
	}*/
}

void SGTGOCAnimatedCharacterBone::UpdateOrientation(Ogre::Quaternion orientation)
{
	if (mNode)
	{
		mNode->setOrientation(orientation);
		//Ogre::Quaternion q = Ogre::Vector3(1,0,0).getRotationTo(Ogre::Vector3(0,1,0));
		mBoneConfig.mBoneOrientation = orientation;// * q.Inverse();
		if (GetTestAnimation() && mBone)
		{
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

bool SGTGOCAnimatedCharacterBone::GetTestAnimation()
{
	if (!mDebugAnimation) return false;
	if (mOwnerGO->GetParent())
	{
		if (mOwnerGO->GetParent()->GetComponent("AnimatedCharacterBone"))
		{
			return ((SGTGOCAnimatedCharacterBone*)(mOwnerGO->GetParent()->GetComponent("AnimatedCharacterBone")))->GetTestAnimation();
		}
	}
	return mDebugAnimation;
}

void SGTGOCAnimatedCharacterBone::CreateJointAxis()
{
	float scale_factor = ((mMeshNode->_getDerivedScale().x + mMeshNode->_getDerivedScale().y + mMeshNode->_getDerivedScale().z) / 3);
	mJointAxis = SGTMain::Instance().GetOgreSceneMgr()->createManualObject("AxisLine_" + mOwnerGO->GetName());
	mJointAxis->begin("WPLine", Ogre::RenderOperation::OT_TRIANGLE_LIST);
	float width = 0.05;
	float height = mBoneConfig.mBoneLength*scale_factor;
	mJointAxis->position(Ogre::Vector3(0, 0, 0)); //0
	mJointAxis->colour(1,0,0);
	mJointAxis->position(Ogre::Vector3(width, 0, 0)); //1
	mJointAxis->colour(1,0,0);
	mJointAxis->position(Ogre::Vector3(0, 0, width)); //2
	mJointAxis->colour(1,0,0);
	mJointAxis->position(Ogre::Vector3(width, 0, width)); //3
	mJointAxis->colour(1,0,0);
	mJointAxis->position(Ogre::Vector3(0, height*0.8, 0)); //4
	mJointAxis->colour(1,0,0);
	mJointAxis->position(Ogre::Vector3(width, height*0.8, 0)); //5
	mJointAxis->colour(1,0,0);
	mJointAxis->position(Ogre::Vector3(0, height*0.8, width)); //6
	mJointAxis->colour(1,0,0);
	mJointAxis->position(Ogre::Vector3(width, height*0.8, width)); //7

	mJointAxis->quad(0, 1, 3, 2); //away
	mJointAxis->quad(6, 7, 5, 4); //forward

	mJointAxis->quad(4,5,1,0);  //forwad
	mJointAxis->quad(7,6,2,3);  //away

	mJointAxis->quad(0,2,6,4);  //away	
	mJointAxis->quad(3, 1, 5, 7); //forward

	mJointAxis->end();
	mJointAxis->setCastShadows(false);
	mJointAxisNode = mNode->createChildSceneNode("AxisLine_" + mOwnerGO->GetName());
	mJointAxisNode->setInheritOrientation(false);
	Ogre::Quaternion parentOrientation = mOwnerGO->GetParent()->GetGlobalOrientation();
	mJointAxisNode->setOrientation(parentOrientation * mBoneConfig.mJointOrientation);
	mJointAxisNode->attachObject(mJointAxis);
}

void SGTGOCAnimatedCharacterBone::ScaleNode()
{
	float scale_factor = ((mMeshNode->_getDerivedScale().x + mMeshNode->_getDerivedScale().y + mMeshNode->_getDerivedScale().z) / 3);
	float capsule_height = (scale_factor*mBoneConfig.mBoneLength) - (scale_factor*mBoneConfig.mBoneLength * 0.4f);
	if (capsule_height <= 0.0f) capsule_height = 0.1f;
	Ogre::Vector3 offset = Ogre::Vector3(0.0f,mBoneConfig.mBoneLength*0.5f*scale_factor,0.0f);
	mOffsetNode->setScale(Ogre::Vector3(mBoneConfig.mRadius*scale_factor, capsule_height, mBoneConfig.mRadius*scale_factor));
	mOffsetNode->setPosition(offset);
}

void SGTGOCAnimatedCharacterBone::SetBone(Ogre::SceneNode *meshnode, SGTGOCAnimatedCharacter* ragdoll, sBoneActorBind &bone_config, bool controlBone)
{
	mMeshNode = meshnode;
	mBone = bone_config.mBone;
	mDebugAnimation = controlBone;
	mBone->setManuallyControlled(mDebugAnimation);
	mBoneConfig.mBoneName = mBone->getName();
	mBoneConfig.mNeedsJointOrientation = bone_config.mNeedsJointOrientation;
	mBoneConfig.mJointOrientation = bone_config.mJointOrientation;
	mBoneConfig.mRadius = bone_config.mBoneRadius;
	mBoneConfig.mBoneLength = bone_config.mBoneLength;
	mBoneConfig.mBoneOffset = bone_config.mOffset;
	mBoneConfig.mSwing1.mValue = bone_config.mSwing1.mValue;
	mBoneConfig.mSwing1.mDamping = bone_config.mSwing1.mDamping;
	mBoneConfig.mSwing1.mRestitution = bone_config.mSwing1.mRestitution;
	mBoneConfig.mSwing1.mSpring = bone_config.mSwing1.mSpring;

	mBoneConfig.mSwing2.mValue = bone_config.mSwing2.mValue;
	mBoneConfig.mSwing2.mDamping = bone_config.mSwing2.mDamping;
	mBoneConfig.mSwing2.mRestitution = bone_config.mSwing2.mRestitution;
	mBoneConfig.mSwing2.mSpring = bone_config.mSwing2.mSpring;

	mBoneConfig.mTwistMax.mValue = bone_config.mTwistMax.mValue;
	mBoneConfig.mTwistMax.mDamping = bone_config.mTwistMax.mDamping;
	mBoneConfig.mTwistMax.mRestitution = bone_config.mTwistMax.mRestitution;
	mBoneConfig.mTwistMax.mSpring = bone_config.mTwistMax.mSpring;

	mBoneConfig.mTwistMin.mValue = bone_config.mTwistMin.mValue;
	mBoneConfig.mTwistMin.mDamping = bone_config.mTwistMin.mDamping;
	mBoneConfig.mTwistMin.mRestitution = bone_config.mTwistMin.mRestitution;
	mBoneConfig.mTwistMin.mSpring = bone_config.mTwistMin.mSpring;

	if (bone_config.mParent)
	{
		mBoneConfig.mParentName = bone_config.mParent->mBone->getName();
	}
	else mBoneConfig.mParentName = "None";

	mBoneGlobalBindOrientation = mBone->_getDerivedOrientation();
	ScaleNode();
	float scale_factor = ((mMeshNode->_getDerivedScale().x + mMeshNode->_getDerivedScale().y + mMeshNode->_getDerivedScale().z) / 3);
	mGlobalBindPosition = mOwnerGO->GetGlobalPosition() - (mBoneConfig.mBoneOffset*scale_factor);

	mRagBoneRef = new RagBoneRef();
	mRagBoneRef->mBone = mBone;
	mRagBoneRef->mMeshNode = mMeshNode;
	mRagBoneRef->mGOCRagdoll = ragdoll;
	mRagBoneRef->mGOCRagdoll->mBoneObjects.push_back(mOwnerGO);
}

void SGTGOCAnimatedCharacterBone::SetOwner(SGTGameObject *go)
{
	mOwnerGO = go;
	mEntity->setUserObject(mOwnerGO);
}



//SGTGOCAnimatedCharacter

SGTGOCAnimatedCharacter::~SGTGOCAnimatedCharacter(void)
{
	std::list<SGTGameObject*>::iterator i = mBoneObjects.begin();
	while (i != mBoneObjects.end())
	{
		delete (*i);
		i = mBoneObjects.begin();
	}
	SGTMain::Instance().GetOgreSceneMgr()->destroySceneNode(mNode);
	if (mRagdoll)
	{
		SGTMessageSystem::Instance().QuitNewsgroup(this, "UPDATE_PER_FRAME");
		delete mRagdoll;
		mRagdoll = 0;
	}
}

SGTGOCAnimatedCharacter::SGTGOCAnimatedCharacter()
{
	mRagdoll = 0;
	mMovementState = 0;
	mSetControlToActorsTemp = false;
	mEditorMode = false;
}

SGTGOCAnimatedCharacter::SGTGOCAnimatedCharacter(Ogre::String meshname, Ogre::Vector3 scale)
{
	mSetControlToActorsTemp = false;
	mEditorMode = false;
	Create(meshname, scale);
}

void SGTGOCAnimatedCharacter::Create(Ogre::String meshname, Ogre::Vector3 scale)
{
	mMovementState = 0;
	mNode = SGTMain::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode(SGTSceneManager::Instance().RequestIDStr());
	mNode->scale(scale);
	if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", meshname))
	{
		Ogre::LogManager::getSingleton().logMessage("Error: Resource \"" + meshname + "\" does not exist. Loading dummy Resource...");
		meshname = "DummyMesh.mesh";
	}
	mRagdoll = new SGTRagdoll(SGTMain::Instance().GetOgreSceneMgr(), SGTMain::Instance().GetNxScene(), meshname, mNode);

	ResetMovementAnis();
	if (Ogre::ResourceGroupManager::getSingleton().resourceExists("General", mRagdoll->GetEntity()->getMesh()->getName() + ".anis"))
	{
		GetMovementAnis(mRagdoll->GetEntity()->getMesh()->getName() + ".anis");
	}
	SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
}

void SGTGOCAnimatedCharacter::Kill()
{
	mRagdoll->SetControlToActors();
}

void SGTGOCAnimatedCharacter::SerialiseBoneObjects(Ogre::String filename)
{
	std::vector<sBoneActorBindConfig> boneconfig;
	for (std::list<SGTGameObject*>::iterator i = mBoneObjects.begin(); i != mBoneObjects.end(); i++)
	{
		SGTGOCAnimatedCharacterBone *bone = (SGTGOCAnimatedCharacterBone*)(*i)->GetComponent("AnimatedCharacterBone");
		boneconfig.push_back(bone->GetBoneConfig());
	}
	mRagdoll->Serialise(boneconfig, filename);
}
void SGTGOCAnimatedCharacter::CreateBoneObjects()
{
	mBoneObjects.clear();
	std::map<Ogre::String, SGTGameObject*> bonemap;
	for (std::vector<sBoneActorBind>::iterator i = mRagdoll->mSkeleton.begin(); i != mRagdoll->mSkeleton.end(); i++)
	{
		SGTGameObject *go = new SGTGameObject();
		SGTGOCAnimatedCharacterBone *bone = new SGTGOCAnimatedCharacterBone();
		go->AddComponent(bone);
		go->SetGlobalPosition((*i).mActor->getGlobalPositionAsOgreVector3());
		//Ogre::Quaternion q = Ogre::Vector3(1,0,0).getRotationTo(Ogre::Vector3(0,1,0));
		go->SetGlobalOrientation((*i).mActor->getGlobalOrientationAsOgreQuaternion());// * q);
		bone->SetBone(mNode, this, (*i), true);
		//go->SetFreezePosition(true);
		(*i).mVisualBone = go;
		bonemap.insert(std::make_pair<Ogre::String, SGTGameObject*>((*i).mBone->getName(), go));
	}
	for (std::map<Ogre::String, SGTGameObject*>::iterator i = bonemap.begin(); i != bonemap.end(); i++)
	{
		Ogre::Bone *bone = mRagdoll->GetEntity()->getSkeleton()->getBone((*i).first);
		if (bone->getParent())
		{
			std::map<Ogre::String, SGTGameObject*>::iterator search = bonemap.find(bone->getParent()->getName());
			if (search != bonemap.end())
			{
				(*i).second->SetParent(search->second);
				((SGTGOCAnimatedCharacterBone*)(*i).second->GetComponent("AnimatedCharacterBone"))->CreateJointAxis();
			}
		}
	}
}

void SGTGOCAnimatedCharacter::ResetMovementAnis()
{
	mMovementAnimations.clear();
	mMovementAnimations.insert(std::make_pair<AnimationID, Ogre::String>(AnimationID::IDLE, "Idle"));
	mMovementAnimations.insert(std::make_pair<AnimationID, Ogre::String>(AnimationID::JUMP, "Jump"));
	mMovementAnimations.insert(std::make_pair<AnimationID, Ogre::String>(AnimationID::FORWARD, "Forward"));
	mMovementAnimations.insert(std::make_pair<AnimationID, Ogre::String>(AnimationID::BACKWARD, "Backward"));
	mMovementAnimations.insert(std::make_pair<AnimationID, Ogre::String>(AnimationID::LEFT, "Left"));
	mMovementAnimations.insert(std::make_pair<AnimationID, Ogre::String>(AnimationID::RIGHT, "Right"));
	mMovementAnimations.insert(std::make_pair<AnimationID, Ogre::String>(AnimationID::RUN, "Run"));
	mMovementAnimations.insert(std::make_pair<AnimationID, Ogre::String>(AnimationID::CROUCH, "Crouch"));
}
void SGTGOCAnimatedCharacter::GetMovementAnis(Ogre::String configfile)
{
	Ogre::ConfigFile cf;
	cf.loadFromResourceSystem(configfile, "General");
	Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
	Ogre::String secName, typeName, archName;
			
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
		for (Ogre::ConfigFile::SettingsMultiMap::iterator i = settings->begin(); i != settings->end(); i++)
		{
			if (i->first == "Jump") mMovementAnimations[AnimationID::JUMP] = i->second;
			if (i->first == "Forward") mMovementAnimations[AnimationID::FORWARD] = i->second;
			if (i->first == "Backward") mMovementAnimations[AnimationID::BACKWARD] = i->second;
			if (i->first == "Left") mMovementAnimations[AnimationID::LEFT] = i->second;
			if (i->first == "Right") mMovementAnimations[AnimationID::RIGHT] = i->second;
			if (i->first == "Run") mMovementAnimations[AnimationID::RUN] = i->second;
			if (i->first == "Crouch") mMovementAnimations[AnimationID::CROUCH] = i->second;
		}
	}
}

void SGTGOCAnimatedCharacter::ReceiveMessage(SGTMsg &msg)
{
	if (msg.mNewsgroup == "UPDATE_PER_FRAME")
	{
		mRagdoll->Update(msg.mData.GetFloat("TIME"));
	}
}

void SGTGOCAnimatedCharacter::ReceiveObjectMessage(Ogre::SharedPtr<SGTObjectMsg> msg)
{
	if (msg->mName == "UpdateCharacterMovementState")
	{
		mMovementState = msg->mData.GetInt("CharacterMovementState");

		if (mMovementState & SGTCharacterMovement::JUMP)
		{
			mRagdoll->SetAnimationState(mMovementAnimations[AnimationID::JUMP]);
		}
		else if (mMovementState & SGTCharacterMovement::FORWARD && !(mMovementState & SGTCharacterMovement::BACKWARD))
		{
			mRagdoll->SetAnimationState(mMovementAnimations[AnimationID::FORWARD]);
		}
		else if (mMovementState & SGTCharacterMovement::BACKWARD && !(mMovementState & SGTCharacterMovement::FORWARD))
		{
			mRagdoll->SetAnimationState(mMovementAnimations[AnimationID::BACKWARD]);
		}
		else if (mMovementState & SGTCharacterMovement::LEFT)
		{
			mRagdoll->SetAnimationState(mMovementAnimations[AnimationID::LEFT]);
		}
		else if (mMovementState & SGTCharacterMovement::RIGHT)
		{
			mRagdoll->SetAnimationState(mMovementAnimations[AnimationID::RIGHT]);
		}
		else mRagdoll->SetAnimationState(mMovementAnimations[AnimationID::IDLE]);
	}
}

void SGTGOCAnimatedCharacter::SetOwner(SGTGameObject *go)
{
	mRagdoll->GetEntity()->setUserObject(go);
	UpdatePosition(go->GetGlobalPosition());
	UpdateOrientation(go->GetGlobalOrientation());
	mRagdoll->Update(0);
	if (mSetControlToActorsTemp)
	{
		mRagdoll->SetControlToActors();
		mSetControlToActorsTemp = false;
	}
	else if (mEditorMode && mAnimationStateStr == "DEBUG") CreateBoneObjects();
}

void SGTGOCAnimatedCharacter::CreateFromDataMap(SGTDataMap *parameters)
{
	Ogre::String meshname = parameters->GetOgreString("MeshName");
	mAnimationStateStr = parameters->GetOgreString("AnimState");
	bool shadowcaster = parameters->GetBool("ShadowCaster");
	mSetControlToActorsTemp = parameters->GetBool("Ragdoll");
	Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
	scale = parameters->GetOgreVec3("Scale");
	Create(meshname, scale);
	if (mAnimationStateStr != "") mRagdoll->SetAnimationState(mAnimationStateStr);
	mRagdoll->GetEntity()->setCastShadows(shadowcaster);
	mRagdoll->ResetBones();
	mEditorMode = true;
}
void SGTGOCAnimatedCharacter::GetParameters(SGTDataMap *parameters)
{
	parameters->AddOgreString("MeshName", mRagdoll->GetEntity()->getMesh()->getName());
	parameters->AddOgreString("AnimState", mAnimationStateStr);
	parameters->AddBool("Ragdoll", false);
	parameters->AddBool("ShadowCaster", mRagdoll->GetEntity()->getCastShadows());
}
void SGTGOCAnimatedCharacter::GetDefaultParameters(SGTDataMap *parameters)
{
	parameters->AddOgreString("MeshName", "");
	parameters->AddOgreString("AnimState", "");
	parameters->AddBool("Ragdoll", false);
	parameters->AddBool("ShadowCaster", true);
}

void SGTGOCAnimatedCharacter::Save(SGTSaveSystem& mgr)
{
	mgr.SaveAtom("Ogre::String", (void*)&mRagdoll->GetEntity()->getMesh()->getName(), "MeshName");
	mgr.SaveAtom("Ogre::Vector3", &mOwnerGO->GetGlobalScale(), "Scale");
	mgr.SaveAtom("Ogre::String", &mAnimationStateStr, "AnimState");
	bool ragdoll = mRagdoll->ControlledByActors();
	mgr.SaveAtom("bool", &ragdoll, "Ragdoll");
	bool shadow = mRagdoll->GetEntity()->getCastShadows();
	mgr.SaveAtom("bool", &shadow, "ShadowCaster");
}
void SGTGOCAnimatedCharacter::Load(SGTLoadSystem& mgr)
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
	if (animstate != "") mRagdoll->SetAnimationState(animstate);
	mRagdoll->GetEntity()->setCastShadows(shadowcaster);
	if (ragdoll) mRagdoll->SetControlToActors();
}

void SGTGOCAnimatedCharacter::AttachToGO(SGTGameObject *go)
{
	go->RemoveComponent(GetFamilyID());
	go->AddComponent(this);
}
void SGTGOCAnimatedCharacterBone::AttachToGO(SGTGameObject *go)
{
	go->RemoveComponent(GetFamilyID());
	go->AddComponent(this);
}