
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

	void GOCAnimKey::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("float", &mTimeToNextKey, "TimeToNextKey");
		mgr.SaveObject(dynamic_cast<LoadSave::Saveable*>(mPredecessor), "Mover", true);	//hack
		mgr.SaveObject(mMover, "Mover");
	}
	void GOCAnimKey::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("float",  &mTimeToNextKey);
		mPredecessor = dynamic_cast<AnimKey*>(mgr.LoadObject());
		mMover = (GOCMover*)mgr.LoadObject();
	}
	void GOCAnimKey::UpdatePosition(Ogre::Vector3 position)
	{
		if (mMover) mMover->UpdateKeys();
	}

	GOCMover::GOCMover()
	{
		//mMover = this;
		mMoving = false;
		mPerformingMovement = false;
		mPaused=false;
		mSplineObject = nullptr;
		mIsClosed = false;
		mIgnoreOrientation=false;
		mLookAtObject = nullptr;
		mNormalLookAtObject = nullptr;
		mLookAtLine = nullptr;
		mNormalLookAtLine = nullptr;
		mLastKeyIndex = -1;
	}
	void GOCMover::Init()
	{
		if (mSplineObject) return;
		MessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
		mSplineObject = Ice::Main::Instance().GetOgreSceneMgr()->createManualObject("Spline_" + SceneManager::Instance().RequestIDStr());
		Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->attachObject(mSplineObject);
		mSplineObject->setVisible(SceneManager::Instance().GetShowEditorVisuals());
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
	void GOCMover::OnSetParameters()
	{
		Init();
	}

	void GOCMover::SetOwner(GameObject *owner)
	{
		mOwnerGO = owner;
		UpdatePosition(mOwnerGO->GetGlobalPosition());
		UpdateOrientation(mOwnerGO->GetGlobalOrientation());
	}

	void GOCMover::Save(LoadSave::SaveSystem& mgr)
	{
		//mgr.SaveAtom("float", &mTimeToNextKey, "TimeToNextKey");
		mgr.SaveAtom("bool", &mIsClosed, "Closed");
		mgr.SaveAtom("bool", &mStaticMode, "StaticMode");
		mgr.SaveAtom("bool", &mIgnoreOrientation, "IgnoreOrientation");
		//mgr.SaveAtom("std::vector<Saveable*>", &mAnimKeys, "AnimKeys");
		mgr.SaveObject(mLookAtObject, "mLookAtObject", true);
		mgr.SaveObject(mNormalLookAtObject, "mNormalLookAtObject", true);
	}
	void GOCMover::Load(LoadSave::LoadSystem& mgr)
	{
		Init();
		//mgr.LoadAtom("float", &mTimeToNextKey);
		mgr.LoadAtom("bool", &mIsClosed);
		mgr.LoadAtom("bool", &mStaticMode);
		mgr.LoadAtom("bool", &mIgnoreOrientation);
		//mgr.LoadAtom("std::vector<Saveable*>", &mAnimKeys);
		mLookAtObject=(Ice::GameObject*)mgr.LoadObject();
		if (mLookAtObject) SetLookAtObject(mLookAtObject);
		mNormalLookAtObject=(Ice::GameObject*)mgr.LoadObject();
		if (mNormalLookAtObject) SetNormalLookAtObject(mNormalLookAtObject);
	}

	void GOCMover::UpdatePosition(Ogre::Vector3 position)
	{
		_updateLookAtLine();
		_updateNormalLookAtLine();
		UpdateKeys();
	}

	void GOCMover::Trigger()
	{
		if(mPaused)
			mPaused=false;
		if(mMoving)
			return;
		mMoving = true;
		//SetKeyIgnoreParent(true);
		mfLastPos=0;
		Ice::Msg msg; msg.type = "MOVER_START";
		mOwnerGO->SendInstantMessage(msg);
		if(mAnimKeys.size()<1)
		{
			Ice::Msg msg; msg.type = "MOVER_END";
			mOwnerGO->SendMessage(msg);
			mMoving = false;
			//SetKeyIgnoreParent(false);
		}
	}

	void GOCMover::PrepareMovement(bool prepare)
	{
		mPerformingMovement = prepare;
		SetKeyIgnoreParent(prepare);
		if (mLookAtObject) mLookAtObject->SetIgnoreParent(prepare);
		if (mNormalLookAtObject) mNormalLookAtObject->SetIgnoreParent(prepare);
	}

	void GOCMover::Pause()
	{
		mPaused=true;
	}

	void GOCMover::Stop()
	{
		SetKeyIgnoreParent(false);
		mMoving = false;
		mfLastPos=0;
		Ice::Msg msg; msg.type = "MOVER_END";
		mOwnerGO->SendInstantMessage(msg);
		mLastKeyIndex = -1;
	}

	void GOCMover::ReceiveMessage( Msg &msg )
	{
		if (msg.type == "UPDATE_PER_FRAME" && mOwnerGO)
		{
			if (mLookAtLine) _updateLookAtLine();
			if (mNormalLookAtLine) _updateNormalLookAtLine();
			Ogre::Vector3 upVector = Ogre::Vector3(0,1,0);
			if (mNormalLookAtObject)
			{
				upVector = mNormalLookAtObject->GetGlobalPosition() - GetOwner()->GetGlobalPosition();
				upVector.normalise();
			}
			if (mLookAtObject)
			{
				Ogre::Vector3 lookAtDir = mLookAtObject->GetGlobalPosition() - GetOwner()->GetGlobalPosition();
				lookAtDir.normalise();

				PrepareMovement(true);
				SetOwnerOrientation(Utils::ZDirToQuat(lookAtDir, upVector));
				PrepareMovement(false);
			}
			if (mMoving && !mPaused)
			{
				float time = msg.params.GetFloat("TIME");

				Ogre::Vector3 oldPos = mOwnerGO->GetGlobalPosition();
				
				if(mStaticMode)
				{
					//search which sector we are in
					int iSearchPos=mKeyTiming.size()/2;
					int iStep=iSearchPos+1;
			
					while(iStep>1)
					{
						iStep>>=1;
						double fSample=mKeyTiming[iSearchPos];
						if(mfLastPos<fSample)
							iSearchPos-=iStep;
						else
							iSearchPos+=iStep;
					}
					if(mfLastPos < mKeyTiming[iSearchPos] && iSearchPos)
						iSearchPos--;
					//iSearchPos holds the index we've passed last

				}
				else
				{
					int keyIndex = 0;
					PrepareMovement(true);
					SetOwnerPosition(mSpline.Sample(mfLastPos, &keyIndex));
					PrepareMovement(false);
					keyIndex--;
					if (keyIndex != mLastKeyIndex)
					{
						Ice::Msg msg; msg.type = "MOVER_KEY";
						IceAssert(keyIndex < (int)mAnimKeys.size());
						msg.params.AddOgreString("Keyname", mAnimKeys[keyIndex]->GetName());
						mOwnerGO->SendInstantMessage(msg);
						mLastKeyIndex = keyIndex;
					}
				}

				if (!(mLookAtObject || mIgnoreOrientation))		//look towards current target direction
				{
					Ogre::Vector3 lookAtDir = (mOwnerGO->GetGlobalPosition() - oldPos).normalisedCopy();
					PrepareMovement(true);
					SetOwnerOrientation(Utils::ZDirToQuat(lookAtDir, upVector));
					PrepareMovement(false);
				}

				mfLastPos+=time;
				if(mfLastPos>=(mStaticMode ? mKeyTiming[mKeyTiming.size()-1] : mSpline.GetLength()))
				{
					//SetKeyIgnoreParent(false);
					mMoving = false;
					mfLastPos=0;
					Ice::Msg msg; msg.type = "MOVER_END";
					mOwnerGO->SendMessage(msg);
					mLastKeyIndex = -1;
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
		if (mPerformingMovement) return;
		/*if (mAnimKeys.size() > 0)
		{
			PrepareMovement(true);
			SetOwnerPosition(mAnimKeys[0]->GetGlobalPosition());
			SetOwnerOrientation(mAnimKeys[0]->GetGlobalOrientation());
			PrepareMovement(false);
		}*/
		//if (mMoving && !mOwnerGO->GetUpdatingFromParent()) return;
		mSplineObject->clear();
		if(mAnimKeys.size()<2)
			return;
		int keyCounter = 0;
		
		std::vector<Ogre::Vector4> vKeys;
		std::vector<Ogre::Vector3> vUntimedKeys;

		double fCurrTime=0.0;

		for(int iKey=0; iKey<(int)mAnimKeys.size(); iKey++)
		{
			mAnimKeys[iKey]->SetName("Key_" + Ogre::StringConverter::toString(++keyCounter));
			Ogre::Vector3 keyPos = mAnimKeys[iKey]->GetGlobalPosition();

			if(mStaticMode)
			{
				vUntimedKeys.push_back(keyPos);
				fCurrTime+=mAnimKeys[iKey]->GetComponent<AnimKey>()->GetTimeToNextKey();
				mKeyTiming.push_back(fCurrTime);
			}
			else
				vKeys.push_back(Ogre::Vector4(keyPos.x, keyPos.y, keyPos.z, mAnimKeys[iKey]->GetComponent<AnimKey>()->GetTimeToNextKey()));
		}
		if(mStaticMode)
			mSpline.SetPoints(vUntimedKeys, mIsClosed);
		else
			mSpline.SetPoints(vKeys, mIsClosed);

		mSplineObject->begin("RedLine", Ogre::RenderOperation::OT_LINE_STRIP);
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

		if (mSplineObject) mSplineObject->setVisible(show);

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
		if (mLookAtObject && SceneManager::Instance().GetShowEditorVisuals())
		{
			if (!mLookAtLine)
			{
				mLookAtLine = Main::Instance().GetOgreSceneMgr()->createManualObject();
				Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->attachObject(mLookAtLine);
			}
			mLookAtLine->clear();
			mLookAtLine->begin("BlueLine", Ogre::RenderOperation::OT_LINE_STRIP);
			mLookAtLine->position(GetOwner()->GetGlobalPosition());
			mLookAtLine->position(mLookAtObject->GetGlobalPosition());
			mLookAtLine->end();
			mLookAtLine->setCastShadows(false);
		}
	}
	void GOCMover::_updateNormalLookAtLine()
	{
		if (mNormalLookAtObject && SceneManager::Instance().GetShowEditorVisuals())
		{
			if (!mNormalLookAtLine)
			{
				mNormalLookAtLine = Main::Instance().GetOgreSceneMgr()->createManualObject();
				Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->attachObject(mNormalLookAtLine);
			}
			mNormalLookAtLine->clear();
			mNormalLookAtLine->begin("BlueLine", Ogre::RenderOperation::OT_LINE_STRIP);
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
		if (subject == mNormalLookAtObject)
		{
			_destroyNormalLookAtLine();
			mNormalLookAtObject = nullptr;
		}
	}

	void GOCMover::SetLookAtObject(GameObject *target)
	{
		_destroyLookAtLine();
		if (mLookAtObject && mNormalLookAtObject != mLookAtObject) mLookAtObject->removeListener(this);
		mLookAtObject = target;
		if (mLookAtObject && mLookAtObject != mNormalLookAtObject) mLookAtObject->addDeleteListener(this);
		if (mOwnerGO) _updateLookAtLine();
	}
	void GOCMover::SetNormalLookAtObject(GameObject *target)
	{
		_destroyNormalLookAtLine();
		if (mNormalLookAtObject && mNormalLookAtObject != mLookAtObject) mNormalLookAtObject->removeListener(this);
		mNormalLookAtObject = target;
		if (mNormalLookAtObject && mLookAtObject != mNormalLookAtObject) mNormalLookAtObject->addDeleteListener(this);
		if (mOwnerGO) _updateNormalLookAtLine();
	}

	void GOCMover::notifyKeyDelete(GOCAnimKey *key)
	{
		for (unsigned int i = 0; i < mAnimKeys.size(); i++)
		{
			auto iter = mAnimKeys.begin() + i;
			if (mAnimKeys[i] == key->GetOwner())
			{
				if ((i+1) < mAnimKeys.size())
				{
					GameObject *succ = mAnimKeys[i+1];
					if (!succ->GetComponent<GOCAnimKey>()) return;
					if (i == 0)
						;//succ->GetComponent<GOCAnimKey>()->mPredecessor = this;
					else succ->GetComponent<GOCAnimKey>()->mPredecessor = mAnimKeys[i-1]->GetComponent<AnimKey>();
				}
				mAnimKeys.erase(iter);
				break;
			}
		}

		UpdateKeys();
	}

	void GOCMover::InsertKey(GameObject *key, AnimKey *pred)
	{
		IceAssert(std::find(mAnimKeys.begin(), mAnimKeys.end(), key) == mAnimKeys.end())
		/*if (pred == this)
		{
			if (!mAnimKeys.empty())
				(*(mAnimKeys.begin()))->GetComponent<GOCAnimKey>()->mPredecessor = key->GetComponent<AnimKey>();
			mAnimKeys.insert(mAnimKeys.begin(), key);
		}
		else*/
		if(pred==nullptr)//insert first key
		{
			mAnimKeys.insert(mAnimKeys.begin(), key);
			if(mAnimKeys.size()>1)
				mAnimKeys[1]->GetComponent<GOCAnimKey>()->mPredecessor = mAnimKeys[0]->GetComponent<AnimKey>();
			UpdateKeys();
			return;
		}


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
