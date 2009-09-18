#pragma once

#include "SGTIncludes.h"
#include "SGTGOCAI.h"
#include <list>
#include "SGTMessageSystem.h"

/*
Verwaltet AI Objekte, vergibt IDs und entscheidet, was aktualisiert wird und was nicht.
*/

class SGTDllExport SGTAIManager : public SGTMessageListener
{
private:
	std::list<SGTGOCAI*> mAIObjects;
	unsigned int mNextID;

public:
	SGTAIManager(void);
	~SGTAIManager(void);

	unsigned int RegisterAIObject(SGTGOCAI* object);
	void UnregisterAIObject(SGTGOCAI* object);

	SGTGOCAI* GetAIByID(unsigned int ID);

	void ReloadScripts();

	void Clear();

	void ReceiveMessage(SGTMsg &msg);
	void Update(float time);

	//Singleton
	static SGTAIManager& Instance();

};
