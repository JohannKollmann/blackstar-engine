
#pragma once

#include "SGTIncludes.h"
#include <list>
#include "SGTGOCIntern.h"
#include "NxPhysics.h"

class SGTDllExport SGTPathfinder
{
private:
	std::list<SGTGOCWaypoint*> mWaynet;

	bool UpdateEdgeList(WPEdge &e, std::list<WPEdge> *WPEdges);
	WPEdge GetBestEdge(std::list<WPEdge> *WPEdges);
	bool ExtractPath(std::list<WPEdge> paths, SGTGOCWaypoint *start, SGTGOCWaypoint *target, std::vector<Ogre::Vector3> *returnpath);

public:
	SGTPathfinder(void);
	~SGTPathfinder(void);

	void RegisterWaypoint(SGTGOCWaypoint *waypoint);
	void UnregisterWaypoint(SGTGOCWaypoint *waypoint);

	SGTGOCWaypoint* GetWPByName(Ogre::String name);
	SGTGOCWaypoint* GetNextWP(Ogre::Vector3 position, std::vector<SGTGOCWaypoint*> excludeList);

	bool FindPath(Ogre::Vector3 position, Ogre::String targetWP, std::vector<Ogre::Vector3> *path, NxActor *actor = 0);
	bool FindPath(Ogre::String startWP, Ogre::String targetWP, std::vector<Ogre::Vector3> *path);
	bool FindPath(SGTGOCWaypoint *start, SGTGOCWaypoint *target, std::vector<Ogre::Vector3> *path);

	//Singleton
	static SGTPathfinder& Instance();
};
