#ifndef __SGTPathfindingManager_H__
#define __SGTPathfindingManager_H__

#include "SGTWaypoint.h"
#include "SGTPathway.h"

class SGTPathfindingManager
{
private:
	static SGTPathfindingManager *mInstance;
	//std::list<Waypoint*> *mWaypoints;
	SGTWaypoint **mWaypoints;
	int WPcount; 
	int calcDistance(Ogre::Vector3 a, Ogre::Vector3 b);
	int calcHeuristik(Ogre::Vector3 a, Ogre::Vector3 b);
	int calcG(SGTWaypoint *start, SGTWaypoint *actual);
public:
	SGTPathfindingManager(){};
	~SGTPathfindingManager(){};
	static SGTPathfindingManager *getInstance();
	void Init(std::list<SGTWaypoint*> *normalWaypoints);
	SGTPathway* FindPath(Ogre::String namestart, Ogre::String nametarget);
};

#endif
