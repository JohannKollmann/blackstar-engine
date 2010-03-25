
#include "IceGOCMover.h"
#include "IcegameObject.h"
#include "IceMessageSystem.h"
#include "IceGOCView.h"

namespace Ice
{

	GOCAnimKey::GOCAnimKey()
	{
	}
	GOCAnimKey::~GOCAnimKey()
	{
	}
	void GOCAnimKey::CreateFromDataMap(DataMap *parameters)
	{
		mTimeSinceLastKey = parameters->GetFloat("TimeSinceLastKey");
	}
	void GOCAnimKey::GetParameters(DataMap *parameters)
	{
		parameters->AddFloat("TimeSinceLastKey", mTimeSinceLastKey);
	}
	void GOCAnimKey::GetDefaultParameters(DataMap *parameters)
	{
		parameters->AddFloat("TimeSinceLastKey", 1.0f);
	}
	void GOCAnimKey::AttachToGO(GameObject *go)
	{
		go->RemoveComponent(GetFamilyID());
		go->AddComponent(this);
	}
	void GOCAnimKey::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("float", &mTimeSinceLastKey, "TimeSinceLastKey");
	}
	void GOCAnimKey::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("float",  &mTimeSinceLastKey);
	}


	GOCMover::GOCMover()
	{
		mMover = this;
		MessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
	}
	GOCMover::~GOCMover()
	{
	}
	void GOCMover::CreateFromDataMap(DataMap *parameters)
	{
		mKeyCallbackScript = parameters->GetOgreString("Callback Script");
	}
	void GOCMover::GetParameters(DataMap *parameters)
	{
		parameters->AddOgreString("Callback Script", mKeyCallbackScript);
	}
	void GOCMover::GetDefaultParameters(DataMap *parameters)
	{
		parameters->AddOgreString("Callback Script", "");
	}
	void GOCMover::AttachToGO(GameObject *go)
	{
		go->RemoveComponent(GetFamilyID());
		go->AddComponent(this);
	}

	void GOCMover::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("Ogre::String", &mKeyCallbackScript, "KeyCallbackScript");
		mgr.SaveAtom("std::vector<Saveable*>", &mAnimKeys, "AnimKeys");
	}
	void GOCMover::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("Ogre::String", &mKeyCallbackScript);
		mgr.LoadAtom("std::vector<Saveable*>", &mAnimKeys);
	}

	GameObject* GOCMover::CreateAnimKey()
	{
		GameObject *go = new GameObject();
		go->SetLoadSaveByParent(false);
		go->SetParent(mOwnerGO);
		GOCAnimKey *key = new GOCAnimKey();
		key->SetMover(this);
		go->AddComponent(key);
		go->AddComponent(new MeshDebugRenderable("Editor_AnimKey.mesh"));
		mAnimKeys.push_back(go);
		return go;
	}

	void GOCMover::Trigger()
	{
		mMoving = true;
		mCurrentTrack.clear();
		for (auto i = mAnimKeys.begin(); i != mAnimKeys.end(); i++)
		{
			(*i)->SetIgnoreParent(true);
			mCurrentTrack.push_back(*i);
		}
	}

	void GOCMover::ReceiveMessage( Msg &msg )
	{
		if (msg.mNewsgroup == "UPDATE_PER_FRAME" && mMoving)
		{
			if (mCurrentTrack.empty())
			{
				mMoving = false;
				SetKeyIgnoreParent(false);
			}
			else
			{
				float time = msg.mData.GetFloat("TIME");
				GameObject *target = mCurrentTrack.front();
				Ogre::Vector3 dir = target->GetGlobalPosition() - mOwnerGO->GetGlobalPosition();
				dir.normalise();
				dir *= time;
				SetOwnerPosition(mOwnerGO->GetGlobalPosition() + dir);
				if (mOwnerGO->GetGlobalPosition().distance(target->GetGlobalPosition()) < 0.1f) mCurrentTrack.pop_front();
			}
		}
	}

	void GOCMover::SetKeyIgnoreParent(bool ignore)
	{
		for (auto i = mAnimKeys.begin(); i != mAnimKeys.end(); i++)
		{
			(*i)->SetIgnoreParent(ignore);
		}
	}


	GameObject* AnimKey::CreateSuccessor()
	{
		if (!mIsLastKey) return nullptr;
		mIsLastKey = false;
		return mMover->CreateAnimKey();
	}

	void AnimKey::SetMover( GOCMover *mover )
	{
		mMover = mover;
	}

}