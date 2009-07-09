
#pragma once

#include "SGTGOComponent.h"
#include "SGTIncludes.h"
#include "SGTMessageListener.h"
#include "SGTGOCCharacterController.h"

class SGTDllExport SGTGOCAI : public SGTCharacterControllerInput, public SGTMessageListener
{
public:
	SGTGOCAI(void);
	~SGTGOCAI(void);

	void ReceiveMessage(SGTMsg &msg);
	void ReceiveObjectMessage(Ogre::SharedPtr<SGTObjectMsg> msg);

	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "AI"; return name; }

	std::string& TellName() { static std::string name = "AI"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "AI"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTSaveable* NewInstance() { return new SGTGOCAI; };
	virtual void Save(SGTSaveSystem& mgr) {};
	virtual void Load(SGTLoadSystem& mgr) {};
};