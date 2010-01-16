
#pragma once

#include "IceIncludes.h"
#include "Ogre.h"
#include <list>
#include "LoadSave.h"
#include <string>
#include "IceIncludes.h"
#include "IceGOComponent.h"

namespace Ice
{

typedef std::string go_id_type;

class DllExport GameObject : public LoadSave::Saveable, public Ogre::UserDefinedObject
{
private:
	//Unique ID
	int mID;

	//Name
	Ogre::String mName;

	bool mSelectable;
	bool mFreezePosition;
	bool mFreezeOrientation;

	bool mTransformingComponents;
	bool mTransformingChildren;
	bool mUpdatingFromParent;

	//Global and local transform
	Ogre::Vector3 mPosition;
	Ogre::Quaternion mOrientation;
	Ogre::Vector3 mLocalPosition;
	Ogre::Quaternion mLocalOrientation;
	Ogre::Vector3 mScale;

	//Components
	std::list<GOComponent*> mComponents;

	//Messaging
	std::vector<Ogre::SharedPtr<ObjectMsg> > mCurrentMessages;

	//Parent and children
	GameObject* mParent;
	std::list<GameObject*> mChildren;

	void UpdateChildren(bool move = true);
	void UpdateLocalTransform();

public:
	GameObject(GameObject *parent = 0);
	~GameObject();

	Ogre::String GetName() { return mName; }
	void SetName(Ogre::String name) { mName = name; }

	int GetID() { return mID; }
	Ogre::String GetIDStr() { return Ogre::StringConverter::toString(mID); }

	GameObject* GetParent() { return mParent; }

	void SendMessage(Ogre::SharedPtr<ObjectMsg> msg);
	void SendInstantMessage(Ogre::String receiver_family, Ogre::SharedPtr<ObjectMsg> msg);
	void ProcessMessages();

	void AddComponent(GOComponent* component);
	GOComponent* GetComponent(const GOComponent::goc_id_family& familyID);
	GOComponent* GetComponent(const GOComponent::goc_id_family& familyID, GOComponent::goc_id_type typeID);
	std::vector<Ogre::String> GetComponentsStr();
	std::list<GOComponent*>::iterator GetComponentIterator() { return mComponents.begin(); }
	std::list<GOComponent*>::iterator GetComponentIteratorEnd() { return mComponents.end(); }
	void RemoveComponent(const GOComponent::goc_id_family& familyID);
	void ClearGOCs();
	void ClearChildren();

	Ogre::Vector3 GetGlobalPosition() { return mPosition; }
	Ogre::Quaternion GetGlobalOrientation() { return mOrientation; }	
	Ogre::Vector3 GetGlobalScale() { return mScale; }
	void SetGlobalPosition(Ogre::Vector3 pos, bool updateChildren = true);
	void SetGlobalOrientation(Ogre::Quaternion quat, bool updateChildren = true);
	void SetGlobalScale(Ogre::Vector3 scale);
	void Translate(Ogre::Vector3 vec, bool updateChildren = true) { if (!mFreezePosition) SetGlobalPosition(mPosition + vec, updateChildren); }
	void Rotate(Ogre::Vector3 axis, Ogre::Radian angle, bool updateChildren = true) { if (!mFreezeOrientation) { Ogre::Quaternion q; q.FromAngleAxis(angle, axis); SetGlobalOrientation(mOrientation * q, updateChildren); } }
	void Rescale(Ogre::Vector3 scaleoffset) { SetGlobalScale(mScale + scaleoffset); }
	bool GetTranformingComponents() { return mTransformingComponents; }
	bool GetTranformingChildren() { return mTransformingChildren; }
	bool GetUpdatingFromParent() { return mUpdatingFromParent; }

	bool IsSelectable() { return mSelectable; }
	void SetSelectable(bool selectable) { mSelectable = selectable; }
	void SetFreezePosition(bool freeze) { mFreezePosition = freeze; }
	void SetFreezeOrientation(bool freeze) { mFreezeOrientation = freeze; }

	bool IsStatic();

	//Editor stuff
	void Freeze(bool freeze);
	void ShowEditorVisuals(bool show);

	void SetParent(GameObject *parent);
	void RegisterChild(GameObject *child);
	void UnregisterChild(GameObject *child);
	std::list<GameObject*> DetachChildren();

	unsigned int GetNumChildren()
	{
		return mChildren.size();
	}
	GameObject* GetChild(unsigned short index);

	/*
	Same as SetGlobalPosition and SetGlobalOrientation, but it doesn't update the components.
	*/
	void UpdateTransform(Ogre::Vector3 pos, Ogre::Quaternion quat);

	void OnParentChanged();

	//Load / Save
	std::string& TellName()
	{
		static std::string name = "GameObject"; return name;
	};
	void Save(LoadSave::SaveSystem& mgr);
	void Load(LoadSave::LoadSystem& mgr);
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "GameObject"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; }
	static LoadSave::Saveable* NewInstance() { return new GameObject; }
};

};