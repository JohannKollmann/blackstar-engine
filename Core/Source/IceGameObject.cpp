
#include "IceGameObject.h"
#include "IceSceneManager.h"
#include "IceGOCScriptMakros.h"
#include "IceGOCScript.h"
#include "IceGOCAI.h"

namespace Ice
{

	ManagedGameObject::ManagedGameObject()
	{
		mSelectable = true;
		mParent = nullptr;
		mManagedByParent = true;
		mID = SceneManager::Instance().RegisterObject(this);
		mName = "GameObject";
		mScale = Ogre::Vector3(1,1,1);
		mPosition = Ogre::Vector3(0,0,0);
		mOrientation = Ogre::Quaternion();
		UpdateLocalTransform();
		mTransformingChildren = false;
		mUpdatingFromParent = false;
		mIgnoreParent = false;
	}
	ManagedGameObject::~ManagedGameObject()
	{
		SceneManager::Instance().NotifyGODelete(this);
		if (mParent)
		{
			mParent->UnregisterChild(this);
		}
		ClearChildren();
		ClearGOCs();
	}


	GameObject::GameObject()
	{
		mSelectable = true;
		mParent = nullptr;
		mManagedByParent = true;
		mID = SceneManager::Instance().RequestID();
		mName = "GameObject";
		mScale = Ogre::Vector3(1,1,1);
		mPosition = Ogre::Vector3(0,0,0);
		mOrientation = Ogre::Quaternion();
		UpdateLocalTransform();
		mTransformingChildren = false;
		mUpdatingFromParent = false;
		mIgnoreParent = false;
		mFreezed = false;
	}

	GameObject::~GameObject()
	{
		if (mParent)
		{
			mParent->UnregisterChild(this);
		}
		ClearChildren();
		ClearGOCs();
	}

