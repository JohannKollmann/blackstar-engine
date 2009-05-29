
#pragma once

#include "SGTIncludes.h"
#include "Ogre.h"
#include <list>
#include "SGTLoadSave.h"
#include <string>
#include "SGTSceneManager.h"
#include "SGTIncludes.h"
#include "SGTGOComponent.h"

typedef std::string go_id_type;

class SGTDllExport SGTGameObject : public SGTSaveable, public Ogre::UserDefinedObject
{
private:
	//Type identifier
	go_id_type mGOID;

	//Name
	Ogre::String mName;

	bool mSelectable;

	//Global and local transform
	Ogre::Vector3 mPosition;
	Ogre::Quaternion mOrientation;
	Ogre::Vector3 mLocalPosition;
	Ogre::Quaternion mLocalOrientation;
	Ogre::Vector3 mScale;

	//Components
	std::list<SGTGOComponent*> mComponents;

	//Messaging
	std::vector<Ogre::SharedPtr<SGTObjectMsg> > mCurrentMessages;

	//Parent and children
	SGTGameObject* mParent;
	std::list<SGTGameObject*> mChildren;

	void UpdateChildren();
	void UpdateLocalTransform();

public:
	SGTGameObject(SGTGameObject *parent = 0);
	~SGTGameObject();

	Ogre::String GetName() { return mName; }

	void SendMessage(Ogre::SharedPtr<SGTObjectMsg> msg);
	void SendInstantMessage(Ogre::String receiver_family, Ogre::SharedPtr<SGTObjectMsg> msg);
	void ProcessMessages();

	void AddComponent(SGTGOComponent* component);
	SGTGOComponent* GetComponent(const SGTGOComponent::goc_id_family& familyID);
	std::list<SGTGOComponent*>::iterator GetComponentIterator() { return mComponents.begin(); }
	std::list<SGTGOComponent*>::iterator GetComponentIteratorEnd() { return mComponents.end(); }
	void RemoveComponent(const SGTGOComponent::goc_id_family& familyID);
	void ClearGOCs();
	void ClearChildren();

	Ogre::Vector3 GetGlobalPosition() { return mPosition; }
	Ogre::Quaternion GetGlobalOrientation() { return mOrientation; }	
	Ogre::Vector3 GetGlobalScale() { return mScale; }
	void SetGlobalPosition(Ogre::Vector3 pos);
	void SetGlobalOrientation(Ogre::Quaternion quat);
	void SetGlobalScale(Ogre::Vector3 scale);
	void Translate(Ogre::Vector3 vec) { SetGlobalPosition(mPosition + vec); }
	void Rotate(Ogre::Vector3 axis, Ogre::Radian angle) { Ogre::Quaternion q; q.FromAngleAxis(angle, axis); SetGlobalOrientation(mOrientation * q); }
	void Rescale(Ogre::Vector3 scaleoffset) { SetGlobalScale(mScale + scaleoffset); }

	bool IsSelectable() { return mSelectable; }
	void SetSelectable(bool selectable) { mSelectable = selectable; }

	//Editor stuff
	void Freeze(bool freeze);
	void ShowEditorVisuals(bool show);

	void SetParent(SGTGameObject *parent);
	void RegisterChild(SGTGameObject *child);
	void UnregisterChild(SGTGameObject *child);
	std::list<SGTGameObject*> DetachChildren();

	unsigned int GetNumChildren()
	{
		return mChildren.size();
	}
	SGTGameObject* GetChild(unsigned short index);

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
	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "GameObject"; *pFn = (SGTSaveableInstanceFn)&NewInstance; }
	static SGTSaveable* NewInstance() { return new SGTGameObject; }
};