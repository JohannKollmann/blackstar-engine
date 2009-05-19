
#include "SGTGameObject.h"

SGTGameObject::SGTGameObject(SGTGameObject *parent)
{
	mParent = parent;
	mName = "GameObject_" + Ogre::StringConverter::toString(SGTSceneManager::Instance().RequestID());
	SGTSceneManager::Instance().mGameObjects.push_back(this);
	if (mParent)
	{
		mParent->RegisterChild(this);
	}
	mScale = Ogre::Vector3(1,1,1);
	mPosition = Ogre::Vector3(0,0,0);
	mOrientation = Ogre::Quaternion();
	UpdateLocalTransform();
}

SGTGameObject::~SGTGameObject()
{
	ClearChildren();
	ClearGOCs();
	SGTSceneManager::Instance().mGameObjects.remove(this);
	if (mParent)
	{
		mParent->UnregisterChild(this);
	}
}

void SGTGameObject::SendMessage(Ogre::SharedPtr<SGTObjectMsg> msg)
{
	mCurrentMessages.push_back(msg);
}

void SGTGameObject::SendInstantMessage(Ogre::String receiver_family, Ogre::SharedPtr<SGTObjectMsg> msg)
{
	for (std::list<SGTGOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
	{
		if ((*i)->GetFamilyID() == receiver_family)
		{
			(*i)->ReceiveObjectMessage(msg);
		}
	}
}

void SGTGameObject::ProcessMessages()
{
	std::vector<Ogre::SharedPtr<SGTObjectMsg> > msgcopy = mCurrentMessages;
	mCurrentMessages.clear();
	for (std::list<SGTGOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
	{
		for (std::vector<Ogre::SharedPtr<SGTObjectMsg> >::iterator x = msgcopy.begin(); x != msgcopy.end(); x++)
		{
			(*i)->ReceiveObjectMessage((*x));
		}
	}
	msgcopy.clear();
}

void SGTGameObject::UpdateLocalTransform()
{
	if (mParent)
	{
		mLocalPosition = mPosition - mParent->GetGlobalPosition();
		mLocalOrientation = mParent->GetGlobalOrientation().Inverse() * mOrientation;
	}
}

void SGTGameObject::SetParent(SGTGameObject *parent)
{
	if (mParent)
	{
		mParent->UnregisterChild(this);
	}
	mParent = parent;
	mParent->RegisterChild(this);
	UpdateLocalTransform();
}

void SGTGameObject::RegisterChild(SGTGameObject *child)
{
	mChildren.push_back(child);
}

void SGTGameObject::UnregisterChild(SGTGameObject *child)
{
	mChildren.remove(child);
}

std::list<SGTGameObject*> SGTGameObject::DetachChildren()
{
	std::list<SGTGameObject*> children_copy = mChildren;
	mChildren.clear();
	return children_copy;
}

void SGTGameObject::AddComponent(SGTGOComponent *component)
{
	for (std::list<SGTGOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
	{
		if ((*i)->GetFamilyID() == component->GetFamilyID())
		{
			Ogre::LogManager::getSingleton().logMessage("Error in SGTGameObject::AddComponent: Component already exists.");
			return;
		}
	}
	component->SetOwner(this);
	component->UpdatePosition(GetGlobalPosition());
	component->UpdateOrientation(GetGlobalOrientation());
	mComponents.push_back(component);
}

SGTGOComponent* SGTGameObject::GetComponent(const SGTGOComponent::goc_id_family &familyID)
{
	for (std::list<SGTGOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
	{
		if ((*i)->GetFamilyID() == familyID)
		{
			return (*i);
		}
	}
	return 0;
}

void SGTGameObject::RemoveComponent(const SGTGOComponent::goc_id_family &familyID)
{
	for (std::list<SGTGOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
	{
		if ((*i)->GetFamilyID() == familyID)
		{
			SGTGOComponent *component = (*i);
			mComponents.erase(i);
			delete component;
			return;
		}
	}
}

void SGTGameObject::ClearGOCs()
{
	std::list<SGTGOComponent*>::iterator i = mComponents.begin();
	while (mComponents.size() > 0)
	{
		SGTGOComponent *component = (*i);
		mComponents.erase(i);
		delete component;
		i = mComponents.begin();
	}
}

void SGTGameObject::ClearChildren()
{
	std::list<SGTGameObject*>::iterator i = mChildren.begin();
	while (mChildren.size() > 0)
	{
		SGTGameObject *child = (*i);
		delete child;	//Children rufen im Destruktor mParent->UnregisterChild(this) auf
		i = mChildren.begin();
	}
}

void SGTGameObject::UpdateChildren()
{
	for (std::list<SGTGameObject*>::iterator i = mChildren.begin(); i != mChildren.end(); i++)
	{
		(*i)->OnParentChanged();
	}
}

SGTGameObject* SGTGameObject::GetChild(unsigned short index)
{
	std::list<SGTGameObject*>::iterator i = mChildren.begin();
	for (unsigned short n = 0; n < index; n++) i++;
	return (*i);
}

void SGTGameObject::SetGlobalPosition(Ogre::Vector3 pos)
{
	mPosition = pos;
	for (std::list<SGTGOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
	{
		(*i)->UpdatePosition(pos);
	}
	if (mParent)
	{
		mLocalPosition = pos - mParent->GetGlobalPosition();
	}
	UpdateLocalTransform();
	UpdateChildren();
}

void SGTGameObject::SetGlobalOrientation(Ogre::Quaternion orientation)
{
	mOrientation = orientation;
	for (std::list<SGTGOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
	{
		(*i)->UpdateOrientation(orientation);
	}
	if (mParent)
	{
		mLocalOrientation = mParent->GetGlobalOrientation().Inverse() * orientation;
	}
	UpdateLocalTransform();
	UpdateChildren();
}

void SGTGameObject::SetGlobalScale(Ogre::Vector3 scale)
{
	mScale = scale;
	for (std::list<SGTGOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
	{
		(*i)->UpdateScale(scale);
	}
}

void SGTGameObject::Freeze(bool freeze)
{
	for (std::list<SGTGOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
	{
		(*i)->Freeze(freeze);
	}
}
void SGTGameObject::ShowEditorVisuals(bool show)
{
	for (std::list<SGTGOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
	{
		(*i)->ShowEditorVisual(show);
	}
}

void SGTGameObject::UpdateTransform(Ogre::Vector3 pos, Ogre::Quaternion orientation)
{
	mPosition = pos;
	mOrientation = orientation;
	UpdateLocalTransform();
	UpdateChildren();
}

void SGTGameObject::OnParentChanged()
{
	SetGlobalOrientation(mParent->GetGlobalOrientation() * mLocalOrientation);
	SetGlobalPosition(mParent->GetGlobalOrientation() * mLocalPosition + mParent->GetGlobalPosition());
	UpdateChildren();
}

void SGTGameObject::Save(SGTSaveSystem& mgr)
{
	mgr.SaveAtom("Ogre::String", (void*)(&mGOID), "ID");
	mgr.SaveAtom("Ogre::Vector3", (void*)(&mPosition), "Position");
	mgr.SaveAtom("Ogre::Quaternion", (void*)(&mOrientation), "Orientation");
	mgr.SaveAtom("Ogre::Vector3", (void*)(&mScale), "Scale");
	mgr.SaveAtom("std::list<SGTSaveable*>", (void*)(&mComponents), "mComponents");
	mgr.SaveAtom("std::list<SGTSaveable*>", (void*)(&mChildren), "mComponents");
}

void SGTGameObject::Load(SGTLoadSystem& mgr)
{
	mgr.LoadAtom("Ogre::String", &mGOID);
	mgr.LoadAtom("Ogre::Vector3", &mPosition);
	mgr.LoadAtom("Ogre::Quaternion", &mOrientation);
	mgr.LoadAtom("Ogre::Vector3", &mScale);
	mgr.LoadAtom("std::list<SGTSaveable*>", (void*)(&mComponents));
	for (std::list<SGTGOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
	{
		(*i)->SetOwner(this);
		(*i)->UpdatePosition(GetGlobalPosition());
		(*i)->UpdateOrientation(GetGlobalOrientation());
	}
	mgr.LoadAtom("std::list<SGTSaveable*>", (void*)(&mChildren));
	for (std::list<SGTGameObject*>::iterator i = mChildren.begin(); i != mChildren.end(); i++)
	{
		(*i)->SetParent(this);
	}
}