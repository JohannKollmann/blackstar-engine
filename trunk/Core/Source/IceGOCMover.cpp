
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
		go->SetParent(mMover->GetOwner());
		return go;
	}

	GOCAnimKey::GOCAnimKey(AnimKey *pred)
	{
		mPredecessor = pred;
		mTimeToNextKey = 1;
	}
	GOCAnimKey::~GOCAnimKey()
	{
		if (mMover) mMover->notifyKeyDelete(this);
	}
	void GOCAnimKey::SetMover(GOCMover *mover)
	{
		mMover = mover;
		mMover->InsertKey(mOwnerGO, mPredecessor);
	}

	void GOCAnimKey::SetParameters(DataMap *parameters)
	{
		mTimeToNextKey = parameters->GetValue<float>("TimeToNextKey", 1);
	}
	void GOCAnimKey::GetParameters(DataMap *parameters)
	{
		parameters->AddFloat("TimeToNextKey", mTimeToNextKey);
	}
	void GOCAnimKey::GetDefaultParameters(DataMap *parameters)
	{
		parameters->AddFloat("TimeToNextKey", 1.0f);
	}
	void GOCAnimKey::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("float", &mTimeToNextKey, "TimeToNextKey");
	}
	void GOCAnimKey::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("float",  &mTimeToNextKey);
	}
	void GOCAnimKey::UpdatePosition(Ogre::Vector3 position)
	{
		if (mMover) mMover->UpdateKeys();
	}

	GOCMover::GOCMover()
	{
		mMover = this;
		mMoving = false;
		mSplineObject = nullptr;
		mIsClosed = false;
		mLookAtObject = nullptr;
		mNormalLookAtObject = nullptr;
	}
	void GOCMover::Init()
	{
		if (mSplineObject) return;
		MessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
		mSplineObject = Ice::Main::Instance().GetOgreSceneMgr()->createManualObject("Spline_" + SceneManager::Instance().RequestIDStr());
		Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->attachObject(mSplineObject);
	}
	GOCMover::~GOCMover()
	{
		if (mSplineObject)
		{
			Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->detachObject(mSplineObject);
			Ice::Main::Instance().GetOgreSceneMgr()->destroyManualObject(mSplineObject);
		}
	}
	void GOCMover::SetParameters(DataMap *parameters)
	{
		mTimeToNextKey = parameters->GetValue<float>("TimeToNextKey", 1);
		mIsClosed = parameters->GetBool("Closed");
		mKeyCallbackScript = parameters->GetOgreString("Callback Script");
		Init();
	}
	void GOCMover::GetParameters(DataMap *parameters)
	{
		parameters->AddFloat("TimeToNextKey", mTimeToNextKey);
		parameters->AddBool("Closed", mIsClosed);
		parameters->AddOgreString("Callback Script", mKeyCallbackScript);
	}
	void GOCMover::GetDefaultParameters(DataMap *parameters)
	{
		parameters->AddFloat("TimeToNextKey", 1.0f);
		parameters->AddBool("Closed", false);
		parameters->AddOgreString("Callback Script", "");
	}

	void GOCMover::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("float", &mTimeToNextKey, "TimeToNextKey");
		mgr.SaveAtom("bool", &mIsClosed, "Closed");
		mgr.SaveAtom("Ogre::String", &mKeyCallbackScript, "KeyCallbackScript");
		mgr.SaveAtom("std::vector<Saveable*>", &mAnimKeys, "AnimKeys");
	}
	void GOCMover::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("float", &mTimeToNextKey);
		mgr.LoadAtom("bool", &mIsClosed);
		mgr.LoadAtom("Ogre::String", &mKeyCallbackScript);
		mgr.LoadAtom("std::vector<Saveable*>", &mAnimKeys);
		Init();
	}

	void GOCMover::Trigger()
	{
		mMoving = true;
		SetKeyIgnoreParent(true);
		mfLastPos=0;
	}

	void GOCMover::ReceiveMessage( Msg &msg )
	{
		if (msg.type == "UPDATE_PER_FRAME" && mMoving)
		{
			float time = msg.params.GetFloat("TIME");
			SetOwnerPosition(mSpline.Sample(mfLastPos));
			mfLastPos+=time;
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
		
		std::vector<Ogre::Vector4> vKeys;
		//float fTime = 0;
		vKeys.push_back(Ogre::Vector4(mOwnerGO->GetGlobalPosition().x, mOwnerGO->GetGlobalPosition().y, mOwnerGO->GetGlobalPosition().z, GetTimeToNextKey()));
		//fTime += GetTimeToNextKey();
		for(int iKey=0; iKey<(int)mAnimKeys.size(); iKey++)
		{
			mAnimKeys[iKey]->SetName("Key_" + Ogre::StringConverter::toString(++keyCounter));
			Ogre::Vector3 keyPos = mAnimKeys[iKey]->GetGlobalPosition();
			vKeys.push_back(Ogre::Vector4(keyPos.x, keyPos.y, keyPos.z, mAnimKeys[iKey]->GetComponent<AnimKey>()->GetTimeToNextKey()));
			//fTime += mAnimKeys[iKey]->GetComponent<AnimKey>()->GetTimeToNextKey();
		}
		mSpline.SetPoints(vKeys, mIsClosed);

		mSplineObject->begin("WPLine", Ogre::RenderOperation::OT_LINE_STRIP);
		for(double fPos=0.0; fPos<=mSpline.GetLength()-0.01; fPos+=0.1)
			mSplineObject->position(mSpline.Sample(fPos));

		mSplineObject->position(mSpline.Sample(mSpline.GetLength()-0.01));
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