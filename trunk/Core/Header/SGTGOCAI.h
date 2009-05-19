
#pragma once

#include "SGTGOComponent.h"
#include "SGTIncludes.h"
#include "SGTMessageListener.h"

class SGTDllExport SGTGOCAI : public SGTGOComponent, public SGTMessageListener
{
private:
	Ogre::Vector3 mDirection;

public:
	SGTGOCAI(void);
	~SGTGOCAI(void);

	void ReceiveMessage(SGTMsg &msg);

	goc_id_family& GetFamilyID() const { static std::string name = "GOCAI"; return name; }
	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "AI"; return name; }

	std::string& TellName() { static std::string name = "AI"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "AI"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTGOCAI* NewInstance() { return new SGTGOCAI; };
};