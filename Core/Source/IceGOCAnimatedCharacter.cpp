
#include "IceGOCAnimatedCharacter.h"
#include "IceSceneManager.h"
#include "IceMain.h"
#include "IceGOCCharacterController.h"
#include "IceUtils.h"
#include "IceProcessNodeManager.h"
#include "IceGOCScript.h"
#include "IceObjectMessageIDs.h"

namespace Ice
{

	GOCAnimatedCharacter::~GOCAnimatedCharacter(void)
	{
		_clear();
	}

	void GOCAnimatedCharacter::_clear()
	{
		_destroyCreatedProcesses();
		GameObjectPtr owner = mOwnerGO.lock();
		if (owner.get())
		{
			if (GOCScriptMessageCallback *c = owner->GetComponent<GOCScriptMessageCallback>())
			{
				Msg msg; msg.typeID = GlobalMessageIDs::REPARSE_SCRIPTS_PRE;
				c->ReceiveMessage(msg);		//HACK - damit script objekt message listener gelöscht werden
			}
		}
		std::list<GameObject*>::iterator i = mBoneObjects.begin();
		while (i != mBoneObjects.end())
		{
			ICE_DELETE (*i);
			i = mBoneObjects.begin();
		}
		if (mEntity)
		{
			Main::Instance().GetOgreSceneMgr()->destroyEntity(mEntity);
			mEntity = nullptr;
		}
		if (mRagdoll)
		{
			Main::Instance().GetPhysXScene()->destroyRagdoll(mRagdoll);
			mRagdoll = nullptr;
		}
	}

	GOCAnimatedCharacter::GOCAnimatedCharacter()
	{
		mRagdoll = nullptr;
		mEntity = nullptr;
		mSetControlToActorsTemp = false;
		mEditorMode = false;
		JoinNewsgroup(GlobalMessageIDs::REPARSE_SCRIPTS_PRE);
	}

	GOCAnimatedCharacter::GOCAnimatedCharacter(Ogre::String meshname, Ogre::Vector3 scale)
	{
		mSetControlToActorsTemp = false;
		mEditorMode = false;
		Create(meshname, scale);
		JoinNewsgroup(GlobalMessageIDs::REPARSE_SCRIPTS_PRE);
	}

	void GOCAnimatedCharacter::Create(Ogre::String meshname, Ogre::Vector3 scale)
	{
		GetNode()->setScale(scale);
		if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", meshname))
		{
			Log::Instance().LogMessage("Error: Resource \"" + meshname + "\" does not exist. Loading dummy Resource...");
			meshname = "DummyMesh.mesh";
		}
		mEntity = Main::Instance().GetOgreSceneMgr()->createEntity(SceneManager::Instance().RequestIDStr(), meshname);
		if (!mEntity->hasSkeleton())
		{
			IceWarning(meshname + "has no skeleton!")
			return;
		}
		mEntity->getSkeleton()->setBlendMode(Ogre::SkeletonAnimationBlendMode::ANIMBLEND_CUMULATIVE);
		GetNode()->attachObject(mEntity);

		mRagdoll = Main::Instance().GetPhysXScene()->createRagdoll(mEntity, GetNode(), CollisionGroups::BONE);
	}

	void GOCAnimatedCharacter::Kill()
	{
		mRagdoll->setControlToActors();
		Msg msg;
		BroadcastObjectMessage(msg);
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
		/*mBoneObjects.clear();
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
		}*/
	}

	void GOCAnimatedCharacter::ReceiveMessage(Msg &msg)
	{
		if (mOwnerGO.expired()) return;
		if (msg.typeID == GlobalMessageIDs::REPARSE_SCRIPTS_PRE)
			_destroyCreatedProcesses();

		if (msg.typeID == ObjectMessageIDs::ENTER_FPS_MODE) mEntity->setVisible(false);
		if (msg.typeID == ObjectMessageIDs::LEAVE_FPS_MODE) mEntity->setVisible(true);
	}

	void GOCAnimatedCharacter::SetOwner(std::weak_ptr<GameObject> go)
	{
		_clear();
		mOwnerGO = go;
		GameObjectPtr owner = mOwnerGO.lock();
		if (!owner.get()) return;
		if (mMeshName == "") return;
		Create(mMeshName, owner->GetGlobalScale());
		if (!mEntity) return;
		mEntity->setCastShadows(mShadowCaster);
		mEntity->setUserAny(Ogre::Any(owner.get()));
		UpdatePosition(owner->GetGlobalPosition());
		UpdateOrientation(owner->GetGlobalOrientation());
		if (!mRagdoll) return;

		mRagdoll->setActorUserData(owner.get());
		mRagdoll->sync();
		if (mSetControlToActorsTemp)
		{
			mRagdoll->setControlToActors();
			mSetControlToActorsTemp = false;
		}

		//else if (mEditorMode) CreateBoneObjects();
	}

