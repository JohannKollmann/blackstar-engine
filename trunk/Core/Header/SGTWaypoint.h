#ifndef __Waypoint_H__
#define __Waypoint_H__


#include <Ogre.h>

/*
Ein virtueller Waypoint reduziert den Waypoint auf das wesentliche: Auf seine Position und seine verbundenen Waypoints.
Die virtuelle Waypointliste wird beim erstellen der Waynettabelle benutzt.
*/
const int NOLIST = 0;
const int OPENLIST = 1;
const int CLOSEDLIST = 2;

class SGTWaypoint
{
public:
	SGTWaypoint() : List (NOLIST) {};
	virtual ~SGTWaypoint() {};
	Ogre::Vector3 Position;
	std::list<SGTWaypoint*> mConnectedWaypointlist;
	Ogre::String Name;
	int List;
	SGTWaypoint *ParentWaypoint;
	int actG;
	int actH;
	int actF;
	void CalcF(){actF = actG + actH;};
};

#endif