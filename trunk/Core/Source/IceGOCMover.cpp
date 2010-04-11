
#include "IceGOCMover.h"
#include "IcegameObject.h"
#include "IceMessageSystem.h"
#include "IceGOCView.h"
#include "IceSceneManager.h"


namespace Ice
{

	GOCAnimKey::GOCAnimKey()
	{
		mTotalStayTime = 0.0f;
		mTimeSinceLastKey = 1.0f;
	}
	GOCAnimKey::~GOCAnimKey()
	{
		mMover->notifyKeyDelete(this);
	}
	void GOCAnimKey::CreateFromDataMap(DataMap *parameters)
	{
		mTotalStayTime = parameters->GetFloat("TotalStayTime");
		mTimeSinceLastKey = parameters->GetFloat("TimeSinceLastKey");
	}
	void GOCAnimKey::GetParameters(DataMap *parameters)
	{
		parameters->AddFloat("TotalStayTime", mTotalStayTime);
		parameters->AddFloat("TimeSinceLastKey", mTimeSinceLastKey);
	}
	void GOCAnimKey::GetDefaultParameters(DataMap *parameters)
	{
		parameters->AddFloat("TotalStayTime", 0);
		parameters->AddFloat("TimeSinceLastKey", 1.0f);
	}
	void GOCAnimKey::AttachToGO(GameObject *go)
	{
		go->RemoveComponent(GetFamilyID());
		go->AddComponent(this);
	}
	void GOCAnimKey::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("float", &mTotalStayTime, "TotalStayTime");
		mgr.SaveAtom("float", &mTimeSinceLastKey, "TimeSinceLastKey");
	}
	void GOCAnimKey::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("float", &mTotalStayTime);
		mgr.LoadAtom("float",  &mTimeSinceLastKey);
	}
	void GOCAnimKey::UpdatePosition(Ogre::Vector3 position)
	{
		mMover->UpdateKeys();
	}

	GOCMover::GOCMover()
	{
		mMover = this;
		mMoving = false;
		MessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
		mSplineObject = Ice::Main::Instance().GetOgreSceneMgr()->createManualObject("Spline_" + SceneManager::Instance().RequestIDStr());
		Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->attachObject(mSplineObject);
	}
	GOCMover::~GOCMover()
	{
		Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->detachObject(mSplineObject);
		Ice::Main::Instance().GetOgreSceneMgr()->destroyManualObject(mSplineObject);
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
		SetKeyIgnoreParent(true);
		mfLastPos=0;
	}

	void GOCMover::ReceiveMessage( Msg &msg )
	{
		if (msg.mNewsgroup == "UPDATE_PER_FRAME" && mMoving)
		{
			float time = msg.mData.GetFloat("TIME");
			mfLastPos+=time;
			SetOwnerPosition(mSpline.Sample(mfLastPos));
			if(mfLastPos>mSpline.GetLength())
			{
				mMoving = false;
				mfLastPos=0;
				SetKeyIgnoreParent(false);
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

	void GOCMover::UpdateKeys()
	{
		mSplineObject->clear();
		if(mAnimKeys.size()<1)
			return;
		int keyCounter = 0;
		std::vector<Ogre::Vector3> vKeys;
		vKeys.push_back(mOwnerGO->GetGlobalPosition());
		for(int iKey=0; iKey<(int)mAnimKeys.size(); iKey++)
		{
			mAnimKeys[iKey]->SetName("Key_" + Ogre::StringConverter::toString(++keyCounter));
			vKeys.push_back(mAnimKeys[iKey]->GetGlobalPosition());
		}
		mSpline.SetPoints(vKeys);

		mSplineObject->begin("WPLine", Ogre::RenderOperation::OT_LINE_STRIP);
		mSplineObject->position(mOwnerGO->GetGlobalPosition());
		for(double fPos=0.1; fPos<=mSpline.GetLength(); fPos+=0.1)
		{
			mSplineObject->position(mSpline.Sample(fPos));
		}
		mSplineObject->end();
		mSplineObject->setCastShadows(false);
	}

	void GOCMover::notifyKeyDelete(GOCAnimKey *key)
	{
		for (auto i = mAnimKeys.begin(); i != mAnimKeys.end(); i++)
		{
			if ((*i) == key->GetOwner())
			{
				mAnimKeys.erase(i);
				break;
			}
		}
		if (mAnimKeys.empty()) mIsLastKey = true;
		else mAnimKeys.back()->GetComponent<GOCAnimKey>()->mIsLastKey = true;

		UpdateKeys();
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