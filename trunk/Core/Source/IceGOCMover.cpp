
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
		MessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
		mSplineObject = Ice::Main::Instance().GetOgreSceneMgr()->createManualObject("Spline_" + SceneManager::Instance().RequestIDStr());
		Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->attachObject(mSplineObject);
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
		/*mCurrentTrack.clear();
		for (auto i = mAnimKeys.begin(); i != mAnimKeys.end(); i++)
		{
			(*i)->SetIgnoreParent(true);
			mCurrentTrack.push_back(*i);
		}*/
		mfLastPos=0;
	}

	void GOCMover::ReceiveMessage( Msg &msg )
	{
		if (msg.mNewsgroup == "UPDATE_PER_FRAME" && mMoving)
		{
			/*if (mCurrentTrack.empty())
			{
				mMoving = false;
				SetKeyIgnoreParent(false);
			}
			else
			{*/
				float time = msg.mData.GetFloat("TIME");
				/*GameObject *target = mCurrentTrack.front();
				Ogre::Vector3 dir = target->GetGlobalPosition() - mOwnerGO->GetGlobalPosition();
				dir.normalise();
				dir *= time;
				SetOwnerPosition(mOwnerGO->GetGlobalPosition() + dir);
				if (mOwnerGO->GetGlobalPosition().distance(target->GetGlobalPosition()) < 0.1f) mCurrentTrack.pop_front();*/
				
				SetOwnerPosition(mOwnerGO->GetGlobalPosition() + mSpline.Sample(mfLastPos+time) - mSpline.Sample(mfLastPos));
				mfLastPos+=time;
				if(mfLastPos>mSpline.GetLength())
				{
					mMoving = false;
					mfLastPos=0;
					SetKeyIgnoreParent(false);
				}
			//}
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
		if(mAnimKeys.size()<1)
			return;
		std::vector<Ogre::Vector3> vKeys;
		vKeys.push_back(mOwnerGO->GetGlobalPosition());
		for(int iKey=0; iKey<(int)mAnimKeys.size(); iKey++)
			vKeys.push_back(mAnimKeys[iKey]->GetGlobalPosition());
		mSpline.SetPoints(vKeys);

		Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->detachObject(mSplineObject);
		Ice::Main::Instance().GetOgreSceneMgr()->destroyManualObject(mSplineObject);

		mSplineObject = Ice::Main::Instance().GetOgreSceneMgr()->createManualObject("Spline_" + SceneManager::Instance().RequestIDStr());
		

		mSplineObject->clear();

		mSplineObject->begin("WPLine", Ogre::RenderOperation::OT_LINE_STRIP);
		for(double fPos=0; fPos<mSpline.GetLength(); fPos+=0.1)
		{
			mSplineObject->position(mSpline.Sample(fPos));
		}
		mSplineObject->end();
		mSplineObject->setCastShadows(false);

		Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->attachObject(mSplineObject);
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