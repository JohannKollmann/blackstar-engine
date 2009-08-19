#include "SGTPathfinder.h"

SGTPathfinder::SGTPathfinder(void)
{
}

SGTPathfinder::~SGTPathfinder(void)
{
}

void SGTPathfinder::RegisterWaypoint(SGTGOCWaypoint *waypoint)
{
	mWaynet.push_back(waypoint);
}

void SGTPathfinder::UnregisterWaypoint(SGTGOCWaypoint *waypoint)
{
	mWaynet.remove(waypoint);
}

void SGTPathfinder::FindPath(Ogre::String targetWP, std::vector<Ogre::Vector3> &path)
{
	std::list<Edge> eventList;
	std::list<Edge> shortestPaths;
}