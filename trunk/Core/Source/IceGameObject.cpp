
#include "IceGameObject.h"
#include "IceSceneManager.h"
#include "IceGOCScriptMakros.h"


namespace Ice
{

	ManagedGameObject::ManagedGameObject()
	{
		mSelectable = true;
		mParent = nullptr;
		mLoadSaveByParent = true;
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
		mLoadSaveByParent = true;
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

	void GameObject::SendMessage(const Msg &msg)
	{
		mCurrentMessages.push_back(msg);
		if (mCurrentMessages.size() == 1) SceneManager::Instance().AddToMessageQueue(this);
	}

	void GameObject::SendInstantMessage(const Msg &msg)
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
		for (std::vector<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
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
			if (*i == child)
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
		std::vector<GameObject*> children_copy = mChildren;
		mChildren.clear();
		return children_copy;
	}

	void GameObject::AddComponent(GOComponent *component)
	{
		for (std::vector<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
		{
			if ((*i)->GetFamilyID() == component->GetFamilyID())
			{
				Ogre::LogManager::getSingleton().logMessage("Error in GameObject::AddComponent: Component already exists.");
				return;
			}
		}
		component->SetOwner(this);
		component->UpdatePosition(GetGlobalPosition());
		component->UpdateOrientation(GetGlobalOrientation());
		mComponents.push_back(component);
	}

	GOComponent* GameObject::GetComponent(const GOComponent::goc_id_family &familyID)
	{
		for (std::vector<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
		{
			if ((*i)->GetFamilyID() == familyID)
			{
				return (*i);
			}
		}
		return 0;
	}

	GOComponent* GameObject::GetComponent(const GOComponent::goc_id_family& familyID, GOComponent::goc_id_type typeID)
	{
		for (std::vector<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
		{
			if ((*i)->GetFamilyID() == familyID)
			{
				if ((*i)->GetComponentID() == typeID) return (*i);
				else return 0;
			}
		}
		return 0;
	}

	void GameObject::RemoveComponent(const GOComponent::goc_id_family &familyID)
	{
		for (std::vector<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
		{
			if ((*i)->GetFamilyID() == familyID)
			{
				GOComponent *component = (*i);
				mComponents.erase(i);
				delete component;
				return;
			}
		}
	}

	void GameObject::ClearGOCs()
	{
		std::vector<GOComponent*>::iterator i = mComponents.begin();
		while (mComponents.size() > 0)
		{
			GOComponent *component = (*i);
			mComponents.erase(i);
			delete component;
			i = mComponents.begin();
		}
	}

	void GameObject::ClearChildren()
	{
		std::vector<GameObject*>::iterator i = mChildren.begin();
		while (mChildren.size() > 0)
		{
			GameObject *child = (*i);
			delete child;	//Children rufen im Destruktor mParent->UnregisterChild(this) auf
			i = mChildren.begin();
		}
	}

	void GameObject::UpdateChildren(bool move)
	{
		for (std::vector<GameObject*>::iterator i = mChildren.begin(); i != mChildren.end(); i++)
		{
			if (move) (*i)->OnParentChanged();
			else (*i)->UpdateLocalTransform();
		}
	}

	GameObject* GameObject::GetChild(unsigned short index)
	{
		std::vector<GameObject*>::iterator i = mChildren.begin();
		for (unsigned short n = 0; n < index; n++) i++;
		return (*i);
	}

	void GameObject::SetGlobalPosition(Ogre::Vector3 pos, bool updateChildren)
	{
		mTransformingChildren = updateChildren;
		mPosition = pos;
		for (std::vector<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
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
		for (std::vector<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
		{
			(*i)->_updateOrientation(orientation);
		}
		if (mParent)
		{
			mLocalOrientation = mParent->GetGlobalOrientation().Inverse() * orientation;
		}
		UpdateChildren(updateChildren);
		mTransformingChildren = true;
	}

	void GameObject::SetGlobalScale(Ogre::Vector3 scale)
	{
		mScale = scale;
		for (std::vector<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
		{
			(*i)->_updateScale(scale);
		}
	}

	void GameObject::Freeze(bool freeze)
	{
		mFreezed = freeze;
		for (std::vector<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
		{
			(*i)->Freeze(freeze);
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
		if (mIgnoreParent) return;

		mUpdatingFromParent = true;
		mOrientation = mParent->GetGlobalOrientation() * mLocalOrientation;
		mPosition = mParent->GetGlobalOrientation() * mLocalPosition + mParent->GetGlobalPosition();
		for (std::vector<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
		{
			(*i)->UpdateOrientation(mOrientation);
			(*i)->UpdatePosition(mPosition);
		}
		UpdateChildren();
		mUpdatingFromParent = false;
	}

	bool GameObject::IsStatic()
	{
		for (std::vector<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
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
			if ((*i)->_getIsSaveable()) saveable_components.push_back(*i);
		mgr.SaveAtom("std::vector<Saveable*>", (void*)(&saveable_components), "mComponents");

		std::vector<GameObject*> managed_children;
		for (auto i = mChildren.begin(); i != mChildren.end(); i++)
			if ((*i)->mLoadSaveByParent) managed_children.push_back(*i);
		mgr.SaveAtom("std::vector<Saveable*>", (void*)(&managed_children), "mChildren");
	}

	void GameObject::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("Ogre::String", &mName);
		mgr.LoadAtom("Ogre::Vector3", &mPosition);
		mgr.LoadAtom("Ogre::Quaternion", &mOrientation);
		mgr.LoadAtom("Ogre::Vector3", &mScale);
		mgr.LoadAtom("bool", &mSelectable);
		mgr.LoadAtom("std::vector<Saveable*>", (void*)(&mComponents));
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

	std::vector<ScriptParam> GameObject::SetObjectProperty(std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		Ogre::String key = vParams[0].getString().c_str();
		mScriptProperties[key] = vParams[1];
		return out;
	}
	std::vector<ScriptParam> GameObject::GetObjectProperty(std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		Ogre::String key = vParams[0].getString().c_str();
		auto i = mScriptProperties.find(key);
		if (i == mScriptProperties.end()) return out;
		out.push_back(i->second);
		return out;
	}
	std::vector<ScriptParam> GameObject::SetObjectPosition(std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		float x = vParams[0].getFloat();
		float y = vParams[1].getFloat();
		float z = vParams[2].getFloat();
		SetGlobalPosition(Ogre::Vector3(x,y,z));
		return out;
	}
	std::vector<ScriptParam> GameObject::SetObjectOrientation(std::vector<ScriptParam> &vParams)
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
	std::vector<ScriptParam> GameObject::SetObjectScale(std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		float x = vParams[0].getFloat();
		float y = vParams[1].getFloat();
		float z = vParams[2].getFloat();
		SetGlobalScale(Ogre::Vector3(x,y,z));
		return out;
	}
	std::vector<ScriptParam> GameObject::GetObjectName(std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		return out;
	}

	DEFINE_TYPEDGOLUAMETHOD_CPP(SetObjectProperty, "string")
	DEFINE_TYPEDGOLUAMETHOD_CPP(GetObjectProperty, "string")
	DEFINE_TYPEDGOLUAMETHOD_CPP(SetObjectPosition, "float float float")
	DEFINE_TYPEDGOLUAMETHOD_CPP(SetObjectOrientation, "float float float")
	DEFINE_TYPEDGOLUAMETHOD_CPP(SetObjectScale, "float float float")
	DEFINE_GOLUAMETHOD_CPP(GetObjectName)

};