
#include "IceGameObject.h"
#include "IceSceneManager.h"


namespace Ice
{

GameObject::GameObject(GameObject *parent)
{
	mSelectable = true;
	mRegistered = true;
	mParent = parent;
	mID = SceneManager::Instance().RegisterObject(this);
	mName = "GameObject";
	if (mParent)
	{
		mParent->RegisterChild(this);
	}
	mScale = Ogre::Vector3(1,1,1);
	mPosition = Ogre::Vector3(0,0,0);
	mOrientation = Ogre::Quaternion();
	UpdateLocalTransform();
	mFreezePosition = false;
	mFreezeOrientation = false;
	mTransformingComponents = false;
	mTransformingChildren = false;
	mUpdatingFromParent = false;
}
GameObject::GameObject(int id, GameObject *parent)
{
	mSelectable = true;
	mRegistered = false;
	mParent = parent;
	if (id != -1) mID = id;
	else mID = SceneManager::Instance().RequestID();
	mName = "GameObject";
	if (mParent)
	{
		mParent->RegisterChild(this);
	}
	mScale = Ogre::Vector3(1,1,1);
	mPosition = Ogre::Vector3(0,0,0);
	mOrientation = Ogre::Quaternion();
	UpdateLocalTransform();
	mFreezePosition = false;
	mFreezeOrientation = false;
	mTransformingComponents = false;
	mTransformingChildren = false;
	mUpdatingFromParent = false;
}

GameObject::~GameObject()
{
	ClearChildren();
	ClearGOCs();
	if (mRegistered) SceneManager::Instance().UnregisterObject(mID);
	if (mParent)
	{
		mParent->UnregisterChild(this);
	}
}

void GameObject::SendMessage(Ogre::SharedPtr<ObjectMsg> msg)
{
	mCurrentMessages.push_back(msg);
	if (mCurrentMessages.size() == 1) SceneManager::Instance().AddToMessageQueue(this);
}

void GameObject::SendInstantMessage(Ogre::String receiver_family, Ogre::SharedPtr<ObjectMsg> msg)
{
	for (std::list<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
	{
		if ((*i)->GetFamilyID() == receiver_family || receiver_family == "")
		{
			(*i)->ReceiveObjectMessage(msg);
		}
	}
}

void GameObject::ProcessMessages()
{
	std::vector<Ogre::SharedPtr<ObjectMsg> > msgcopy = mCurrentMessages;
	mCurrentMessages.clear();
	for (std::list<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
	{
		for (std::vector<Ogre::SharedPtr<ObjectMsg> >::iterator x = msgcopy.begin(); x != msgcopy.end(); x++)
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
}

void GameObject::UnregisterChild(GameObject *child)
{
	mChildren.remove(child);
}

std::list<GameObject*> GameObject::DetachChildren()
{
	std::list<GameObject*> children_copy = mChildren;
	mChildren.clear();
	return children_copy;
}

void GameObject::AddComponent(GOComponent *component)
{
	for (std::list<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
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
	for (std::list<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
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
	for (std::list<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
	{
		if ((*i)->GetFamilyID() == familyID)
		{
			if ((*i)->GetComponentID() == typeID) return (*i);
			else return 0;
		}
	}
	return 0;
}

std::vector<Ogre::String> GameObject::GetComponentsStr()
{
	std::vector<Ogre::String> returner;
	for (std::list<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
	{
		returner.push_back((*i)->GetFamilyID());
	}
	return returner;
}

void GameObject::RemoveComponent(const GOComponent::goc_id_family &familyID)
{
	for (std::list<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
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
	std::list<GOComponent*>::iterator i = mComponents.begin();
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
	std::list<GameObject*>::iterator i = mChildren.begin();
	while (mChildren.size() > 0)
	{
		GameObject *child = (*i);
		delete child;	//Children rufen im Destruktor mParent->UnregisterChild(this) auf
		i = mChildren.begin();
	}
}

void GameObject::UpdateChildren(bool move)
{
	for (std::list<GameObject*>::iterator i = mChildren.begin(); i != mChildren.end(); i++)
	{
		if (move) (*i)->OnParentChanged();
		else (*i)->UpdateLocalTransform();
	}
}

GameObject* GameObject::GetChild(unsigned short index)
{
	std::list<GameObject*>::iterator i = mChildren.begin();
	for (unsigned short n = 0; n < index; n++) i++;
	return (*i);
}

void GameObject::SetGlobalPosition(Ogre::Vector3 pos, bool updateChildren)
{
	mTransformingChildren = updateChildren;
	mPosition = pos;
	for (std::list<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
	{
		(*i)->UpdatePosition(pos);
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
	for (std::list<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
	{
		(*i)->UpdateOrientation(orientation);
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
	for (std::list<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
	{
		(*i)->UpdateScale(scale);
	}
}

void GameObject::Freeze(bool freeze)
{
	for (std::list<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
	{
		(*i)->Freeze(freeze);
	}
}
void GameObject::ShowEditorVisuals(bool show)
{
	for (std::list<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
	{
		(*i)->ShowEditorVisual(show);
	}
}

void GameObject::UpdateTransform(Ogre::Vector3 pos, Ogre::Quaternion orientation)
{
	mPosition = pos;
	mOrientation = orientation;
	mTransformingComponents = true;
	for (std::list<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
	{
		(*i)->UpdateOrientation(mOrientation);
		(*i)->UpdatePosition(mPosition);
	}
	mTransformingComponents = false;
	UpdateLocalTransform();
	UpdateChildren();
}

void GameObject::OnParentChanged()
{
	mUpdatingFromParent = true;
	mOrientation = mParent->GetGlobalOrientation() * mLocalOrientation;
	mPosition = mParent->GetGlobalOrientation() * mLocalPosition + mParent->GetGlobalPosition();
	for (std::list<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
	{
		(*i)->UpdateOrientation(mOrientation);
		(*i)->UpdatePosition(mPosition);
	}
	UpdateChildren();
	mUpdatingFromParent = false;
}

bool GameObject::IsStatic()
{
	for (std::list<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
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
	mgr.SaveAtom("std::list<Saveable*>", (void*)(&mComponents), "mComponents");
	mgr.SaveAtom("std::list<Saveable*>", (void*)(&mChildren), "mChildren");
}

void GameObject::Load(LoadSave::LoadSystem& mgr)
{
	mgr.LoadAtom("Ogre::String", &mName);
	mgr.LoadAtom("Ogre::Vector3", &mPosition);
	mgr.LoadAtom("Ogre::Quaternion", &mOrientation);
	mgr.LoadAtom("Ogre::Vector3", &mScale);
	mgr.LoadAtom("bool", &mSelectable);
	mgr.LoadAtom("std::list<Saveable*>", (void*)(&mComponents));
	for (std::list<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
	{
		(*i)->SetOwner(this);
		(*i)->UpdatePosition(GetGlobalPosition());
		(*i)->UpdateOrientation(GetGlobalOrientation());
	}
	mgr.LoadAtom("std::list<Saveable*>", (void*)(&mChildren));
	for (std::list<GameObject*>::iterator i = mChildren.begin(); i != mChildren.end(); i++)
	{
		(*i)->SetParent(this);
	}
}

};