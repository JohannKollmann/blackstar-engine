
#pragma once

#include "SGTIncludes.h"
#include <list>
#include "SGTGOCWaypoint.h"

class SGTPathfinder
{
	struct Edge
	{
		SGTGOCWaypoint *WP1;
		SGTGOCWaypoint *WP2;
		float cost;
	};
private:
	std::list<SGTGOCWaypoint*> mWaynet;

public:
	SGTPathfinder(void);
	~SGTPathfinder(void);

	void RegisterWaypoint(SGTGOCWaypoint *waypoint);
	void UnregisterWaypoint(SGTGOCWaypoint *waypoint);

	void FindPath(Ogre::String targetWP, std::vector<Ogre::Vector3> &path);
};
