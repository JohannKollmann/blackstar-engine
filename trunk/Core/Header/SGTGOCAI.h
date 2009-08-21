
#pragma once

#include "SGTGOComponent.h"
#include "SGTIncludes.h"
#include "SGTMessageListener.h"
#include "SGTGOCCharacterController.h"
#include "SGTAIState.h"
#include "SGTScriptedAIState.h"

class SGTDllExport SGTGOCAI : public SGTCharacterControllerInput
{
private:
	/*
	Die Idle-Routine des AI Objekts (Tagesablauf).
	*/
	std::list<SGTScriptedAIState*> mIdleQueue;

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

	/*
	Fuer Lua.
	*/
	unsigned int mID;

public:
	SGTGOCAI(void);
	~SGTGOCAI(void);

	void AddState(SGTAIState *state);
	void AddScriptedState(SGTScriptedAIState *state);
	void ClearActionQueue();
	void ClearIdleQueue();
	void SelectState();

	unsigned int GetID() { return mID; }

	void Update(float time);

	void ReceiveObjectMessage(Ogre::SharedPtr<SGTObjectMsg> msg);

	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "AI"; return name; }

	std::string& TellName() { static std::string name = "AI"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "AI"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTSaveable* NewInstance() { return new SGTGOCAI; };
	virtual void Save(SGTSaveSystem& mgr) {};
	virtual void Load(SGTLoadSystem& mgr) {};
};