	void GameObject::SendMessage(Msg &msg)
	{
		mCurrentMessages.push_back(msg);
		if (mCurrentMessages.size() == 1) SceneManager::Instance().AddToMessageQueue(this);
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

	void GameObject::UpdateLocalTransform()
	{
		if (mParent)
		{
			mLocalPosition = mParent->GetGlobalOrientation().Inverse () * (mPosition - mParent->GetGlobalPosition());
			mLocalOrientation = mParent->GetGlobalOrientation().Inverse() * mOrientation;
		}
	}

	void GameObject::SetParent(GameObject *parent)
	{
		if (mParent)
		{
			mParent->UnregisterChild(this);
		}
		mParent = parent;
		if (mParent)
		{
			mParent->RegisterChild(this);
			UpdateLocalTransform();
		}
	}

	void GameObject::RegisterChild(GameObject *child)
	{
		mChildren.push_back(child);
		for (auto i = mComponents.begin(); i != mComponents.end(); i++)
		{
			(*i)->OnAddChild(child);
		}
	}

	void GameObject::UnregisterChild(GameObject *child)
	{
		for (auto i = mChildren.begin(); i != mChildren.end(); i++)
		{
			if ((*i) == child)
			{
				mChildren.erase(i);
				return;
			}
		}
		for (auto i = mComponents.begin(); i != mComponents.end(); i++)
		{
			(*i)->OnRemoveChild(child);
		}
	}

	std::vector<GameObject*> GameObject::DetachChildren()
	{
		auto children_copy = mChildren;
		mChildren.clear();
		return children_copy;
	}

	void GameObject::AddComponent(GOComponentPtr component)
	{
		for (auto i = mComponents.begin(); i != mComponents.end(); i++)
		{
			IceAssert((*i)->GetFamilyID() != component->GetFamilyID());
		}
		mComponents.push_back(component);
		component->SetOwner(this);
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
		for (int i = 0; i < mComponents.size(); i++)
		{
			mComponents[i]->SetOwner(nullptr);
		}

		mComponents.clear();
	}

	void GameObject::ClearChildren()
	{
		while (mChildren.size() > 0)
			delete mChildren[0];
		mChildren.clear();
	}

	void GameObject::UpdateChildren(bool move)
	{
		for (auto i = mChildren.begin(); i != mChildren.end(); i++)
		{
			if (move) (*i)->OnParentChanged();
			else (*i)->UpdateLocalTransform();
		}
	}

	GameObject* GameObject::GetChild(unsigned short index)
	{
		IceAssert(mChildren.size() > index && index >= 0);
		return mChildren[index];
	}

	void GameObject::SetGlobalPosition(Ogre::Vector3 pos, bool updateChildren)
	{
		mTransformingChildren = updateChildren;
		mPosition = pos;
		for (auto i = mComponents.begin(); i != mComponents.end(); i++)
		{
			(*i)->_updatePosition(pos);
		}
		if (mParent)
		{
			mLocalPosition = mParent->GetGlobalOrientation().Inverse () * (mPosition - mParent->GetGlobalPosition());
		}
		UpdateChildren(updateChildren);
		mTransformingChildren = false;
	}

	void GameObject::SetGlobalOrientation(Ogre::Quaternion orientation, bool updateChildren)
	{
		mTransformingChildren = updateChildren;
		mOrientation = orientation;
		for (auto i = mComponents.begin(); i != mComponents.end(); i++)
		{
			(*i)->_updateOrientation(orientation);
		}
		if (mParent)
		{
			mLocalOrientation = mParent->GetGlobalOrientation().Inverse() * orientation;
		}
		UpdateChildren(updateChildren);
		mTransformingChildren = false;
	}

	void GameObject::SetGlobalScale(Ogre::Vector3 scale)
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

	void GameObject::OnParentChanged()
	{
		if (mIgnoreParent)
		{
			UpdateLocalTransform();
			return;
		}

		mUpdatingFromParent = true;
		mOrientation = mParent->GetGlobalOrientation() * mLocalOrientation;
		mPosition = mParent->GetGlobalOrientation() * mLocalPosition + mParent->GetGlobalPosition();
		for (auto i = mComponents.begin(); i != mComponents.end(); i++)
		{
			(*i)->UpdateOrientation(mOrientation);
			(*i)->UpdatePosition(mPosition);
		}
		UpdateChildren();
		mUpdatingFromParent = false;
	}

	bool GameObject::IsStatic()
	{
		for (auto i = mComponents.begin(); i != mComponents.end(); i++)
		{
			if (!(*i)->IsStatic()) return false;
		}
		return true;
	}

	void GameObject::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("Ogre::String", (void*)(&mName), "mName");
		mgr.SaveAtom("Ogre::Vector3", (void*)(&mPosition), "Position");
		mgr.SaveAtom("Ogre::Quaternion", (void*)(&mOrientation), "Orientation");
		mgr.SaveAtom("Ogre::Vector3", (void*)(&mScale), "Scale");
		mgr.SaveAtom("bool", (void*)&mSelectable, "Selectable");

		std::vector<GOComponent*> saveable_components;
		for (auto i = mComponents.begin(); i != mComponents.end(); i++)
			if ((*i)->_getIsSaveable()) saveable_components.push_back((*i).get());
		mgr.SaveAtom("std::vector<Saveable*>", (void*)(&saveable_components), "mComponents");

		std::vector<GameObject*> managed_children;
		for (auto i = mChildren.begin(); i != mChildren.end(); i++)
			if ((*i)->mManagedByParent) managed_children.push_back((*i));
		mgr.SaveAtom("std::vector<Saveable*>", (void*)(&managed_children), "mChildren");
	}

