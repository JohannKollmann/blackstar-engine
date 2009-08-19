#include "../Header/SGTPathfindingManager.h"
#include <list>

SGTPathfindingManager *SGTPathfindingManager::mInstance = 0;

SGTPathfindingManager *SGTPathfindingManager::getInstance()
{
	if(mInstance == 0)
		mInstance = new SGTPathfindingManager();
	return mInstance;
}

int SGTPathfindingManager::calcDistance(Ogre::Vector3 a, Ogre::Vector3 b)
{
	Ogre::Vector3 temp;
	temp = a - b;
	return temp.length();
}

int SGTPathfindingManager::calcHeuristik(Ogre::Vector3 a, Ogre::Vector3 b)
{
	return sqrt(
		(a.x-b.x)*(a.x-b.x) +
		(a.y-b.y)*(a.y-b.y) +
		(a.z-b.z)*(a.z-b.z));
}

int SGTPathfindingManager::calcG(SGTWaypoint *start, SGTWaypoint *actual)
{
	int G = 0;
	SGTWaypoint* tempWP;
	tempWP = actual;
	bool fineshed = false;
	if(tempWP == start)
		fineshed = true;
	while (!fineshed)
	{
		G += calcDistance(tempWP->Position, tempWP->ParentWaypoint->Position); 
		tempWP = tempWP->ParentWaypoint;
		if(tempWP == start)
			fineshed = true;
	}
	return G;
}

void SGTPathfindingManager::Init(std::list<SGTWaypoint*> *normalWaypoints)
{
	WPcount = normalWaypoints->size();
	mWaypoints = new SGTWaypoint*[WPcount];
	std::list<SGTWaypoint*>::iterator WPIter;
	WPIter = normalWaypoints->begin();
	for(int i = 0; i < WPcount; ++i, ++WPIter)
	{
		mWaypoints[i] = (*WPIter);
	}
}
//optimierte Funktion

