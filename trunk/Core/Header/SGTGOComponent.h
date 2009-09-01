
#pragma once

#include "SGTLoadSave.h"
#include "SGTIncludes.h"
#include "Ogre.h"
#include "SGTDataMap.h"

struct SGTObjectMsg
{
	Ogre::String mName;
	SGTDataMap mData;
};

class SGTDllExport SGTGOComponent : public SGTSaveable
{
protected:
	SGTGameObject *mOwnerGO;
	bool mRenderEditorVisuals;

public:
	typedef std::string goc_id_type;
	typedef std::string goc_id_family;

	SGTGOComponent() : mOwnerGO(0) { mRenderEditorVisuals = true; }
	virtual ~SGTGOComponent() {}

	virtual goc_id_type& GetComponentID() const = 0;
	virtual goc_id_family& GetFamilyID() const = 0;

	virtual void SetOwner(SGTGameObject *go);
	SGTGameObject* GetOwner() const { return mOwnerGO; }

	//Called by GO Owner
	virtual void UpdatePosition(Ogre::Vector3 position) {}
	virtual void UpdateOrientation(Ogre::Quaternion orientation) {}
	virtual void UpdateScale(Ogre::Vector3 scale) {}

	virtual bool IsStatic() { return true; }

	//Messaging
	virtual void ReceiveObjectMessage(Ogre::SharedPtr<SGTObjectMsg> msg) {}

	//Editor stuff
	virtual void Freeze(bool freeze) {};
	virtual void ShowEditorVisual(bool show) { mRenderEditorVisuals = show; };

	//Load / Save
	/*virtual std::string& TellName()
	{
		return GetComponentID();
	};
	virtual void Save(SGTSaveSystem& mgr) {};
	virtual void Load(SGTLoadSystem& mgr) {};*/
};