
#pragma once

#include "IceGOComponent.h"
#include "IceIncludes.h"
#include "IceGOCCharacterController.h"
#include "IceAIState.h"
#include "IceScriptedAIState.h"
#include "IceDataMap.h"
#include "IceScriptSystem.h"

namespace Ice
{

	class DllExport GOCAI : public GOCEditorInterface, public CharacterControllerInput, public MessageListener
{
private:
	/*
	Die Idle-Routine des AI Objekts (Tagesablauf), wird immer gelooped.
	*/
	std::vector<DayCycle*> mIdleQueue;

	/*
	Priority Queue von Events, ueberlagert immer Idle-Routine.
	*/
	std::vector<AIState*> mActionQueue;


	Ogre::String mScriptFileName;

	//Lua stuff
	std::map<std::string, ScriptParam> mProperties;

public:
	GOCAI(void);
	~GOCAI(void);

	void AddState(AIState *state);
	void AddDayCycleState(DayCycle *state);
	void ClearActionQueue();
	void ClearIdleQueue();
	void SelectState();
	void LeaveActiveActionState();

	void ReloadScript();

	void SetOwner(GameObject *go);

	void SetProperty(std::string key, ScriptParam prop);
	ScriptParam GetProperty(std::string key);

	int GetID();

	void Update(float time);

	void ReceiveObjectMessage(const Msg &msg);

	void ReceiveMessage(Msg &msg);

	GOComponent::goc_id_type& GetComponentID() const { static std::string name = "AI"; return name; }

	Ogre::String getTypeName() { return "AI"; }

	void CreateFromDataMap(DataMap *parameters);
	void GetParameters(DataMap *parameters);
	static void GetDefaultParameters(DataMap *parameters);
	static GOCEditorInterface* NewEditorInterfaceInstance() { return new GOCAI(); }
	void AttachToGO(GameObject *go);
	Ogre::String GetLabel() { return "Script"; }

	bool IsStatic() { return false; }

	void Create(Ogre::String scriptFile);
	std::string& TellName() { static std::string name = "AI"; return name; };
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "AI"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
	static LoadSave::Saveable* NewInstance() { return new GOCAI; };
	void Save(LoadSave::SaveSystem& mgr);
	void Load(LoadSave::LoadSystem& mgr);

	Script mScript;
};

};