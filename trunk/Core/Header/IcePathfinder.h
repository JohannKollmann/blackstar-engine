
#pragma once

#include "IceIncludes.h"
#include <list>
#include "IceGOCIntern.h"
#include "NxPhysics.h"

namespace Ice
{

class DllExport Pathfinder
{
private:
	std::list<GOCWaypoint*> mWaynet;

	bool UpdateEdgeList(WPEdge &e, std::list<WPEdge> *WPEdges);
	WPEdge GetBestEdge(std::list<WPEdge> *WPEdges);
	bool ExtractPath(std::list<WPEdge> paths, GOCWaypoint *start, GOCWaypoint *target, std::vector<Ogre::Vector3> *returnpath);

public:
	Pathfinder(void);
	~Pathfinder(void);

	void RegisterWaypoint(GOCWaypoint *waypoint);
	void UnregisterWaypoint(GOCWaypoint *waypoint);

	GOCWaypoint* GetWPByName(Ogre::String name);
	GOCWaypoint* GetNextWP(Ogre::Vector3 position, std::vector<GOCWaypoint*> excludeList);

	bool FindPath(Ogre::Vector3 position, Ogre::String targetWP, std::vector<Ogre::Vector3> *path, NxActor *actor = 0);
	bool FindPath(Ogre::String startWP, Ogre::String targetWP, std::vector<Ogre::Vector3> *path);
	bool FindPath(GOCWaypoint *start, GOCWaypoint *target, std::vector<Ogre::Vector3> *path);

	//Singleton
	static Pathfinder& Instance();
};

};