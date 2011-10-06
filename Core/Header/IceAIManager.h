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

class DllExport AIManager : public IndependantMessageListener
{
	friend class SceneManager;

private:

	std::vector<GOCWaypoint*> mWaypoints;

	//maps object ID <=> ai object
	std::vector<GOCAI*> mAIObjects;

	NavigationMesh *mNavigationMesh;

public:
	AIManager(void);
	~AIManager(void);

	void RegisterWaypoint(GOCWaypoint *waypoint);
	void UnregisterWaypoint(GOCWaypoint *waypoint);
	GOCWaypoint* GetWPByName(Ogre::String name);
	void FindPath(Ogre::Vector3 origin, Ogre::String targetWP, std::vector<AStarNode3D*> &oPath);

	void RegisterAIObject(GOCAI* object);
	void UnregisterAIObject(GOCAI* object);

	NavigationMesh* GetNavigationMesh();
	void SetNavigationMesh(NavigationMesh *mesh);

	void Clear();
	void Shutdown();

	void ReceiveMessage(Msg &msg);
	void Update(float time);

	//Singleton
	static AIManager& Instance();

};

};