SGTPathway* SGTPathfindingManager::FindPath(Ogre::String namestart, Ogre::String nametarget)
{
	Ogre::LogManager::getSingleton().logMessage(Ogre::String("Pathfinding startet"));
	SGTWaypoint *Start = NULL;
	SGTWaypoint *Target = NULL;
	SGTWaypoint *Active = NULL;
	int foundst = 0;
	for(int i = 0;foundst<2, i < WPcount; ++i)
	{
		if(mWaypoints[i]->Name == namestart)
		{
			Start = mWaypoints[i];
			++foundst;
		}
		if(mWaypoints[i]->Name == nametarget)
		{
			Target = mWaypoints[i];
			++foundst;
		}
	}
	if(foundst < 2)
	{
		Ogre::LogManager::getSingleton().logMessage(Ogre::String("Start oder Ziel nicht gefunden"));
		return 0;
	}
	Ogre::LogManager::getSingleton().logMessage(Ogre::String("Start + Ziel gefunden"));
	Start->List = OPENLIST;
	Active = Start;
	std::list<SGTWaypoint*>::iterator ConnectedWayIter;
	//ab hier ne schleife^^
	//
	Ogre::LogManager::getSingleton().logMessage(Ogre::String("durchgehen der WPs startet"));
	bool found = false;
	while(!found)
	{
		//Neue WPs hinzufügen
		for(ConnectedWayIter = Active->mConnectedWaypointlist.begin(); ConnectedWayIter != Active->mConnectedWaypointlist.end(); ++ConnectedWayIter)
		{
			if((*ConnectedWayIter)->List == NOLIST)
			{
				//Hier wird der WP hinzugefügt
				(*ConnectedWayIter)->List = OPENLIST;
				(*ConnectedWayIter)->ParentWaypoint = Active;	
			}

		}
		
		int GOld;
		SGTWaypoint* tempParent = 0;
		int lowestF = 9999999;
		SGTWaypoint* WPlowestF = 0;
		for(int i = 0; i < WPcount; ++i)
		{
			if(mWaypoints[i]->List == OPENLIST)
			{
				GOld = mWaypoints[i]->actG;
				tempParent = mWaypoints[i]->ParentWaypoint;
				mWaypoints[i]->ParentWaypoint = Active;
				if(GOld < calcG(Start, mWaypoints[i]))
				{
					mWaypoints[i]->ParentWaypoint = tempParent;
				}
	
				mWaypoints[i]->actG = calcG(Start, mWaypoints[i]);
				mWaypoints[i]->actH = calcHeuristik(Active->Position, Target->Position);
				mWaypoints[i]->CalcF();
			}
			
			if(mWaypoints[i]->List == OPENLIST && mWaypoints[i]->actF<lowestF) 
			{
				lowestF = mWaypoints[i]->actF;
				WPlowestF = mWaypoints[i]; 
			}
			
		}
		if(WPlowestF != Target)
		{
			Active = WPlowestF;
			Active->List = CLOSEDLIST;
		}
		else
		{
			found = true;
			Ogre::LogManager::getSingleton().logMessage(Ogre::String("am Ziel angelangt"));
		}
	}
	Ogre::LogManager::getSingleton().logMessage(Ogre::String("Durchgehen der Nodes beendet"));
	std::list<Ogre::Vector3> WPKoords;
	SGTWaypoint *tempPathWP;
	Ogre::Vector3 tempPathPos;
	tempPathWP = Target;
	bool finished = false;
	while(!finished)
	{
		tempPathPos = tempPathWP->Position;
		WPKoords.push_front(tempPathPos);
		Ogre::LogManager::getSingleton().logMessage(Ogre::String("hinzugefügt: " + tempPathWP->Name));
		if(tempPathWP != Start)
			tempPathWP = tempPathWP->ParentWaypoint;
		else
			finished = true;
	}
	std::vector<Ogre::Vector3> WPKoordsVec;
	std::list<Ogre::Vector3>::iterator WPIter;
	for(WPIter = WPKoords.begin(); WPIter != WPKoords.end(); WPIter++)
	{
		WPKoordsVec.push_back((*WPIter));
		Ogre::LogManager::getSingleton().logMessage(Ogre::String("hinzugefügt: " + Ogre::StringConverter::toString((*WPIter))));
	}

	SGTPathway* Path;
	Path = new SGTPathway(WPKoordsVec, 2);
	//Path->smoothPath();

	for(int i = 0; i < WPcount; ++i)
	{
		mWaypoints[i]->List = NOLIST;
	}

	return Path;
}

