#pragma once

#include "IceIncludes.h"
#include "IceGOCAI.h"
#include <list>
#include "IceMessageSystem.h"

/*
Verwaltet AI Objekte, vergibt IDs und entscheidet, was aktualisiert wird und was nicht.
*/

namespace Ice
{

class DllExport AIManager : public MessageListener
{
private:
	std::map<int, GOCAI*> mAIObjects;
	unsigned int mNextID;

public:
	AIManager(void);
	~AIManager(void);

	unsigned int RegisterAIObject(GOCAI* object);
	void UnregisterAIObject(GOCAI* object);

	GOCAI* GetAIByID(unsigned int ID);

	void ReloadScripts();

	void Clear();

	void ReceiveMessage(Msg &msg);
	void Update(float time);

	//Singleton
	static AIManager& Instance();

};

};