	void GOCAnimatedCharacter::SetParameters(DataMap *parameters)
	{
		mMeshName = parameters->GetOgreString("MeshName");
		mShadowCaster = parameters->GetBool("ShadowCaster");
		mSetControlToActorsTemp = parameters->GetBool("Ragdoll");
		Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
		scale = parameters->GetOgreVec3("Scale");
		GameObjectPtr owner = mOwnerGO.lock();
		if (owner.get())
		{
			SetOwner(mOwnerGO);
			if (!mRagdoll) return;
			mRagdoll->resetBones();
			mEditorMode = true;
		}
	}
	void GOCAnimatedCharacter::GetParameters(DataMap *parameters)
	{
		parameters->AddOgreString("MeshName", mEntity->getMesh()->getName());
		parameters->AddBool("Ragdoll", false);
		parameters->AddBool("ShadowCaster", mEntity->getCastShadows());
	}
	void GOCAnimatedCharacter::GetDefaultParameters(DataMap *parameters)
	{
		parameters->AddOgreString("MeshName", "");
		parameters->AddBool("Ragdoll", false);
		parameters->AddBool("ShadowCaster", true);
	}

	void GOCAnimatedCharacter::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("Ogre::String", (void*)&mEntity->getMesh()->getName(), "MeshName");
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
		mgr.LoadAtom("bool", &mSetControlToActorsTemp);
		mgr.LoadAtom("bool", &mShadowCaster);
	}

	void GOCAnimatedCharacter::_destroyCreatedProcesses()
	{
		ITERATE(i, mCreatedProcesses)
			ProcessNodeManager::Instance().RemoveProcessNode(*i);
		mCreatedProcesses.clear();
	}

	std::vector<ScriptParam> GOCAnimatedCharacter::AnimProcess_Create(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		Ogre::String animName = vParams[0].getString();
		if (mEntity->getSkeleton()->hasAnimation(animName))
		{
			Ogre::AnimationState *ogreAnimState = mEntity->getAnimationState(animName);
		
			bool looped = false;
			if (vParams.size() > 1 && vParams[1].getType() == ScriptParam::PARM_TYPE_BOOL) looped = vParams[1].getBool();
			ogreAnimState->setLoop(looped);

			float blendTime = 0.2f;
			if (vParams.size() > 2 && vParams[2].getType() == ScriptParam::PARM_TYPE_FLOAT) blendTime = vParams[2].getFloat();
			float timeScale = 1.0f;
			if (vParams.size() > 3 && vParams[3].getType() == ScriptParam::PARM_TYPE_FLOAT) timeScale = vParams[3].getFloat();

			std::shared_ptr<PlayAnimationProcess> process = ProcessNodeManager::Instance().CreatePlayAnimationProcess(ogreAnimState);
			mCreatedProcesses.push_back(process->GetProcessID());
			process->SetLooped(looped);
			process->SetInBlendDuration(blendTime);
			process->SetOutBlendDuration(blendTime);
			process->SetTimeScale(timeScale);
			for (unsigned int i = 4; vParams.size() > i+1; i+=2)
			{
				if (vParams[i].getType() == ScriptParam::PARM_TYPE_FLOAT && vParams[i+1].getType() == ScriptParam::PARM_TYPE_FUNCTION)
					process->AddCallback(vParams[i].getFloat(), vParams[i+1]);
			}
			out.push_back(ScriptParam(process->GetProcessID()));
			
		}
		else IceWarning("Animation does not exist: " + animName)
		return out;
	}



	//==========================================================================================================================
	//==========================================================================================================================

	//GOCAnimatedCharacterBone

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
		mEntity->setUserAny(Ogre::Any(GetOwner().get()));
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
		/*if (mEntity) Main::Instance().GetOgreSceneMgr()->destroyEntity(mEntity);
		if (mJointAxis) Main::Instance().GetOgreSceneMgr()->destroyManualObject(mJointAxis);
		if (mJointAxisNode) Main::Instance().GetOgreSceneMgr()->destroySceneNode(mJointAxisNode);
		if (mOffsetNode) Main::Instance().GetOgreSceneMgr()->destroySceneNode(mOffsetNode);
		if (mBone) mBone->setManuallyControlled(false);
		if (mRagBoneRef) mRagBoneRef->mGOCRagdoll->mBoneObjects.remove(mOwnerGO);*/
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
		mGlobalBindPosition = mOwnerGO.lock()->GetGlobalPosition() - (mBoneConfig.mBoneOffset*scale_factor);
		if (mBoneConfig.mParentName != "None")
		{
			mBoneConfig.mJointOrientation = mOwnerGO.lock()->GetGlobalOrientation().Inverse() * mBoneConfig.mJointOrientation;
			CreateJointAxis();
		}
	}

	void GOCAnimatedCharacterBone::GetParameters(DataMap *parameters)
	{
		/*parameters->AddOgreString("mParentName", mBoneConfig.mParentName);
		float scale_factor = ((mMeshNode->_getDerivedScale().x + mMeshNode->_getDerivedScale().y + mMeshNode->_getDerivedScale().z) / 3);
		parameters->AddFloat("BoneLength", mBoneConfig.mBoneLength * scale_factor);
		parameters->AddOgreVec3("BoneOffset", mBoneConfig.mBoneOffset * scale_factor);
		parameters->AddFloat("Radius", mBoneConfig.mRadius * scale_factor);
		Ogre::Quaternion parentOrientation = Ogre::Quaternion();
		if (GetOwner()->GetLinkedObjectByName("Parent").get()) parentOrientation = GetOwner()->GetLinkedObjectByName("Parent")->GetGlobalOrientation();
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

		parameters->AddBool("TestAnimation", mDebugAnimation);*/
	}
	void GOCAnimatedCharacterBone::GetDefaultParameters(DataMap *parameters)
	{
	}
	void GOCAnimatedCharacterBone::UpdatePosition(Ogre::Vector3 position)
	{
		/*if (GetNode())
		{
			GetNode()->setPosition(position);
			if (mBone)
			{
				float scale_factor = ((mMeshNode->_getDerivedScale().x + mMeshNode->_getDerivedScale().y + mMeshNode->_getDerivedScale().z) / 3);
				if (GetOwner()->GetUpdatingFromParent())
				{
					//if root bone moves, don't update BoneOffset
					GameObjectPtr superparent = GetOwner()->GetLinkedObjectByName("Parent");
					while (superparent.get() && superparent->GetLinkedObjectByName("Parent").get()) superparent = superparent->GetParent(); 
					if (superparent->GetTransformingLinkedObjects()) mGlobalBindPosition = position - (mBoneConfig.mBoneOffset*scale_factor);
					else
					{
						float scale_factor = ((mMeshNode->_getDerivedScale().x + mMeshNode->_getDerivedScale().y + mMeshNode->_getDerivedScale().z) / 3);
						mBoneConfig.mBoneOffset = position - mGlobalBindPosition;
						mBoneConfig.mBoneOffset = mBoneConfig.mBoneOffset / scale_factor;
					}
				}
				else if (GetOwner()->GetLinkedObjectByName("Parent")|| !GetOwner()->GetTransformingLinkedObjects())
				{
					mBoneConfig.mBoneOffset = position - mGlobalBindPosition;
					mBoneConfig.mBoneOffset = mBoneConfig.mBoneOffset / scale_factor;
				}
			}
		}*/
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
		/*if (!mDebugAnimation) return false;
		if (GetOwner()->GetLinkedObjectByName("Parent").get())
		{
			if (GetOwner()->GetLinkedObjectByName("Parent")->GetComponent("AnimatedCharacterBone"))
			{
				return ((GOCAnimatedCharacterBone*)(mOwnerGO.lock()->GetLinkedObjectByName("Parent")->GetComponent("AnimatedCharacterBone")))->GetTestAnimation();
			}
		}*/
		return mDebugAnimation;
	}

	void GOCAnimatedCharacterBone::CreateJointAxis()
	{
		/*float scale_factor = ((mMeshNode->_getDerivedScale().x + mMeshNode->_getDerivedScale().y + mMeshNode->_getDerivedScale().z) / 3);
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
		Ogre::Quaternion parentOrientation = mOwnerGO.lock()->GetLinkedObjectByName("Parent")->GetGlobalOrientation();
		mJointAxisNode->setOrientation(parentOrientation * mBoneConfig.mJointOrientation);
		mJointAxisNode->attachObject(mJointAxis);*/
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
		/*Init();
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
		mGlobalBindPosition = mOwnerGO.lock()->GetGlobalPosition() - (mBoneConfig.mBoneOffset*scale_factor);

		mRagBoneRef = ICE_NEW RagBoneRef();
		mRagBoneRef->mBone = mBone;
		mRagBoneRef->mMeshNode = mMeshNode;
		mRagBoneRef->mGOCRagdoll = ragdoll;
		mRagBoneRef->mGOCRagdoll->mBoneObjects.push_back(mOwnerGO);*/
	}

	void GOCAnimatedCharacterBone::SetOwner(std::weak_ptr<GameObject> go)
	{
		mOwnerGO = go;
		if (mEntity) mEntity->setUserAny(Ogre::Any(GetOwner().get()));
	}

};