/*

SGTPathway* SGTPathfindingManager::FindPath(Ogre::String namestart, Ogre::String nametarget)
{
	Ogre::LogManager::getSingleton().logMessage(Ogre::String("Pathfinding startet"));
	if(mWaypoints == NULL)
		Ogre::LogManager::getSingleton().logMessage(Ogre::String("WaypointList = 0"));
	std::list<Waypoint*>::iterator WayIter = mWaypoints->begin();
	Waypoint *Start = NULL;
	Waypoint *Target = NULL;
	Waypoint *Active = NULL;
	int foundst = 0;
	Ogre::LogManager::getSingleton().logMessage(Ogre::String("Initialisierung beendet"));
	for(;foundst<2, WayIter != mWaypoints->end() ;++WayIter)
	{
		if((*WayIter)->Name == namestart)
		{
			Start = (*WayIter);
			++foundst;
			Ogre::LogManager::getSingleton().logMessage(Ogre::String("Start gefunden"));
		}
		if((*WayIter)->Name == nametarget)
		{
			Target = (*WayIter);
			++foundst;
			Ogre::LogManager::getSingleton().logMessage(Ogre::String("Ziel gefunden"));
		}
	}
	if(foundst < 2)
	{
		Ogre::LogManager::getSingleton().logMessage(Ogre::String("Start oder Ziel nicht gefunden"));
		return 0;
	}
	Ogre::LogManager::getSingleton().logMessage(Ogre::String("Start + Ziel gefunden"));
	Start->List = OPENLIST;
	Active = Start;
	std::list<Waypoint*>::iterator ConnectedWayIter;
	//ab hier ne schleife^^
	//
	Ogre::LogManager::getSingleton().logMessage(Ogre::String("durchgehen der WPs startet"));
	bool found = false;
	while(!found)
	{
		Ogre::LogManager::getSingleton().logMessage(Ogre::String("\n Neuer Durchgang"));
		
		//Neue WPs hinzufügen
		for(ConnectedWayIter = Active->mConnectedWaypointlist.begin(); ConnectedWayIter != Active->mConnectedWaypointlist.end(); ++ConnectedWayIter)
		{
			if((*ConnectedWayIter)->List == NOLIST)
			{
				//Hier wird der WP hinzugefügt
				(*ConnectedWayIter)->List = OPENLIST;
				(*ConnectedWayIter)->ParentWaypoint = Active;	
				Ogre::LogManager::getSingleton().logMessage(Ogre::String("WP" + (*ConnectedWayIter)->Name + "zur OPENLIST hinzugefügt"));
			}

		}
		Ogre::LogManager::getSingleton().logMessage(Ogre::String("neue WPs hinzugefügt"));
		
		int GOld;
		Waypoint* tempParent = 0;
		int lowestF = 9999999;
		Waypoint* WPlowestF = 0;
		for(WayIter = mWaypoints->begin();WayIter != mWaypoints->end(); ++WayIter)
		{
			if((*WayIter)->List == OPENLIST)
			{
				GOld = (*WayIter)->actG;
				tempParent = (*WayIter)->ParentWaypoint;
				(*WayIter)->ParentWaypoint = Active;
				if(GOld < calcG(Start, (*WayIter)))
				{
					(*WayIter)->ParentWaypoint = tempParent;
				}
				Ogre::LogManager::getSingleton().logMessage(Ogre::String("Parent überprüft + evtl verändert"));

				(*WayIter)->actG = calcG(Start, (*WayIter));
				(*WayIter)->actH = calcHeuristik(Active->Position, Target->Position);
				(*WayIter)->CalcF();
				Ogre::LogManager::getSingleton().logMessage(Ogre::String("F berechnet"));
			}
			
			if((*WayIter)->List == OPENLIST && (*WayIter)->actF<lowestF) 
			{
				lowestF = (*WayIter)->actF;
				WPlowestF = (*WayIter); 
				Ogre::LogManager::getSingleton().logMessage(Ogre::String("niedrigstes F gefunden + zugewiesen"));
			}
			
		}
		if(WPlowestF != Target)
		{
			Active = WPlowestF;
			Active->List = CLOSEDLIST;
			Ogre::LogManager::getSingleton().logMessage(Ogre::String("WP auf closed gesetzt"));
			Ogre::LogManager::getSingleton().logMessage(Ogre::String("neues Active festlegt"));
		}
		else
		{
			found = true;
			Ogre::LogManager::getSingleton().logMessage(Ogre::String("am Ziel angelangt"));
		}
	}
	Ogre::LogManager::getSingleton().logMessage(Ogre::String("Durchgehen der Nodes beendet"));
	std::list<Ogre::Vector3> WPKoords;
	Waypoint *tempPathWP;
	Ogre::Vector3 tempPathPos;
	tempPathWP = Target;
	bool finished = false;
	while(!finished)
	{
		tempPathPos = tempPathWP->Position;
		WPKoords.push_front(tempPathPos);
		Ogre::LogManager::getSingleton().logMessage(Ogre::String("hinzugefügt: " + tempPathWP->Name));
		if(tempPathWP != Start)
			tempPathWP = tempPathWP->ParentWaypoint;
		else
			finished = true;
	}
	std::vector<Ogre::Vector3> WPKoordsVec;
	std::list<Ogre::Vector3>::iterator WPIter;
	for(WPIter = WPKoords.begin(); WPIter != WPKoords.end(); WPIter++)
	{
		WPKoordsVec.push_back((*WPIter));
	}

	SGTPathway* Path;
	Path = new SGTPathway(WPKoordsVec, 2);

	for(WayIter = mWaypoints->begin();WayIter != mWaypoints->end(); ++WayIter)
	{
		(*WayIter)->List = NOLIST;
	}

	return Path;
}

*/