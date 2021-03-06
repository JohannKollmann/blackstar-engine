#pragma once

#include "IceGOComponent.h"
#include "IceIncludes.h"
#include "IceInput.h"
#include "IceMessageListener.h"
#include "IceGOCCharacterController.h"

namespace Ice
{

class DllExport GOCPlayerInput : public CharacterControllerInput, public GOCEditorInterface
{
private:
	bool mActive;

public:
	GOCPlayerInput(void);
	~GOCPlayerInput(void);

	AccessPermissionID GetAccessPermissionID() { return AccessPermissions::ACCESS_PHYSICS; }

	void ReceiveMessage(Msg &msg);

	void SetActive(bool active);
	void Pause(bool pause);

	GOComponent::TypeID& GetComponentID() const { static std::string name = "PlayerInput"; return name; }

	std::string& TellName() { static std::string name = "PlayerInput"; return name; };
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "PlayerInput"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
	static LoadSave::Saveable* NewInstance() { return new GOCPlayerInput; };
	virtual void Save(LoadSave::SaveSystem& mgr) {};
	virtual void Load(LoadSave::LoadSystem& mgr) {};

	//Editor
	void SetParameters(DataMap *parameters) {};
	void GetParameters(DataMap *parameters) {};
	void GetDefaultParameters(DataMap *parameters) {};
	GOCEditorInterface* New() { return new GOCPlayerInput(); }
	Ogre::String GetLabel() { return "Player Input"; }
	GOComponent* GetGOComponent() { return this; }
};

};