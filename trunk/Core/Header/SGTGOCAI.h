
#pragma once

#include "SGTGOComponent.h"
#include "SGTIncludes.h"
#include "SGTMessageListener.h"
#include "SGTGOCCharacterController.h"
#include "SGTAIState.h"
#include "SGTScriptedAIState.h"
#include "SGTDataMap.h"

class SGTDllExport SGTGOCAI : public SGTGOCEditorInterface, public SGTCharacterControllerInput
{
private:
	/*
	Die Idle-Routine des AI Objekts (Tagesablauf).
	*/
	std::list<SGTDayCycle*> mIdleQueue;

	/*
	Die aktive Queue nach dem FIFO Prinzip mit Priorisierung, ueberlagert immer Idle-Routine.
	Die einzige Möglichkeit, tatsaechlich etwas zu tun. Die gescripteten Idle Routinen (Tagesablaeufe)
	befuellen die ActionQueue.
	*/
	std::list<SGTAIState*> mActionQueue;

	
	/*
	Der aktuelle AIState aus der ActionQueue.
	*/
	SGTAIState *mActiveState;

	SGTScript mScript;
	Ogre::String mScriptFileName;

	//Lua stuff
	std::map<std::string, SGTScriptParam> mProperties;
	unsigned int mID;

public:
	SGTGOCAI(void);
	~SGTGOCAI(void);

	void AddState(SGTAIState *state);
	void AddScriptedState(SGTDayCycle *state);
	void ClearActionQueue();
	void ClearIdleQueue();
	void SelectState();

	void ReloadScript();

	void SetOwner(SGTGameObject *go);

	void SetProperty(std::string key, SGTScriptParam prop);
	SGTScriptParam GetProperty(std::string key);

	unsigned int GetID() { return mID; }

	void Update(float time);

	void ReceiveObjectMessage(Ogre::SharedPtr<SGTObjectMsg> msg);

	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "AI"; return name; }

	void CreateFromDataMap(SGTDataMap *parameters);
	void GetParameters(SGTDataMap *parameters);
	static void GetDefaultParameters(SGTDataMap *parameters);
	bool IsViewComponent() { return false; }
	static SGTGOCEditorInterface* NewEditorInterfaceInstance() { return new SGTGOCAI(); }

	bool IsStatic() { return false; }

	void Create(Ogre::String scriptFile);
	std::string& TellName() { static std::string name = "AI"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "AI"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTSaveable* NewInstance() { return new SGTGOCAI; };
	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
};