	void GameObject::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("Ogre::String", &mName);
		mgr.LoadAtom("Ogre::Vector3", &mPosition);
		mgr.LoadAtom("Ogre::Quaternion", &mOrientation);
		mgr.LoadAtom("Ogre::Vector3", &mScale);
		mgr.LoadAtom("bool", &mSelectable);
		std::vector<GOComponent*> loaded_components;
		mgr.LoadAtom("std::vector<Saveable*>", (void*)(&loaded_components));
		for (unsigned int i = 0; i < loaded_components.size(); i++)
			mComponents.push_back(std::shared_ptr<GOComponent>(loaded_components[i]));
		for (unsigned int i = 0; i < mComponents.size(); i++)
		{
			mComponents[i]->SetOwner(this);
			mComponents[i]->UpdatePosition(GetGlobalPosition());
			mComponents[i]->UpdateOrientation(GetGlobalOrientation());
		}
		std::vector<GameObject*> managed_children;
		mgr.LoadAtom("std::vector<Saveable*>", (void*)(&managed_children));
		for (auto i = managed_children.begin(); i != managed_children.end(); i++)
		{
			(*i)->SetParent(this);
		}
	}

	std::vector<ScriptParam> GameObject::SetObjectProperty(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		Ogre::String key = vParams[0].getString().c_str();
		mScriptProperties[key] = vParams[1];
		return out;
	}
	std::vector<ScriptParam> GameObject::GetObjectProperty(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		Ogre::String key = vParams[0].getString().c_str();
		auto i = mScriptProperties.find(key);
		if (i == mScriptProperties.end()) return out;
		out.push_back(i->second);
		return out;
	}
	std::vector<ScriptParam> GameObject::HasObjectProperty(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		Ogre::String key = vParams[0].getString().c_str();
		auto i = mScriptProperties.find(key);
		out.push_back(i != mScriptProperties.end());
		return out;
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
		SetGlobalOrientation(q);
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
	std::vector<ScriptParam> GameObject::GetChildObjectByName(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		int id = -1;
		Ogre::String cName = vParams[0].getString();
		for (auto i = mChildren.begin(); i != mChildren.end(); i++)
		{
			if ((*i)->GetName() == cName)
			{
				id = (*i)->GetID();
				break;
			}
		}
		out.push_back(ScriptParam(id));
		return out;
	}
	std::vector<ScriptParam> GameObject::GetParent(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		int id = -1;
		if (mParent) id = mParent->GetID();
		out.push_back(ScriptParam(id));
		return out;
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
		if (GameObject *object = SceneManager::Instance().GetObjectByInternID(vParams[0].getInt()))
			SCRIPT_RETURNVALUE(GetGlobalPosition().distance(object->GetGlobalPosition()))
		else SCRIPT_RETURNERROR("invalid object id")
	}

	DEFINE_TYPEDGOLUAMETHOD_CPP(SetObjectProperty, "string")
	DEFINE_TYPEDGOLUAMETHOD_CPP(GetObjectProperty, "string")
	DEFINE_TYPEDGOLUAMETHOD_CPP(HasObjectProperty, "string")
	DEFINE_TYPEDGOLUAMETHOD_CPP(SetObjectPosition, "float float float")
	DEFINE_TYPEDGOLUAMETHOD_CPP(SetObjectOrientation, "float float float")
	DEFINE_TYPEDGOLUAMETHOD_CPP(SetObjectScale, "float float float")
	DEFINE_GOLUAMETHOD_CPP(GetObjectName)
	DEFINE_TYPEDGOLUAMETHOD_CPP(SendObjectMessage, "string")
	DEFINE_TYPEDGOLUAMETHOD_CPP(ReceiveObjectMessage, "string function")
	DEFINE_TYPEDGOLUAMETHOD_CPP(GetChildObjectByName, "string")
	DEFINE_TYPEDGOLUAMETHOD_CPP(HasScriptListener, "string")
	DEFINE_TYPEDGOLUAMETHOD_CPP(FreeResources, "bool")
	DEFINE_GOLUAMETHOD_CPP(IsNpc)
	DEFINE_GOLUAMETHOD_CPP(GetParent)
	DEFINE_TYPEDGOLUAMETHOD_CPP(Object_Play3DSound, "string float float")		//audio file range loudness
	DEFINE_TYPEDGOLUAMETHOD_CPP(Object_GetDistToObject, "int")

};