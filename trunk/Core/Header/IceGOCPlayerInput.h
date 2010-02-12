#pragma once

#include "IceGOComponent.h"
#include "IceIncludes.h"
#include "IceMessageListener.h"
#include "IceGOCCharacterController.h"

namespace Ice
{

class DllExport GOCPlayerInput : public CharacterControllerInput, public MessageListener, public GOCEditorInterface
{
private:
	bool mActive;

public:
	GOCPlayerInput(void);
	~GOCPlayerInput(void);

	void ReceiveMessage(Msg &msg);

	void SetActive(bool active);
	void Pause(bool pause);

	void ReceiveObjectMessage(Ogre::SharedPtr<ObjectMsg> msg);
	GOComponent::goc_id_type& GetComponentID() const { static std::string name = "PlayerInput"; return name; }

	std::string& TellName() { static std::string name = "PlayerInput"; return name; };
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "PlayerInput"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
	static LoadSave::Saveable* NewInstance() { return new GOCPlayerInput; };
	virtual void Save(LoadSave::SaveSystem& mgr) {};
	virtual void Load(LoadSave::LoadSystem& mgr) {};

	//Editor
	void CreateFromDataMap(DataMap *parameters) {};
	void GetParameters(DataMap *parameters) {};
	static void GetDefaultParameters(DataMap *parameters) {};
	static GOCEditorInterface* NewEditorInterfaceInstance() { return new GOCPlayerInput(); }
	void AttachToGO(GameObject *go); 
	Ogre::String GetLabel() { return "Player Input"; }
};

};