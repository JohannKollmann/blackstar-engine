
#pragma once

#include "LoadSave.h"
#include "IceIncludes.h"
#include "Ogre.h"
#include "IceDataMap.h"

namespace Ice
{

struct ObjectMsg
{
	Ogre::String mName;
	DataMap mData;
	void *rawData;
};

class DllExport GOComponent : public LoadSave::Saveable
{
protected:
	GameObject *mOwnerGO;
	bool mRenderEditorVisuals;

public:
	typedef std::string goc_id_type;
	typedef std::string goc_id_family;

	GOComponent() : mOwnerGO(0) { mRenderEditorVisuals = true; }
	virtual ~GOComponent() {}

	virtual goc_id_type& GetComponentID() const = 0;
	virtual goc_id_family& GetFamilyID() const = 0;

	virtual void SetOwner(GameObject *go);
	GameObject* GetOwner() const { return mOwnerGO; }

	//Called by GO Owner
	virtual void UpdatePosition(Ogre::Vector3 position) {}
	virtual void UpdateOrientation(Ogre::Quaternion orientation) {}
	virtual void UpdateScale(Ogre::Vector3 scale) {}

	virtual bool IsStatic() { return true; }

	//Messaging
	virtual void ReceiveObjectMessage(Ogre::SharedPtr<ObjectMsg> msg) {}

	//Editor stuff
	virtual void Freeze(bool freeze) {};
	virtual void ShowEditorVisual(bool show) { mRenderEditorVisuals = show; };

	//Load / Save
	/*virtual std::string& TellName()
	{
		return GetComponentID();
	};
	virtual void Save(LoadSave::SaveSystem& mgr) {};
	virtual void Load(LoadSave::LoadSystem& mgr) {};*/
};

};