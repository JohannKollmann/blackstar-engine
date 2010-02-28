#pragma once

#include "IceIncludes.h"
#include "IceGOCAI.h"
#include <list>
#include "IceMessageSystem.h"
#include "IceNavigationMesh.h"

/*
Verwaltet AI Objekte, vergibt IDs und entscheidet, was aktualisiert wird und was nicht.
*/

namespace Ice
{

class DllExport AIManager : public MessageListener
{
	friend class SceneManager;
private:
	//maps script ID <=> ai object
	std::map<int, GOCAI*> mScriptAIBinds;

	//maps object ID <=> ai object
	std::map<int, GOCAI*> mAIObjects;

	NavigationMesh *mNavigationMesh;
	bool mLoadWayMeshAsObjects;

public:
	AIManager(void);
	~AIManager(void);

	void RegisterAIObject(GOCAI* object, int id);
	void UnregisterAIObject(int id);
	void RegisterScriptAIBind(GOCAI* object, int scriptID);
	void UnregisterScriptAIBind(int scriptID);

	void SetWayMeshLoadingMode(bool loadAsObjects) { mLoadWayMeshAsObjects = loadAsObjects; }
	bool GetWayMeshLoadingMode() { return mLoadWayMeshAsObjects; }

	GOCAI* GetAIByID(int ID);
	GOCAI* GetAIByScriptID(int scriptID);

	NavigationMesh* GetNavigationMesh();

	void ReloadScripts();

	void Clear();

	void ReceiveMessage(Msg &msg);
	void Update(float time);

	//Singleton
	static AIManager& Instance();

};

};