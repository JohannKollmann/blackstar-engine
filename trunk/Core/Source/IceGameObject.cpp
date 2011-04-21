
#include "IceGameObject.h"
#include "IceSceneManager.h"
#include "IceGOCScriptMakros.h"
#include "IceGOCScript.h"
#include "IceGOCAI.h"
#include "IceProcessNodeManager.h"

namespace Ice
{
	void ObjectReference::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("int", &Flags, "Flags");
		mgr.SaveAtom("int", &UserID, "UserID");
		mgr.SaveObject(Object.lock().get(), "Object", true);
	}
	void ObjectReference::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("int", &Flags);
		mgr.LoadAtom("int", &UserID);
		Object = std::weak_ptr<GameObject>(mgr.LoadTypedObject<GameObject>());
	}

	GameObject::GameObject()
	{
		mSelectable = true;
		mID = SceneManager::Instance().RequestID();
		mName = "GameObject";
		mScale = Ogre::Vector3(1,1,1);
		mPosition = Ogre::Vector3(0,0,0);
		mOrientation = Ogre::Quaternion();
		mTransformingReferencedObjects = false;
		mFreezed = false;
		mReferencedObjectsInternIterator = mReferencedObjects.begin();
	}

	GameObject::~GameObject()
	{
		ClearGOCs();
		ClearOwnedObjects();
	}

	void GameObject::SetWeakThis(std::weak_ptr<LoadSave::Saveable> wThis)
	{
		IceAssert(wThis.lock().get() == this)
		mWeakThis = std::weak_ptr<GameObject>(std::static_pointer_cast<GameObject, LoadSave::Saveable>(wThis.lock()));
	}

	void GameObject::SendMessage(Msg &msg)
	{
		mCurrentMessages.push_back(msg);
		if (mCurrentMessages.size() == 1) SceneManager::Instance().AddToMessageQueue(mWeakThis);
	}

	void GameObject::SendInstantMessage(Msg &msg)
	{
		for (unsigned int i = 0; i < mComponents.size(); i++)
		{
			mComponents[i]->ReceiveObjectMessage(msg);
		}
	}

	void GameObject::ProcessMessages()
	{
		std::vector<Msg> msgcopy = mCurrentMessages;
		mCurrentMessages.clear();
		for (auto i = mComponents.begin(); i != mComponents.end(); i++)
		{
			for (std::vector<Msg>::iterator x = msgcopy.begin(); x != msgcopy.end(); x++)
			{
				(*i)->ReceiveObjectMessage((*x));
			}
		}
		msgcopy.clear();
	}

	void GameObject::AddComponent(GOComponentPtr component)
	{
		for (auto i = mComponents.begin(); i != mComponents.end(); i++)
		{
			IceAssert((*i)->GetFamilyID() != component->GetFamilyID());
		}
		mComponents.push_back(component);
		component->SetOwner(mWeakThis);
		component->UpdatePosition(GetGlobalPosition());
		component->UpdateOrientation(GetGlobalOrientation());
	}

	GOComponent* GameObject::GetComponent(const GOComponent::goc_id_family &familyID)
	{
		for (auto i = mComponents.begin(); i != mComponents.end(); i++)
		{
			if ((*i)->GetFamilyID() == familyID)
			{
				return (*i).get();
			}
		}
		return nullptr;
	}

	GOComponent* GameObject::GetComponent(const GOComponent::goc_id_family& familyID, GOComponent::goc_id_type typeID)
	{
		for (auto i = mComponents.begin(); i != mComponents.end(); i++)
		{
			if ((*i)->GetFamilyID() == familyID)
			{
				if ((*i)->GetComponentID() == typeID) return (*i).get();
				else return nullptr;
			}
		}
		return nullptr;
	}

	void GameObject::RemoveComponent(const GOComponent::goc_id_family &familyID)
	{
		for (auto i = mComponents.begin(); i != mComponents.end(); i++)
		{
			if ((*i)->GetFamilyID() == familyID)
			{
				mComponents.erase(i);
				return;
			}
		}
	}

	void GameObject::ClearGOCs()
	{
		for (unsigned int i = 0; i < mComponents.size(); i++)
		{
			mComponents[i]->SetOwner(std::weak_ptr<GameObject>());
		}

		mComponents.clear();
	}

	void GameObject::ClearOwnedObjects()
	{
		if (!SceneManager::Instance().GetClearingScene())
		{
			while (HasNextObjectReference())
			{
				ObjectReferencePtr objRef = GetNextObjectReference();
				GameObjectPtr obj = objRef->Object.lock();
				if (obj.get() && objRef->Flags & ObjectReference::OWNER)
					SceneManager::Instance().RemoveGameObject(obj->GetID());
			}
		}
		mReferencedObjectsInternIterator = mReferencedObjects.begin();
	}

	void GameObject::AddObjectReference(const GameObjectPtr &other, unsigned int flags, unsigned int userID)
	{
		if ((flags & ObjectReference::MOVEIT) || (flags & ObjectReference::OWNER)) 
		{
			other->ResetObjectReferenceIterator();
			while (other->HasNextObjectReference())
			{
				ObjectReferencePtr objRef = other->GetNextObjectReference();
				GameObjectPtr obj = objRef->Object.lock();
				IceAssert(! (obj.get() == this && ((objRef->Flags & ObjectReference::MOVEIT) || (objRef->Flags & ObjectReference::OWNER))))
			}
		}
		ObjectReferencePtr objLink = std::make_shared<ObjectReference>();
		objLink->Object = std::weak_ptr<GameObject>(other);
		objLink->Flags = flags;
		objLink->UserID = userID;
		mReferencedObjects.push_back(objLink);
		mReferencedObjectsInternIterator = mReferencedObjects.begin();
	}

	void  GameObject::RemoveObjectReferences(GameObject *object)
	{
		ResetObjectReferenceIterator();
		while (HasNextObjectReference())
		{
			ObjectReferencePtr objRef = *mReferencedObjectsInternIterator;
			if (objRef->Object.lock().get() == object)
			{
				mReferencedObjectsInternIterator = mReferencedObjects.erase(mReferencedObjectsInternIterator);
			}
			else mReferencedObjectsInternIterator++;
		}
		mReferencedObjectsInternIterator = mReferencedObjects.begin();
	}

	void  GameObject::RemoveObjectReferences(unsigned int userID)
	{
		ResetObjectReferenceIterator();
		while (HasNextObjectReference())
		{
			ObjectReferencePtr objRef = *mReferencedObjectsInternIterator;
			if (objRef->UserID == userID)
			{
				mReferencedObjectsInternIterator = mReferencedObjects.erase(mReferencedObjectsInternIterator);
			}
			else mReferencedObjectsInternIterator++;
		}
		mReferencedObjectsInternIterator = mReferencedObjects.begin();
	}

	void GameObject::SetParent(GameObjectPtr parent)
	{
		GameObjectPtr oldParent = GetParent();
		if (oldParent.get())
		{
			oldParent->RemoveObjectReferences(this);
			RemoveObjectReferences(ReferenceTypes::PARENT);		//Remove old parent
		}

		if (parent.get())
		{
			AddObjectReference(parent, ObjectReference::PERSISTENT|ObjectReference::OWNED, ReferenceTypes::PARENT);
			parent->AddObjectReference(mWeakThis.lock(), ObjectReference::OWNER|ObjectReference::MOVEIT|ObjectReference::PERSISTENT);
		}
	}

	GameObjectPtr GameObject::GetParent()
	{
		ResetObjectReferenceIterator();
		while (HasNextObjectReference())
		{
			ObjectReferencePtr objRef = GetNextObjectReference();
			GameObjectPtr obj = objRef->Object.lock();
			if (obj.get() && objRef->UserID == ReferenceTypes::PARENT) return obj;
		}
		return GameObjectPtr();
	}

	bool GameObject::HasNextObjectReference()
	{
		if (mReferencedObjectsInternIterator == mReferencedObjects.end())
		{
			mReferencedObjectsInternIterator = mReferencedObjects.begin();
			return false;
		}
		else
		{
			if ((*mReferencedObjectsInternIterator)->Object.expired())
			{
				mReferencedObjectsInternIterator = mReferencedObjects.erase(mReferencedObjectsInternIterator);
				return HasNextObjectReference();
			}
			else return true;
		}
	}

	std::shared_ptr<ObjectReference> GameObject::GetNextObjectReference()
	{
		IceAssert(mReferencedObjectsInternIterator != mReferencedObjects.end());
		std::shared_ptr<ObjectReference> out = *mReferencedObjectsInternIterator;
		mReferencedObjectsInternIterator++;
		return out;
	}

	void GameObject::ResetObjectReferenceIterator()
	{
		mReferencedObjectsInternIterator = mReferencedObjects.begin();
	}

	GameObjectPtr GameObject::GetReferencedObjectByName(Ogre::String name)
	{
		ResetObjectReferenceIterator();
		while (HasNextObjectReference())
		{
			GameObjectPtr obj = GetNextObjectReference()->Object.lock();
			if (obj.get() && obj->GetName() == name) return obj;
		}
		return GameObjectPtr();
	}

	void GameObject::GetReferencedObjects(unsigned int userID, std::vector<GameObjectPtr> &out)
	{
		ResetObjectReferenceIterator();
		while (HasNextObjectReference())
		{
			ObjectReferencePtr objRef = GetNextObjectReference();
			GameObjectPtr object = objRef->Object.lock();
			if (objRef->UserID == userID && object.get()) out.push_back(object);
		}
	}
	void GameObject::GetReferencedObjects(unsigned int userID, std::list<GameObjectPtr> &out)
	{
		ResetObjectReferenceIterator();
		while (HasNextObjectReference())
		{
			ObjectReferencePtr objRef = GetNextObjectReference();
			GameObjectPtr object = objRef->Object.lock();
			if (objRef->UserID == userID && object.get()) out.push_back(object);
		}
	}

	void GameObject::GetReferencedObjectsByFlag(unsigned int flags, std::vector<GameObjectPtr> &out)
	{
		ResetObjectReferenceIterator();
		while (HasNextObjectReference())
		{
			ObjectReferencePtr objRef = GetNextObjectReference();
			GameObjectPtr object = objRef->Object.lock();
			if (objRef->Flags & flags && object.get()) out.push_back(object);
		}		
	}
	void GameObject::GetReferencedObjectsByFlag(unsigned int flags, std::list<GameObjectPtr> &out)
	{
		ResetObjectReferenceIterator();
		while (HasNextObjectReference())
		{
			ObjectReferencePtr objRef = GetNextObjectReference();
			GameObjectPtr object = objRef->Object.lock();
			if (objRef->Flags & flags && object.get()) out.push_back(object);
		}		
	}

	void GameObject::SetGlobalPosition(const Ogre::Vector3 &pos, bool moveReferences, bool moveChildren, std::set<GameObject*> *referenceBlacklist)
	{	
		if (moveReferences || moveChildren)
		{
			mTransformingReferencedObjects = true;
			ResetObjectReferenceIterator();
			while (HasNextObjectReference())
			{
				ObjectReferencePtr objRef = GetNextObjectReference();
				GameObjectPtr obj = objRef->Object.lock();
				if (objRef->Flags & ObjectReference::MOVEIT || objRef->Flags & ObjectReference::MOVEIT_USER)
				{
					Ogre::Vector3 localObjPos = mOrientation.Inverse () * (obj->GetGlobalPosition() - mPosition);

					if (objRef->Flags & ObjectReference::MOVEIT && moveChildren)
					{
						obj->SetGlobalPosition(pos + (mOrientation * localObjPos), moveReferences, moveChildren, referenceBlacklist);
					}
					else if (objRef->Flags & ObjectReference::MOVEIT_USER && moveReferences)
					{
						if (!referenceBlacklist || referenceBlacklist->find(obj.get()) == referenceBlacklist->end())
						{
							if (referenceBlacklist) referenceBlacklist->insert(obj.get());			
							obj->SetGlobalPosition(pos + (mOrientation * localObjPos), moveReferences, moveChildren, referenceBlacklist);
						}
					}
				}
			}
			mTransformingReferencedObjects = false;
		}

		mPosition = pos;
		for (auto i = mComponents.begin(); i != mComponents.end(); i++)
		{
			(*i)->_updatePosition(pos);
		}
	}

	void GameObject::SetGlobalOrientation(const Ogre::Quaternion &orientation, bool moveReferences, bool moveChildren, std::set<GameObject*> *referenceBlacklist)
	{
		if (moveReferences || moveChildren)
		{
			mTransformingReferencedObjects = true;
			ResetObjectReferenceIterator();
			while (HasNextObjectReference())
			{
				ObjectReferencePtr objRef = GetNextObjectReference();
				GameObjectPtr obj = objRef->Object.lock();
				if (objRef->Flags & ObjectReference::MOVEIT || objRef->Flags & ObjectReference::MOVEIT_USER)
				{
					Ogre::Vector3 localObjPos = mOrientation.Inverse () * (obj->GetGlobalPosition() - mPosition);
					Ogre::Quaternion localObjRot = mOrientation.Inverse() * obj->GetGlobalOrientation();

					if (objRef->Flags & ObjectReference::MOVEIT && moveChildren)
					{
						obj->SetGlobalOrientation(orientation * localObjRot, moveReferences, moveChildren, referenceBlacklist);
						obj->SetGlobalPosition(mPosition + (orientation * localObjPos), moveReferences, moveChildren, referenceBlacklist);
					}
					else if (objRef->Flags & ObjectReference::MOVEIT_USER && moveReferences)
					{
						if (!referenceBlacklist || referenceBlacklist->find(obj.get()) == referenceBlacklist->end())
						{
							if (referenceBlacklist) referenceBlacklist->insert(obj.get());
							obj->SetGlobalOrientation(orientation * localObjRot, moveReferences, moveChildren, referenceBlacklist);
							obj->SetGlobalPosition(mPosition + (orientation * localObjPos), moveReferences, moveChildren, referenceBlacklist);
						}
					}
				}
			}
			mTransformingReferencedObjects = false;
		}

		mOrientation = orientation;
		for (auto i = mComponents.begin(); i != mComponents.end(); i++)
		{
			(*i)->_updateOrientation(orientation);
		}
	}

	void GameObject::SetGlobalScale(const Ogre::Vector3 &scale)
	{
		mScale = scale;
		for (auto i = mComponents.begin(); i != mComponents.end(); i++)
		{
			(*i)->_updateScale(scale);
		}
	}

	void GameObject::Freeze(bool freeze)
	{
		mFreezed = freeze;
		for (auto i = mComponents.begin(); i != mComponents.end(); i++)
		{
			(*i)->Freeze(freeze);
		}
	}
	void GameObject::FreeResources(bool free)
	{
		for (unsigned int i = 0; i < mComponents.size(); i++)
		{
			mComponents[i]->FreeResources(free);
		}
	}
	void GameObject::ShowEditorVisuals(bool show)
	{
		for (unsigned int i = 0; i < mComponents.size(); i++)
		{
			mComponents[i]->ShowEditorVisual(show);
		}
	}

	bool GameObject::IsStatic()
	{
		for (auto i = mComponents.begin(); i != mComponents.end(); i++)
		{
			if (!(*i)->IsStatic()) return false;
		}
		return true;
	}

	void GameObject::FirePostInit()
	{
		ITERATE(i, mComponents)
			(*i)->NotifyPostInit();

		ResetObjectReferenceIterator();
		while (HasNextObjectReference())
		{
			std::shared_ptr<ObjectReference> ref = GetNextObjectReference();
			if (ref->Flags & ObjectReference::OWNER) ref->Object.lock()->FirePostInit();
		}

		Msg msg;
		msg.type = "INIT_POST";
		SendInstantMessage(msg);
	}

	void GameObject::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("Ogre::String", (void*)(&mName), "mName");
		mgr.SaveAtom("Ogre::Vector3", (void*)(&mPosition), "Position");
		mgr.SaveAtom("Ogre::Quaternion", (void*)(&mOrientation), "Orientation");
		mgr.SaveAtom("Ogre::Vector3", (void*)(&mScale), "Scale");
		mgr.SaveAtom("bool", (void*)&mSelectable, "Selectable");

		std::vector< GOComponentPtr > persistentComponents;
		for (auto i = mComponents.begin(); i != mComponents.end(); i++)
			if ((*i)->_getIsSaveable()) persistentComponents.push_back((*i));
		mgr.SaveAtom("vector<GOComponentPtr>", (void*)(&persistentComponents), "Components");

		std::vector<ObjectReferencePtr> persistentLinks;
		std::vector< GameObjectPtr > ownershipBarrier;
		for (auto i = mReferencedObjects.begin(); i != mReferencedObjects.end();)
		{
			std::shared_ptr<GameObject> obj = (*i)->Object.lock();
			if (!obj.get())
			{
				i = mReferencedObjects.erase(i);
				if (i == mReferencedObjects.end()) break;
				else continue;
			}
			ownershipBarrier.push_back(obj);
			if ((*i)->Flags & ObjectReference::PERSISTENT) persistentLinks.push_back(*i);
			i++;
		}
		mgr.SaveAtom("vector<ObjectReferencePtr>", (void*)(&persistentLinks), "Object References");
	}

	void GameObject::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("Ogre::String", &mName);
		mgr.LoadAtom("Ogre::Vector3", &mPosition);
		mgr.LoadAtom("Ogre::Quaternion", &mOrientation);
		mgr.LoadAtom("Ogre::Vector3", &mScale);
		mgr.LoadAtom("bool", &mSelectable);
		std::vector< GOComponentPtr > persistentComponents;
		mgr.LoadAtom("vector<GOComponentPtr>", (void*)(&persistentComponents));
		for (unsigned int i = 0; i < persistentComponents.size(); i++)
			mComponents.push_back(persistentComponents[i]);

		mgr.LoadAtom("vector<ObjectReferencePtr>", (void*)(&mReferencedObjects));

		SceneManager::Instance().RegisterGameObject(mWeakThis.lock());

		for (unsigned int i = 0; i < mComponents.size(); i++)
		{
			mComponents[i]->SetOwner(mWeakThis);
			mComponents[i]->UpdatePosition(GetGlobalPosition());
			mComponents[i]->UpdateOrientation(GetGlobalOrientation());
		}

		FirePostInit();
	}

	std::vector<ScriptParam> GameObject::AddComponent(Script& caller, std::vector<ScriptParam> &vParams)
	{
		GOCEditorInterface *goc = SceneManager::Instance().NewGOC(vParams[0].getString());
		DataMap data;
		data.ParseString(vParams[1].getString());
		goc->SetParameters(&data);
		AddComponent(GOComponentPtr(goc->GetGOComponent()));
		SCRIPT_RETURN()
	}

	std::vector<ScriptParam> GameObject::SetObjectPosition(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		float x = vParams[0].getFloat();
		float y = vParams[1].getFloat();
		float z = vParams[2].getFloat();
		SetGlobalPosition(Ogre::Vector3(x,y,z));
		return out;
	}
	std::vector<ScriptParam> GameObject::SetObjectOrientation(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		Ogre::Degree yDeg = Ogre::Degree(vParams[0].getFloat());
		Ogre::Degree pDeg = Ogre::Degree(vParams[1].getFloat());
		Ogre::Degree rDeg = Ogre::Degree(vParams[2].getFloat());
		Ogre::Matrix3 mat3;
		mat3.FromEulerAnglesYXZ(yDeg, pDeg, rDeg);
		Ogre::Quaternion q;
		q.FromRotationMatrix(mat3);

		if (vParams.size() > 3 && vParams[3].getType() == ScriptParam::PARM_TYPE_FLOAT)
		{
			ProcessNodeManager::Instance().CreateOrientationBlendProcess(this, q, vParams[3].getFloat());
		}
		else SetGlobalOrientation(q);
		return out;
	}
	std::vector<ScriptParam> GameObject::SetObjectScale(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		float x = vParams[0].getFloat();
		float y = vParams[1].getFloat();
		float z = vParams[2].getFloat();
		SetGlobalScale(Ogre::Vector3(x,y,z));
		return out;
	}
	std::vector<ScriptParam> GameObject::GetObjectName(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		out.push_back(ScriptParam(std::string(mName.c_str())));
		return out;
	}

	std::vector<ScriptParam> GameObject::SendObjectMessage(Script& caller, std::vector<ScriptParam> &vParams)
	{
		Msg msg;
		msg.type = vParams[0].getString();
		Utils::ScriptParamsToDataMap(caller, vParams, &msg.params, 1);
		SendMessage(msg);
		return std::vector<ScriptParam>();
	}

	std::vector<ScriptParam> GameObject::ReceiveObjectMessage(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;

		GOCScriptMessageCallback *callback = GetComponent<GOCScriptMessageCallback>();
		if (!callback)
		{
			callback = new GOCScriptMessageCallback();
			AddComponent(GOComponentPtr(callback));
		}
		callback->AddListener(vParams[0].getString(), vParams[1]);
		return out;
	}

	std::vector<ScriptParam> GameObject::HasScriptListener(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		bool usable = false;
		GOCScriptMessageCallback *callback = GetComponent<GOCScriptMessageCallback>();
		if (callback) usable = callback->HasListener(vParams[0].getString());
		out.push_back(ScriptParam(usable));
		return out;
	}
	std::vector<ScriptParam> GameObject::GetReferencedObjectByName(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		int id = -1;
		Ogre::String cName = vParams[0].getString();
		GameObjectPtr obj = GetReferencedObjectByName(cName);
		if (obj.get()) SCRIPT_RETURNVALUE(obj->GetID())
		else SCRIPT_RETURNVALUE(-1)
	}
	std::vector<ScriptParam> GameObject::IsNpc(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		out.push_back(ScriptParam(GetComponent<GOCAI>() != nullptr));
		return out;
	}
	std::vector<ScriptParam> GameObject::FreeResources(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		FreeResources(vParams[0].getBool());
		return out;
	}
	std::vector<ScriptParam> GameObject::Object_Play3DSound(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> delegateParams;
		delegateParams.push_back(ScriptParam(mPosition.x));
		delegateParams.push_back(ScriptParam(mPosition.y));
		delegateParams.push_back(ScriptParam(mPosition.z));
		delegateParams.push_back(vParams[0]);
		delegateParams.push_back(vParams[1]);
		delegateParams.push_back(vParams[2]);
		SceneManager::Lua_Play3DSound(caller, delegateParams);
		std::vector<ScriptParam> out;
		return out;
	}
	std::vector<ScriptParam> GameObject::Object_GetDistToObject(Script& caller, std::vector<ScriptParam> &vParams)
	{
		if (GameObjectPtr object = SceneManager::Instance().GetObjectByInternID(vParams[0].getInt()))
			SCRIPT_RETURNVALUE(GetGlobalPosition().distance(object->GetGlobalPosition()))
		else SCRIPT_RETURNERROR("invalid object id")
	}

	DEFINE_TYPEDGOLUAMETHOD_CPP(AddComponent, "string string")
	DEFINE_TYPEDGOLUAMETHOD_CPP(SetObjectPosition, "float float float")
	DEFINE_TYPEDGOLUAMETHOD_CPP(SetObjectOrientation, "float float float")
	DEFINE_TYPEDGOLUAMETHOD_CPP(SetObjectScale, "float float float")
	DEFINE_GOLUAMETHOD_CPP(GetObjectName)
	DEFINE_TYPEDGOLUAMETHOD_CPP(SendObjectMessage, "string")
	DEFINE_TYPEDGOLUAMETHOD_CPP(ReceiveObjectMessage, "string function")
	DEFINE_TYPEDGOLUAMETHOD_CPP(GetReferencedObjectByName, "string")
	DEFINE_TYPEDGOLUAMETHOD_CPP(HasScriptListener, "string")
	DEFINE_TYPEDGOLUAMETHOD_CPP(FreeResources, "bool")
	DEFINE_GOLUAMETHOD_CPP(IsNpc)
	DEFINE_TYPEDGOLUAMETHOD_CPP(Object_Play3DSound, "string float float")		//audio file, range, loudness
	DEFINE_TYPEDGOLUAMETHOD_CPP(Object_GetDistToObject, "int")

};