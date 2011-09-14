
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
		destroyAllProcesses();
		GameObjectPtr owner = mOwnerGO.lock();
		/*if (owner.get())
		{
			if (GOCScriptMessageCallback *c = owner->GetComponent<GOCScriptMessageCallback>())
			{
				Msg msg; msg.typeID = GlobalMessageIDs::REPARSE_SCRIPTS_PRE;
				c->ReceiveMessage(msg);		//HACK - damit script objekt message listener gelöscht werden
			}
		}*/
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
			Main::Instance().GetPhysXScene()->destroyRenderableBinding(mRagdoll);
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

		mRagdoll = Main::Instance().GetPhysXScene()->createRagdollBinding(mEntity, GetNode());
	}

	void GOCAnimatedCharacter::Kill()
	{
		mRagdoll->setControlToActors();
		Msg msg;
		BroadcastObjectMessage(msg);
	}

	void GOCAnimatedCharacter::SerialiseBoneObjects(Ogre::String filename)
	{
		/*std::vector<OgrePhysX::sBoneActorBindConfig> boneconfig;
		for (std::list<GameObject*>::iterator i = mBoneObjects.begin(); i != mBoneObjects.end(); i++)
		{
			GOCAnimatedCharacterBone *bone = (GOCAnimatedCharacterBone*)(*i)->GetComponent("AnimatedCharacterBone");
			boneconfig.push_back(bone->GetBoneConfig());
		}
		mRagdoll->serialise(boneconfig, filename);*/
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
			destroyAllProcesses();

		if (msg.typeID == ObjectMessageIDs::ENTER_FPS_MODE) mEntity->setVisible(false);
		if (msg.typeID == ObjectMessageIDs::LEAVE_FPS_MODE) mEntity->setVisible(true);

		if (msg.typeID == GlobalMessageIDs::PHYSICS_BEGIN) 
			if (!mRagdoll->isControlledByActors()) mRagdoll->updateBoneActors();
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
			registerProcess(process->GetProcessID());
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

};