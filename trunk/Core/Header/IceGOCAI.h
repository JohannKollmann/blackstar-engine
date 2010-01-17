
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
	Die Idle-Routine des AI Objekts (Tagesablauf).
	*/
	std::list<DayCycle*> mIdleQueue;

	/*
	Die aktive Queue nach dem FIFO Prinzip mit Priorisierung, ueberlagert immer Idle-Routine.
	Die einzige Möglichkeit, tatsaechlich etwas zu tun. Die gescripteten Idle Routinen (Tagesablaeufe)
	befuellen die ActionQueue.
	*/
	std::list<AIState*> mActionQueue;

	
	/*
	Der aktuelle AIState aus der ActionQueue.
	*/
	AIState *mActiveState;

	Script mScript;
	Ogre::String mScriptFileName;

	//Lua stuff
	std::map<std::string, ScriptParam> mProperties;

public:
	GOCAI(void);
	~GOCAI(void);

	void AddState(AIState *state);
	void AddScriptedState(DayCycle *state);
	void ClearActionQueue();
	void ClearIdleQueue();
	void SelectState();

	void ReloadScript();

	void SetOwner(GameObject *go);

	void SetProperty(std::string key, ScriptParam prop);
	ScriptParam GetProperty(std::string key);

	int GetID();

	void Update(float time);

	void ReceiveObjectMessage(Ogre::SharedPtr<ObjectMsg> msg);

	void ReceiveMessage(Msg &msg);

	GOComponent::goc_id_type& GetComponentID() const { static std::string name = "AI"; return name; }

	void CreateFromDataMap(DataMap *parameters);
	void GetParameters(DataMap *parameters);
	static void GetDefaultParameters(DataMap *parameters);
	static GOCEditorInterface* NewEditorInterfaceInstance() { return new GOCAI(); }
	void AttachToGO(GameObject *go);
	Ogre::String GetLabel() { return "Scripted AI"; }

	bool IsStatic() { return false; }

	void Create(Ogre::String scriptFile);
	std::string& TellName() { static std::string name = "AI"; return name; };
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "AI"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
	static LoadSave::Saveable* NewInstance() { return new GOCAI; };
	void Save(LoadSave::SaveSystem& mgr);
	void Load(LoadSave::LoadSystem& mgr);
};

};