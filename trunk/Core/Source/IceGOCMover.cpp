
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
		mgr.SaveObject(mMover, "Mover");
	}
	void GOCAnimKey::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("float",  &mTimeToNextKey);
		mMover = (GOCMover*)mgr.LoadObject();
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
		mLookAtLine = nullptr;
		mNormalLookAtLine = nullptr;
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
		_destroyLookAtLine();
		_destroyNormalLookAtLine();
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
		mStaticMode = parameters->GetBool("Static Mode");
		mKeyCallback = parameters->GetOgreString("Key Callback");
		Init();
	}
	void GOCMover::GetParameters(DataMap *parameters)
	{
		parameters->AddFloat("TimeToNextKey", mTimeToNextKey);
		parameters->AddBool("Closed", mIsClosed);
		parameters->AddBool("Static Mode", mStaticMode);
		parameters->AddOgreString("Key Callback", mKeyCallback);
	}
	void GOCMover::GetDefaultParameters(DataMap *parameters)
	{
		parameters->AddFloat("TimeToNextKey", 1.0f);
		parameters->AddBool("Closed", false);
		parameters->AddBool("Static Mode", false);
		parameters->AddOgreString("Key Callback", "");
	}

	void GOCMover::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("float", &mTimeToNextKey, "TimeToNextKey");
		mgr.SaveAtom("bool", &mIsClosed, "Closed");
		mgr.SaveAtom("bool", &mStaticMode, "StaticMode");
		mgr.SaveAtom("Ogre::String", &mKeyCallback, "KeyCallback");
		mgr.SaveAtom("std::vector<Saveable*>", &mAnimKeys, "AnimKeys");
	}
	void GOCMover::Load(LoadSave::LoadSystem& mgr)
	{
		Init();
		mgr.LoadAtom("float", &mTimeToNextKey);
		mgr.LoadAtom("bool", &mIsClosed);
		mgr.LoadAtom("bool", &mStaticMode);
		mgr.LoadAtom("Ogre::String", &mKeyCallback);
		mgr.LoadAtom("std::vector<Saveable*>", &mAnimKeys);
	}

	void GOCMover::UpdatePosition(Ogre::Vector3 position)
	{
		_updateLookAtLine();
		_updateNormalLookAtLine();
		UpdateKeys();
	}

	void GOCMover::Trigger()
	{
		mMoving = true;
		SetKeyIgnoreParent(true);
		mfLastPos=0;
	}

	void GOCMover::ReceiveMessage( Msg &msg )
	{
		if (msg.type == "UPDATE_PER_FRAME")
		{
			if (mLookAtLine) _updateLookAtLine();
			if (mNormalLookAtLine) _updateNormalLookAtLine();
			if (mLookAtObject)
			{
				Ogre::Vector3 upVector = Ogre::Vector3(0,1,0);
				if (mNormalLookAtObject)
				{
					upVector = mNormalLookAtObject->GetGlobalPosition() - GetOwner()->GetGlobalPosition();
					upVector.normalise();
				}
				Ogre::Vector3 lookAtDir = mLookAtObject->GetGlobalPosition() - GetOwner()->GetGlobalPosition();
				lookAtDir.normalise();
				Ogre::Vector3 xAxis = lookAtDir.crossProduct(upVector);
				Ogre::Vector3 yAxis = lookAtDir.crossProduct(xAxis);

				Ogre::Quaternion q(xAxis, yAxis, lookAtDir);
				SetOwnerOrientation(q);
			}
			if (mMoving)
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
		if (!mOwnerGO) return;
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

	void GOCMover::ShowEditorVisual(bool show)
	{
		if (show && !mEditorVisual)
		{
			mEditorVisual = Main::Instance().GetOgreSceneMgr()->createEntity(GetEditorVisualMeshName());
			mEditorVisual->setUserAny(Ogre::Any(mOwnerGO));
			GetNode()->attachObject(mEditorVisual);
		}
		else if (!show && mEditorVisual)
		{
			GetNode()->detachObject(mEditorVisual);
			Main::Instance().GetOgreSceneMgr()->destroyEntity(mEditorVisual);
			mEditorVisual = nullptr;
		}

		if (show)
		{
			_updateLookAtLine();
			_updateNormalLookAtLine();
		}
		else
		{
			_destroyLookAtLine();
			_destroyNormalLookAtLine();
		}

		for (auto i = mAnimKeys.begin(); i != mAnimKeys.end(); i++)
			(*i)->ShowEditorVisuals(show);

	}
	void GOCMover::_destroyNormalLookAtLine()
	{
		if (mNormalLookAtLine)
		{
			Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->detachObject(mNormalLookAtLine);
			Main::Instance().GetOgreSceneMgr()->destroyManualObject(mNormalLookAtLine);
			mNormalLookAtLine = nullptr;
		}
	}
	void GOCMover::_destroyLookAtLine()
	{
		if (mLookAtLine)
		{
			Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->detachObject(mLookAtLine);
			Main::Instance().GetOgreSceneMgr()->destroyManualObject(mLookAtLine);
			mLookAtLine = nullptr;
		}
	}
	void GOCMover::_updateLookAtLine()
	{
		if (mLookAtObject)
		{
			if (!mLookAtLine)
			{
				mLookAtLine = Main::Instance().GetOgreSceneMgr()->createManualObject();
				Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->attachObject(mLookAtLine);
			}
			mLookAtLine->clear();
			mLookAtLine->begin("WPLine", Ogre::RenderOperation::OT_LINE_STRIP);
			mLookAtLine->position(GetOwner()->GetGlobalPosition());
			mLookAtLine->position(mLookAtObject->GetGlobalPosition());
			mLookAtLine->end();
			mLookAtLine->setCastShadows(false);
		}
	}
	void GOCMover::_updateNormalLookAtLine()
	{
		if (mNormalLookAtObject)
		{
			if (!mNormalLookAtLine)
			{
				mNormalLookAtLine = Main::Instance().GetOgreSceneMgr()->createManualObject();
				Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->attachObject(mNormalLookAtLine);
			}
			mNormalLookAtLine->clear();
			mNormalLookAtLine->begin("WPLine", Ogre::RenderOperation::OT_LINE_STRIP);
			mNormalLookAtLine->position(GetOwner()->GetGlobalPosition());
			mNormalLookAtLine->position(mNormalLookAtObject->GetGlobalPosition());
			mNormalLookAtLine->end();
			mNormalLookAtLine->setCastShadows(false);
		}
	}
	void GOCMover::onDeleteSubject(Utils::DeleteListener* subject)
	{
		if (subject == mLookAtObject)
		{
			_destroyLookAtLine();
			mLookAtObject = nullptr;
		}
		else if (subject == mNormalLookAtObject)
		{
			_destroyNormalLookAtLine();
			mNormalLookAtObject = nullptr;
		}
	}

	void GOCMover::SetLookAtObject(GameObject *target)
	{
		if (mLookAtObject) mLookAtObject->removeListener(this);
		mLookAtObject = target;
		mLookAtObject->addDeleteListener(this);
		_updateLookAtLine();
	}
	void GOCMover::SetNormalLookAtObject(GameObject *target)
	{
		if (mNormalLookAtObject) mNormalLookAtObject->removeListener(this);
		mNormalLookAtObject = target;
		mNormalLookAtObject->addDeleteListener(this);
		_updateNormalLookAtLine();
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