
#include "IceGOCMover.h"
#include "IcegameObject.h"
#include "IceMessageSystem.h"
#include "IceGOCView.h"
#include "IceSceneManager.h"


namespace Ice
{

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
		GameObjectPtr mover = GetMover();
		if (!mover.get()) return;
		mover->GetComponent<GOCMover>()->UpdateKeys();
	}
	GameObjectPtr GOCAnimKey::GetMover()
	{
		GameObjectPtr owner = mOwnerGO.lock();
		if (!owner.get()) return GameObjectPtr();
		owner->ResetObjectReferenceIterator();
		while (owner->HasNextObjectReference())
		{
			ObjectReferencePtr objRef = owner->GetNextObjectReference();
			if (objRef->UserID == GOCMover::ReferenceTypes::MOVER) return objRef->Object.lock();
		}
		return GameObjectPtr();
	}
	GameObjectPtr GOCAnimKey::CreateSuccessor()
	{
		GameObjectPtr mover = GetMover();
		IceAssert(mover.get());
		unsigned int index = 0;
		mover->ResetObjectReferenceIterator();
		while (mover->HasNextObjectReference())
		{
			ObjectReferencePtr objRef = mover->GetNextObjectReference();
			if (objRef->UserID == GOCMover::ReferenceTypes::KEY)
			{
				index++;
				if (objRef->Object.lock().get() == mOwnerGO.lock().get()) break;
			}
		}
		return mover->GetComponent<GOCMover>()->CreateKey(index);
	}

	GOCMover::GOCMover()
	{
		mMoving = false;
		mPerformingMovement = false;
		mPaused=false;
		mSplineObject = nullptr;
		mIsClosed = false;
		mIgnoreOrientation=false;
		mLookAtLine = nullptr;
		mNormalLookAtLine = nullptr;
		mEnabled = false;
		mLastKeyIndex = -1;
		mfLastPos = 0;
		MessageSystem::Instance().JoinNewsgroup(this, "START_PHYSICS");
	}
	void GOCMover::Init()
	{
		if (!mSplineObject)
		{
			mSplineObject = Ice::Main::Instance().GetOgreSceneMgr()->createManualObject("Spline_" + SceneManager::Instance().RequestIDStr());
			Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->attachObject(mSplineObject);
			mSplineObject->setVisible(SceneManager::Instance().GetShowEditorVisuals());
		}
		Reset();
		if (mEnabled) Trigger();
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

	void GOCMover::SetOwner(std::weak_ptr<GameObject> go)
	{
		mOwnerGO = go;
		GameObjectPtr owner = mOwnerGO.lock();
		if (!owner.get()) return;
		UpdatePosition(owner->GetGlobalPosition());
		UpdateOrientation(owner->GetGlobalOrientation());
		Reset();
		if (mEnabled) Trigger();
	}

	void GOCMover::Save(LoadSave::SaveSystem& mgr)
	{
		//mgr.SaveAtom("float", &mTimeToNextKey, "TimeToNextKey");
		mgr.SaveAtom("bool", &mIsClosed, "Closed");
		mgr.SaveAtom("bool", &mStaticMode, "StaticMode");
		mgr.SaveAtom("bool", &mIgnoreOrientation, "IgnoreOrientation");
		mgr.SaveAtom("bool", &mEnabled, "Enabled");
	}
	void GOCMover::Load(LoadSave::LoadSystem& mgr)
	{
		Init();
		//mgr.LoadAtom("float", &mTimeToNextKey);
		mgr.LoadAtom("bool", &mIsClosed);
		mgr.LoadAtom("bool", &mStaticMode);
		mgr.LoadAtom("bool", &mIgnoreOrientation);
		mgr.LoadAtom("bool", &mEnabled);
	}

	void GOCMover::UpdatePosition(Ogre::Vector3 position)
	{
		_updateLookAtLine();
		_updateNormalLookAtLine();
		UpdateKeys();
	}

	void GOCMover::Reset()
	{
		GameObjectPtr keyObj = GetKey(0);
		if (keyObj.get())
		{
			PrepareMovement(true);
			SetOwnerPosition(keyObj->GetGlobalPosition());
			SetOwnerOrientation(keyObj->GetGlobalOrientation());
			PrepareMovement(false);
		}
		mMoving = false;
		mfLastPos=0;
		mLastKeyIndex = -1;
	}

	void GOCMover::Trigger()
	{
		if(mPaused)
			mPaused=false;
		if(mMoving)
			return;
		mMoving = true;
		mfLastPos=0;
		Ice::Msg msg; msg.type = "MOVER_START";
		mOwnerGO.lock()->SendInstantMessage(msg);
		if(GetNumKeys() < 1)
		{
			Ice::Msg msg; msg.type = "MOVER_END";
			mOwnerGO.lock()->SendMessage(msg);
			mMoving = false;
		}
	}

	GameObjectPtr GOCMover::CreateKey(unsigned int insertIndex)
	{
		GameObjectPtr owner = mOwnerGO.lock();
		IceAssert(owner.get());
		std::list<GameObjectPtr> keyObjects;
		owner->GetReferencedObjects(ReferenceTypes::KEY, keyObjects);

		owner->RemoveObjectReferences(ReferenceTypes::KEY);

		GameObjectPtr newKey = SceneManager::Instance().CreateGameObject();
		newKey->AddComponent(std::make_shared<GOCAnimKey>());
		newKey->AddObjectReference(SceneManager::Instance().GetObjectByInternID(owner->GetID()), ObjectReference::PERSISTENT, ReferenceTypes::MOVER);
		std::weak_ptr<GameObject> weakNewKey = std::weak_ptr<GameObject>(newKey);
		
		auto iter = keyObjects.begin();
		for (unsigned int i = 0; i < insertIndex; i++)
		{
			if (iter != keyObjects.end()) iter++;
		}
		keyObjects.insert(iter, weakNewKey);

		ITERATE(i, keyObjects) 
			owner->AddObjectReference(*i, ObjectReference::OWNER|ObjectReference::PERSISTENT|ObjectReference::MOVEIT_USER, ReferenceTypes::KEY);
		return newKey;
	}

	unsigned int GOCMover::GetNumKeys()
	{
		GameObjectPtr owner = mOwnerGO.lock();
		int out = 0;
		if (!owner.get()) return out;
		owner->ResetObjectReferenceIterator();
		while (owner->HasNextObjectReference())
			if (owner->GetNextObjectReference()->UserID == ReferenceTypes::KEY) out++;

		return out;
	}

	GameObjectPtr GOCMover::GetKey(unsigned int index)
	{
		GameObjectPtr owner = mOwnerGO.lock();
		if (!owner.get()) return GameObjectPtr();

		int keyIndexIter = 0;
		owner->ResetObjectReferenceIterator();
		while (owner->HasNextObjectReference())
		{
			ObjectReferencePtr objRef = owner->GetNextObjectReference();
			if (objRef->UserID == ReferenceTypes::KEY)
			{
				if (keyIndexIter == index) return objRef->Object.lock();
				keyIndexIter++;
			}
		}
		return GameObjectPtr();
	}

	void GOCMover::PrepareMovement(bool prepare)
	{
		mPerformingMovement = prepare;
	}

	void GOCMover::Pause()
	{
		mPaused=true;
	}

	void GOCMover::Stop()
	{
		mMoving = false;
		mfLastPos=0;
		Ice::Msg msg; msg.type = "MOVER_END";
		mOwnerGO.lock()->SendInstantMessage(msg);
		mLastKeyIndex = -1;
		//reset mover to first key
		Reset();
	}

	void GOCMover::ReceiveMessage( Msg &msg )
	{
		GameObjectPtr owner = mOwnerGO.lock();
		if (msg.type == "START_PHYSICS" && owner.get())
		{
			if (mLookAtLine) _updateLookAtLine();
			if (mNormalLookAtLine) _updateNormalLookAtLine();
			Ogre::Vector3 upVector = Ogre::Vector3(0,1,0);
			GameObjectPtr normalLookAtObj = GetNormalLookAtObject();
			if (normalLookAtObj.get())
			{
				upVector = normalLookAtObj->GetGlobalPosition() - owner->GetGlobalPosition();
				upVector.normalise();
			}
			GameObjectPtr lookAtObj = GetLookAtObject();
			if (lookAtObj)
			{
				Ogre::Vector3 lookAtDir = lookAtObj->GetGlobalPosition() - owner->GetGlobalPosition();
				lookAtDir.normalise();

				PrepareMovement(true);
				SetOwnerOrientation(Utils::ZDirToQuat(lookAtDir, upVector));
				PrepareMovement(false);
			}
			if (mMoving && !mPaused && SceneManager::Instance().IsClockEnabled())
			{
				float time = msg.params.GetFloat("TIME");

				Ogre::Vector3 oldPos = mOwnerGO.lock()->GetGlobalPosition();
				
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
					if(keyIndex!=-1)
					{
						if (keyIndex != mLastKeyIndex)
						{
							//Send a message that a key passed.
							Ice::Msg msg; msg.type = "MOVER_KEY";
							GameObjectPtr keyObj = GetKey(keyIndex);
							if (keyObj.get())
							{
								msg.params.AddOgreString("Keyname", keyObj->GetName());
								owner->SendInstantMessage(msg);
							}
							mLastKeyIndex = keyIndex;
						}
					}
					else
						mLastKeyIndex = keyIndex;
				}

				if (!(lookAtObj.get() || mIgnoreOrientation))		//look towards current target direction
				{
					Ogre::Vector3 lookAtDir = (mOwnerGO.lock()->GetGlobalPosition() - oldPos).normalisedCopy();
					PrepareMovement(true);
					SetOwnerOrientation(Utils::ZDirToQuat(lookAtDir, upVector));
					PrepareMovement(false);
				}

				mfLastPos+=time;
				if(mfLastPos>=(mStaticMode ? mKeyTiming[mKeyTiming.size()-1] : mSpline.GetLength()))
				{
					mMoving = false;
					mfLastPos=0;
					Ice::Msg msg; msg.type = "MOVER_END";
					owner->SendMessage(msg);
					mLastKeyIndex = -1;
				}
			}
		}
	}

	void GOCMover::UpdateKeys()
	{
		GameObjectPtr owner = mOwnerGO.lock();
		if (!owner.get()) return;
		if (mPerformingMovement) return;
		/*if (mAnimKeys.size() > 0)
		{
			PrepareMovement(true);
			SetOwnerPosition(mAnimKeys[0]->GetGlobalPosition());
			SetOwnerOrientation(mAnimKeys[0]->GetGlobalOrientation());
			PrepareMovement(false);
		}*/
		//if (mMoving && !mOwnerGO.lock()->GetUpdatingFromParent()) return;
		mSplineObject->clear();
		if(GetNumKeys() < 2) return;
		int keyCounter = 0;
		
		std::vector<Ogre::Vector4> vKeys;
		std::vector<Ogre::Vector3> vUntimedKeys;

		double fCurrTime=0.0;

		owner->ResetObjectReferenceIterator();
		while (owner->HasNextObjectReference())
		{
			ObjectReferencePtr objRef = owner->GetNextObjectReference();
			if (objRef->UserID == ReferenceTypes::KEY)
			{
				GameObjectPtr keyObj = objRef->Object.lock();
				if (!keyObj.get()) continue;
				Ogre::Vector3 keyPos = keyObj->GetGlobalPosition();

				if(mStaticMode)
				{
					vUntimedKeys.push_back(keyPos);
					fCurrTime += keyObj->GetComponent<GOCAnimKey>()->GetTimeToNextKey();
					mKeyTiming.push_back(fCurrTime);
				}
				else
					vKeys.push_back(Ogre::Vector4(keyPos.x, keyPos.y, keyPos.z, keyObj->GetComponent<GOCAnimKey>()->GetTimeToNextKey()));
			}
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
			mEditorVisual->setUserAny(Ogre::Any(GetOwner().get()));
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

		GameObjectPtr owner = GetOwner();
		if (owner.get())
		{
			owner->ResetObjectReferenceIterator();
			while (owner->HasNextObjectReference())
			{
				ObjectReferencePtr objRef = owner->GetNextObjectReference();
				if (objRef->UserID == ReferenceTypes::KEY) objRef->Object.lock()->ShowEditorVisuals(show);
			}
		}
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
		GameObjectPtr lookAtObj = GetLookAtObject();
		if (lookAtObj.get() && SceneManager::Instance().GetShowEditorVisuals())
		{
			if (!mLookAtLine)
			{
				mLookAtLine = Main::Instance().GetOgreSceneMgr()->createManualObject();
				Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->attachObject(mLookAtLine);
			}
			mLookAtLine->clear();
			mLookAtLine->begin("BlueLine", Ogre::RenderOperation::OT_LINE_STRIP);
			mLookAtLine->position(GetOwner()->GetGlobalPosition());
			mLookAtLine->position(lookAtObj->GetGlobalPosition());
			mLookAtLine->end();
			mLookAtLine->setCastShadows(false);
		}
	}
	void GOCMover::_updateNormalLookAtLine()
	{
		GameObjectPtr lookAtObj = GetNormalLookAtObject();
		if (lookAtObj.get() && SceneManager::Instance().GetShowEditorVisuals())
		{
			if (!mNormalLookAtLine)
			{
				mNormalLookAtLine = Main::Instance().GetOgreSceneMgr()->createManualObject();
				Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->attachObject(mNormalLookAtLine);
			}
			mNormalLookAtLine->clear();
			mNormalLookAtLine->begin("BlueLine", Ogre::RenderOperation::OT_LINE_STRIP);
			mNormalLookAtLine->position(GetOwner()->GetGlobalPosition());
			mNormalLookAtLine->position(lookAtObj->GetGlobalPosition());
			mNormalLookAtLine->end();
			mNormalLookAtLine->setCastShadows(false);
		}
	}

	GameObjectPtr GOCMover::GetLookAtObject()
	{
		GameObjectPtr owner = GetOwner();
		if (owner.get())
		{
			owner->ResetObjectReferenceIterator();
			while (owner->HasNextObjectReference())
			{
				ObjectReferencePtr objRef = owner->GetNextObjectReference();
				if (objRef->UserID == ReferenceTypes::LOOKAT) return objRef->Object.lock();
			}
		}
		return GameObjectPtr();
	}
	std::shared_ptr<GameObject> GOCMover::GetNormalLookAtObject()
	{
		GameObjectPtr owner = GetOwner();
		if (owner.get())
		{
			owner->ResetObjectReferenceIterator();
			while (owner->HasNextObjectReference())
			{
				ObjectReferencePtr objRef = owner->GetNextObjectReference();
				if (objRef->UserID == ReferenceTypes::NORMALLOOKAT) return objRef->Object.lock();
			}
		}
		return GameObjectPtr();
	}

	void GOCMover::SetLookAtObject(GameObjectPtr target)
	{
		_destroyLookAtLine();
		GameObjectPtr owner = GetOwner();
		owner->RemoveObjectReferences(ReferenceTypes::LOOKAT);
		if (target.get()) owner->AddObjectReference(target, ObjectReference::PERSISTENT, ReferenceTypes::LOOKAT);
		_updateLookAtLine();
	}
	void GOCMover::SetNormalLookAtObject(GameObjectPtr target)
	{
		_destroyLookAtLine();
		GameObjectPtr owner = GetOwner();
		owner->RemoveObjectReferences(ReferenceTypes::LOOKAT);
		if (target.get()) owner->AddObjectReference(target, ObjectReference::PERSISTENT, ReferenceTypes::NORMALLOOKAT);
		_updateLookAtLine();
	}

	/*void GOCMover::notifyKeyDelete(GOCAnimKey *key)
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
		else*//*
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
	}*/

}
