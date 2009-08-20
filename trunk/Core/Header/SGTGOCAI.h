
#pragma once

#include "SGTGOComponent.h"
#include "SGTIncludes.h"
#include "SGTMessageListener.h"
#include "SGTGOCCharacterController.h"
#include "SGTAIState.h"
#include "SGTScriptedAIState.h"

class SGTDllExport SGTGOCAI : public SGTCharacterControllerInput, public SGTMessageListener
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

public:
	SGTGOCAI(void);
	~SGTGOCAI(void);

	void AddState(SGTAIState *state);
	void ClearActionQueue();
	void ClearIdleQueue();
	void SelectState();

	void ReceiveMessage(SGTMsg &msg);
	void ReceiveObjectMessage(Ogre::SharedPtr<SGTObjectMsg> msg);

	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "AI"; return name; }

	std::string& TellName() { static std::string name = "AI"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "AI"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTSaveable* NewInstance() { return new SGTGOCAI; };
	virtual void Save(SGTSaveSystem& mgr) {};
	virtual void Load(SGTLoadSystem& mgr) {};
};