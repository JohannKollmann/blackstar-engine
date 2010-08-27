
#include "IceGOCAnimatedCharacter.h"
#include "IceSceneManager.h"
#include "IceMain.h"
#include "IceGOCCharacterController.h"
#include "IceUtils.h"

namespace Ice
{

GOCAnimatedCharacterBone::GOCAnimatedCharacterBone(void)
{
	mOffsetNode = nullptr;
	mEntity = nullptr;
	mMeshNode = nullptr;
	mJointAxisNode = nullptr;
	mJointAxis = nullptr;
	mBone = nullptr;
	mRagBoneRef = nullptr;
}
void GOCAnimatedCharacterBone::Init()
{
	if (GetNode()) return;
	mOffsetNode = GetNode()->createChildSceneNode(Ogre::Vector3(0,0,0), Ogre::Quaternion());
	mEntity = Main::Instance().GetOgreSceneMgr()->createEntity(Ogre::StringConverter::toString(SceneManager::Instance().RequestID()), "capsule.mesh");
	mEntity->setCastShadows(false);
	mEntity->setUserAny(Ogre::Any(mOwnerGO));
	mOffsetNode->attachObject(mEntity);
	mDebugAnimation = true;
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
GOCAnimatedCharacterBone::~GOCAnimatedCharacterBone(void)
{
	if (mEntity) Main::Instance().GetOgreSceneMgr()->destroyEntity(mEntity);
	if (mJointAxis) Main::Instance().GetOgreSceneMgr()->destroyManualObject(mJointAxis);
	if (mJointAxisNode) Main::Instance().GetOgreSceneMgr()->destroySceneNode(mJointAxisNode);
	if (mOffsetNode) Main::Instance().GetOgreSceneMgr()->destroySceneNode(mOffsetNode);
	if (mBone) mBone->setManuallyControlled(false);
	if (mRagBoneRef) mRagBoneRef->mGOCRagdoll->mBoneObjects.remove(mOwnerGO);
}

void GOCAnimatedCharacterBone::SetParameters(DataMap *parameters)
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

void GOCAnimatedCharacterBone::GetParameters(DataMap *parameters)
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
void GOCAnimatedCharacterBone::GetDefaultParameters(DataMap *parameters)
{
}
void GOCAnimatedCharacterBone::UpdatePosition(Ogre::Vector3 position)
{
	if (GetNode())
	{
		GetNode()->setPosition(position);
		if (mBone)
		{
			float scale_factor = ((mMeshNode->_getDerivedScale().x + mMeshNode->_getDerivedScale().y + mMeshNode->_getDerivedScale().z) / 3);
			if (mOwnerGO->GetUpdatingFromParent())
			{
				//if root bone moves, don't update BoneOffset
				GameObject *superparent = mOwnerGO->GetParent();
				while (superparent->GetParent()) superparent = superparent->GetParent(); 
				if (superparent->GetTransformingChildren()) mGlobalBindPosition = position - (mBoneConfig.mBoneOffset*scale_factor);
				else
				{
					float scale_factor = ((mMeshNode->_getDerivedScale().x + mMeshNode->_getDerivedScale().y + mMeshNode->_getDerivedScale().z) / 3);
					mBoneConfig.mBoneOffset = position - mGlobalBindPosition;
					mBoneConfig.mBoneOffset = mBoneConfig.mBoneOffset / scale_factor;
				}
			}
			else if (mOwnerGO->GetParent() || !mOwnerGO->GetTransformingChildren())
			{
				mBoneConfig.mBoneOffset = position - mGlobalBindPosition;
				mBoneConfig.mBoneOffset = mBoneConfig.mBoneOffset / scale_factor;
			}
		}
	}
}

void GOCAnimatedCharacterBone::UpdateScale(Ogre::Vector3 scale)
{
	/*if (GetNode())
	{
		mBoneConfig.mRadius = mBoneConfig.mBoneLength * ((scale.x + scale.z) * 0.5f);
		mBoneConfig.mBoneLength = mBoneConfig.mBoneLength*scale.y;
		ScaleNode();
	}*/
}

void GOCAnimatedCharacterBone::UpdateOrientation(Ogre::Quaternion orientation)
{
	if (GetNode())
	{
		GetNode()->setOrientation(orientation);
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

bool GOCAnimatedCharacterBone::GetTestAnimation()
{
	if (!mDebugAnimation) return false;
	if (mOwnerGO->GetParent())
	{
		if (mOwnerGO->GetParent()->GetComponent("AnimatedCharacterBone"))
		{
			return ((GOCAnimatedCharacterBone*)(mOwnerGO->GetParent()->GetComponent("AnimatedCharacterBone")))->GetTestAnimation();
		}
	}
	return mDebugAnimation;
}

void GOCAnimatedCharacterBone::CreateJointAxis()
{
	float scale_factor = ((mMeshNode->_getDerivedScale().x + mMeshNode->_getDerivedScale().y + mMeshNode->_getDerivedScale().z) / 3);
	mJointAxis = Main::Instance().GetOgreSceneMgr()->createManualObject("AxisLine_" + SceneManager::Instance().RequestIDStr());
	mJointAxis->begin("BlueLine", Ogre::RenderOperation::OT_TRIANGLE_LIST);
	float width = 0.05f;
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
	mJointAxisNode = GetNode()->createChildSceneNode("AxisLine_" + SceneManager::Instance().RequestIDStr());
	mJointAxisNode->setInheritOrientation(false);
	Ogre::Quaternion parentOrientation = mOwnerGO->GetParent()->GetGlobalOrientation();
	mJointAxisNode->setOrientation(parentOrientation * mBoneConfig.mJointOrientation);
	mJointAxisNode->attachObject(mJointAxis);
}

void GOCAnimatedCharacterBone::ScaleNode()
{
	float scale_factor = ((mMeshNode->_getDerivedScale().x + mMeshNode->_getDerivedScale().y + mMeshNode->_getDerivedScale().z) / 3);
	float capsule_height = (scale_factor*mBoneConfig.mBoneLength) - (scale_factor*mBoneConfig.mBoneLength * 0.4f);
	if (capsule_height <= 0.0f) capsule_height = 0.1f;
	Ogre::Vector3 offset = Ogre::Vector3(0.0f,mBoneConfig.mBoneLength*0.5f*scale_factor,0.0f);
	mOffsetNode->setScale(Ogre::Vector3(mBoneConfig.mRadius*scale_factor, capsule_height, mBoneConfig.mRadius*scale_factor));
	mOffsetNode->setPosition(offset);
}

void GOCAnimatedCharacterBone::SetBone(Ogre::SceneNode *meshnode, GOCAnimatedCharacter* ragdoll, OgrePhysX::sBoneActorBind &bone_config, bool controlBone)
{
	Init();
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

	mRagBoneRef = ICE_NEW RagBoneRef();
	mRagBoneRef->mBone = mBone;
	mRagBoneRef->mMeshNode = mMeshNode;
	mRagBoneRef->mGOCRagdoll = ragdoll;
	mRagBoneRef->mGOCRagdoll->mBoneObjects.push_back(mOwnerGO);
}

void GOCAnimatedCharacterBone::SetOwner(GameObject *go)
{
	mOwnerGO = go;
	if (mEntity) mEntity->setUserAny(Ogre::Any(mOwnerGO));
}


//GOCAnimatedCharacter

GOCAnimatedCharacter::~GOCAnimatedCharacter(void)
{
	_clear();
}

void GOCAnimatedCharacter::_clear()
{
	std::list<GameObject*>::iterator i = mBoneObjects.begin();
	while (i != mBoneObjects.end())
	{
		ICE_DELETE (*i);
		i = mBoneObjects.begin();
	}
	if (mEntity) Main::Instance().GetOgreSceneMgr()->destroyEntity(mEntity);
	if (mRagdoll)
	{
		Main::Instance().GetPhysXScene()->destroyRagdoll(mRagdoll);
		mRagdoll = 0;
	}
}

GOCAnimatedCharacter::GOCAnimatedCharacter()
{
	mRagdoll = nullptr;
	mMovementState = 0;
	mEntity = nullptr;
	mSetControlToActorsTemp = false;
	mEditorMode = false;
	mAnimationState = nullptr;
	MessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
}

GOCAnimatedCharacter::GOCAnimatedCharacter(Ogre::String meshname, Ogre::Vector3 scale)
{
	mSetControlToActorsTemp = false;
	mEditorMode = false;
	mAnimationState = 0;
	Create(meshname, scale);
	MessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
}

void GOCAnimatedCharacter::Create(Ogre::String meshname, Ogre::Vector3 scale)
{
	mMovementState = 0;
	GetNode()->scale(scale);
	if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", meshname))
	{
		Ogre::LogManager::getSingleton().logMessage("Error: Resource \"" + meshname + "\" does not exist. Loading dummy Resource...");
		meshname = "DummyMesh.mesh";
	}
	mEntity = Main::Instance().GetOgreSceneMgr()->createEntity(SceneManager::Instance().RequestIDStr(), meshname);
	GetNode()->attachObject(mEntity);
	if (!mEntity->hasSkeleton()) return;

	mRagdoll = Main::Instance().GetPhysXScene()->createRagdoll(mEntity, GetNode(), CollisionGroups::BONE);

	ResetMovementAnis();
	if (Ogre::ResourceGroupManager::getSingleton().resourceExists("General", mEntity->getMesh()->getName() + ".anis"))
	{
		GetMovementAnis(mEntity->getMesh()->getName() + ".anis");
	}
}

void GOCAnimatedCharacter::SetAnimationState(Ogre::String statename)
{
	if (mAnimationState) mAnimationState->setEnabled(false);
	mAnimationStateStr = "";
	mAnimationState = 0;
	if (mEntity->getAllAnimationStates()->hasAnimationState(statename))
	{
		mAnimationStateStr = statename;
		mAnimationState = mEntity->getAnimationState(statename);
	}
	if (mAnimationState) mAnimationState->setEnabled(true);
}

void GOCAnimatedCharacter::Kill()
{
	mRagdoll->setControlToActors();
	Msg msg;
	if (mOwnerGO) mOwnerGO->SendMessage(msg);
}

void GOCAnimatedCharacter::SerialiseBoneObjects(Ogre::String filename)
{
	std::vector<OgrePhysX::sBoneActorBindConfig> boneconfig;
	for (std::list<GameObject*>::iterator i = mBoneObjects.begin(); i != mBoneObjects.end(); i++)
	{
		GOCAnimatedCharacterBone *bone = (GOCAnimatedCharacterBone*)(*i)->GetComponent("AnimatedCharacterBone");
		boneconfig.push_back(bone->GetBoneConfig());
	}
	mRagdoll->serialise(boneconfig, filename);
}
void GOCAnimatedCharacter::CreateBoneObjects()
{
	mBoneObjects.clear();
	std::map<Ogre::String, GameObject*> bonemap;
	std::vector<OgrePhysX::sBoneActorBind> skeleton = mRagdoll->getSkeleton();
	for (std::vector<OgrePhysX::sBoneActorBind>::iterator i = skeleton.begin(); i != skeleton.end(); i++)
	{
		GameObject *go = ICE_NEW GameObject();
		GOCAnimatedCharacterBone *bone = ICE_NEW GOCAnimatedCharacterBone();
		go->AddComponent(GOComponentPtr(bone));
		go->SetGlobalPosition((*i).mActor->getGlobalPosition());
		//Ogre::Quaternion q = Ogre::Vector3(1,0,0).getRotationTo(Ogre::Vector3(0,1,0));
		go->SetGlobalOrientation((*i).mActor->getGlobalOrientation());// * q);
		bone->SetBone(GetNode(), this, (*i), true);
		//go->SetFreezePosition(true);
		bonemap.insert(std::make_pair<Ogre::String, GameObject*>((*i).mBone->getName(), go));
	}
	for (std::map<Ogre::String, GameObject*>::iterator i = bonemap.begin(); i != bonemap.end(); i++)
	{
		Ogre::Bone *bone = mEntity->getSkeleton()->getBone((*i).first);
		if (bone->getParent())
		{
			std::map<Ogre::String, GameObject*>::iterator search = bonemap.find(bone->getParent()->getName());
			if (search != bonemap.end())
			{
				(*i).second->SetParent(search->second);
				((GOCAnimatedCharacterBone*)(*i).second->GetComponent("AnimatedCharacterBone"))->CreateJointAxis();
			}
		}
	}
}

void GOCAnimatedCharacter::ResetMovementAnis()
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
void GOCAnimatedCharacter::GetMovementAnis(Ogre::String configfile)
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

void GOCAnimatedCharacter::ReceiveMessage(Msg &msg)
{
	if (msg.type == "UPDATE_PER_FRAME")
	{
		if (mAnimationState) mAnimationState->addTime(msg.params.GetFloat("TIME"));
	}
}

void GOCAnimatedCharacter::ReceiveObjectMessage(Msg &msg)
{
	if (msg.type == "ENTER_FPS_MODE") mEntity->setVisible(false);
	if (msg.type == "LEAVE_FPS_MODE") mEntity->setVisible(true);
	if (msg.type == "UpdateCharacterMovementState")
	{
		mMovementState = msg.params.GetInt("CharacterMovementState");

		if (mMovementState & CharacterMovement::JUMP)
		{
			SetAnimationState(mMovementAnimations[AnimationID::JUMP]);
		}
		else if (mMovementState & CharacterMovement::FORWARD && !(mMovementState & CharacterMovement::BACKWARD))
		{
			SetAnimationState(mMovementAnimations[AnimationID::FORWARD]);
		}
		else if (mMovementState & CharacterMovement::BACKWARD && !(mMovementState & CharacterMovement::FORWARD))
		{
			SetAnimationState(mMovementAnimations[AnimationID::BACKWARD]);
		}
		else if (mMovementState & CharacterMovement::LEFT)
		{
			SetAnimationState(mMovementAnimations[AnimationID::LEFT]);
		}
		else if (mMovementState & CharacterMovement::RIGHT)
		{
			SetAnimationState(mMovementAnimations[AnimationID::RIGHT]);
		}
		else SetAnimationState(mMovementAnimations[AnimationID::IDLE]);
	}
}

void GOCAnimatedCharacter::SetOwner(GameObject *go)
{
	mOwnerGO = go;
	_clear();
	if (!mOwnerGO) return;
	if (mMeshName == "") return;
	Create(mMeshName, mOwnerGO->GetGlobalScale());
	if (!mEntity) return;
	mEntity->setCastShadows(mShadowCaster);
	mEntity->setUserAny(Ogre::Any(mOwnerGO));
	UpdatePosition(go->GetGlobalPosition());
	UpdateOrientation(go->GetGlobalOrientation());
	if (!mRagdoll) return;

	mRagdoll->setActorUserData(mOwnerGO);
	mRagdoll->sync();
	if (mSetControlToActorsTemp)
	{
		mRagdoll->setControlToActors();
		mSetControlToActorsTemp = false;
	}
	else if (mEditorMode && mAnimationStateStr == "DEBUG") CreateBoneObjects();
}

void GOCAnimatedCharacter::SetParameters(DataMap *parameters)
{
	mMeshName = parameters->GetOgreString("MeshName");
	mAnimationStateStr = parameters->GetOgreString("AnimState");
	mShadowCaster = parameters->GetBool("ShadowCaster");
	mSetControlToActorsTemp = parameters->GetBool("Ragdoll");
	Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
	scale = parameters->GetOgreVec3("Scale");
	if (mOwnerGO)
	{
		SetOwner(mOwnerGO);
		if (!mRagdoll) return;
		if (mAnimationStateStr != "DEBUG") SetAnimationState(mAnimationStateStr);
		mRagdoll->resetBones();
		mEditorMode = true;
	}
}
void GOCAnimatedCharacter::GetParameters(DataMap *parameters)
{
	parameters->AddOgreString("MeshName", mEntity->getMesh()->getName());
	parameters->AddOgreString("AnimState", mAnimationStateStr);
	parameters->AddBool("Ragdoll", false);
	parameters->AddBool("ShadowCaster", mEntity->getCastShadows());
}
void GOCAnimatedCharacter::GetDefaultParameters(DataMap *parameters)
{
	parameters->AddOgreString("MeshName", "");
	parameters->AddOgreString("AnimState", "");
	parameters->AddBool("Ragdoll", false);
	parameters->AddBool("ShadowCaster", true);
}

void GOCAnimatedCharacter::Save(LoadSave::SaveSystem& mgr)
{
	mgr.SaveAtom("Ogre::String", (void*)&mEntity->getMesh()->getName(), "MeshName");
	mgr.SaveAtom("Ogre::String", &mAnimationStateStr, "AnimState");
	bool ragdoll = mRagdoll->isControlledByActors();
	mgr.SaveAtom("bool", &ragdoll, "Ragdoll");
	bool shadow = mEntity->getCastShadows();
	mgr.SaveAtom("bool", &shadow, "ShadowCaster");
}
void GOCAnimatedCharacter::Load(LoadSave::LoadSystem& mgr)
{
	Ogre::String meshname;
	Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
	Ogre::String animstate;
	bool shadowcaster = true;
	bool ragdoll = false;
	mgr.LoadAtom("Ogre::String", &mMeshName);
	mgr.LoadAtom("Ogre::String", &mAnimationStateStr);
	mgr.LoadAtom("bool", &mSetControlToActorsTemp);
	mgr.LoadAtom("bool", &mShadowCaster);
}

};