
#include "IceGOCMover.h"
#include "IcegameObject.h"
#include "IceMessageSystem.h"
#include "IceGOCView.h"
#include "IceSceneManager.h"


namespace Ice
{

	GameObject* AnimKey::CreateSuccessor()
	{
		if (!mMover)
		{
			Ogre::LogManager::getSingleton().logMessage("Error in AnimKey::CreateSuccessor: mMover = 0 !");
			return 0;
		}
		GameObject *go = new GameObject();
		go->SetLoadSaveByParent(false);
		GOCAnimKey *key = new GOCAnimKey(this);
		go->AddComponent(key);
		go->AddComponent(new MeshDebugRenderable("Editor_AnimKey.mesh"));
		go->SetParent(mMover->GetOwner());
		return go;
	}

	GOCAnimKey::GOCAnimKey(AnimKey *pred)
	{
		mPredecessor = pred;
		mTotalStayTime = 0.0f;
		mTimeSinceLastKey = 1.0f;
	}
	GOCAnimKey::~GOCAnimKey()
	{
		mMover->notifyKeyDelete(this);
	}
	void GOCAnimKey::SetMover(GOCMover *mover)
	{
		mMover = mover;
		mMover->InsertKey(mOwnerGO, mPredecessor);
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
		if (mMover) mMover->UpdateKeys();
	}

	void* GOCAnimKey::GetUserData()
	{
		//Hack: This is called before the key is de deleted and recreated with the new properties.
		return mPredecessor;
	}

	void GOCAnimKey::InjectUserData( void* data )
	{
		mPredecessor = (AnimKey*)data;
		mOwnerGO->GetParent()->GetComponent<GOCMover>()->OnAddChild(mOwnerGO);	//Hack
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
				if ((i+1) != mAnimKeys.end())
				{
					if (i == mAnimKeys.begin())
						(*(i+1))->GetComponent<GOCAnimKey>()->mPredecessor = this;
					else (*(i+1))->GetComponent<GOCAnimKey>()->mPredecessor = (*(i-1))->GetComponent<AnimKey>();
				}
				mAnimKeys.erase(i);
				break;
			}
		}

		UpdateKeys();
	}

	void GOCMover::InsertKey(GameObject *key, AnimKey *pred)
	{
		if (pred == this)
		{
			if (!mAnimKeys.empty())
				(*(mAnimKeys.begin()))->GetComponent<GOCAnimKey>()->mPredecessor = key->GetComponent<AnimKey>();
			mAnimKeys.insert(mAnimKeys.begin(), key);
		}
		else
		{
			for (auto i = mAnimKeys.begin(); i != mAnimKeys.end(); i++)
			{
				if ((*i)->GetComponent<AnimKey>() == pred)
				{
					if ((i+1) != mAnimKeys.end())
					{
						(*(i+1))->GetComponent<GOCAnimKey>()->mPredecessor = key->GetComponent<AnimKey>();
					}
					mAnimKeys.insert((i+1), key);
					break;
				}
			}
		}
		UpdateKeys();
	}

	void GOCMover::OnAddChild(GameObject *child)
	{
		GOCAnimKey *key = child->GetComponent<GOCAnimKey>();
		if (key) key->SetMover(this);
	}

}