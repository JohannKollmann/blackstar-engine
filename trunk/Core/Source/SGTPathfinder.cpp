
#include "SGTPathfinder.h"
#include "SGTGameObject.h"

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

SGTGOCWaypoint* SGTPathfinder::GetWPByName(Ogre::String name)
{
	for (std::list<SGTGOCWaypoint*>::iterator i = mWaynet.begin(); i != mWaynet.end(); i++)
	{
		if ((*i)->GetOwner()->GetName() == name) return (*i);
	}
	Ogre::LogManager::getSingleton().logMessage("Error: Waypoint " + name + " doesn't exist!");
	return 0;
}

SGTGOCWaypoint* SGTPathfinder::GetNextWP(Ogre::Vector3 position)
{
	if (mWaynet.size() == 0) return 0;
	std::list<SGTGOCWaypoint*>::iterator shortest = mWaynet.begin();
	float shortest_distance = (*shortest)->GetPosition().squaredDistance(position);
	for (std::list<SGTGOCWaypoint*>::iterator i = ++mWaynet.begin(); i != mWaynet.end(); i++)
	{
		float distance = (*i)->GetPosition().squaredDistance(position);
		if (distance < shortest_distance)
		{
			shortest_distance = distance;
			shortest = i;
		}
	}
	return (*shortest);
}

void SGTPathfinder::FindPath(Ogre::Vector3 position, Ogre::String targetWP, std::vector<Ogre::Vector3> *path)
{
	SGTGOCWaypoint *start = GetNextWP(position);
	SGTGOCWaypoint *target = GetWPByName(targetWP);
	if (start == 0 || target == 0) return;
	FindPath(start, target, path);
}

void SGTPathfinder::FindPath(Ogre::String startWP, Ogre::String targetWP, std::vector<Ogre::Vector3> *path)
{
	SGTGOCWaypoint *start = GetWPByName(startWP);
	SGTGOCWaypoint *target = GetWPByName(targetWP);
	if (start == 0 || target == 0) return;
	FindPath(start, target, path);
}


//Methoden für A*
bool SGTPathfinder::UpdateEdgeList(WPEdge &e, std::list<WPEdge> *WPEdges)
{
	for (std::list<WPEdge>::iterator i = WPEdges->begin(); i != WPEdges->end(); i++)
	{
		if ((*i).mNeighbor == e.mNeighbor)
		{
			if (e.mCost < (*i).mCost)
			{
				(*i).mWP = e.mWP;
				(*i).mCost = e.mCost;
			}
			return true;
		}
	}
	return false;
}

WPEdge SGTPathfinder::GetBestEdge(std::list<WPEdge> *WPEdges)
{
	if (WPEdges->size() == 0) return WPEdge();
	std::list<WPEdge>::iterator best = WPEdges->begin();
	for (std::list<WPEdge>::iterator i = ++(WPEdges->begin()); i != WPEdges->end(); i++)
	{
		if ((*i).mCost+(*i).mCostOffset < (*best).mCost+(*best).mCostOffset) best = i;
	}
	WPEdge result = (*best);
	WPEdges->erase(best);
	return result;
}

void SGTPathfinder::ExtractPath(std::list<WPEdge> paths, SGTGOCWaypoint *start, SGTGOCWaypoint *target, std::vector<Ogre::Vector3> *returnpath)
{
	SGTGOCWaypoint *current = start;
	do
	{
		bool found = false;
		for (std::list<WPEdge>::iterator i = paths.begin(); i != paths.end(); i++)
		{
			if ((*i).mNeighbor == current)
			{
				returnpath->push_back(current->GetPosition());
				current = (*i).mWP;
				found = true;
				break;
			}
		}
		if (!found)
		{
			Ogre::LogManager::getSingleton().logMessage("Error: Could not find path for " + start->GetOwner()->GetName() + " - " + target->GetOwner()->GetName());
			return;
		}
	} while (current != target);
	returnpath->push_back(target->GetPosition());
}

void SGTPathfinder::FindPath(SGTGOCWaypoint *start, SGTGOCWaypoint *target, std::vector<Ogre::Vector3> *path)
{
	std::list<WPEdge> eventList;
	std::list<WPEdge> shortestPaths;
	//eventList.push_back(target);
	target->GetNeighbors(&eventList, start->GetPosition());		//Wir suchen vom Ziel aus nach dem Startknoten
	while (eventList.size() > 0)
	{
		WPEdge current = GetBestEdge(&eventList);
		std::list<WPEdge> neighbors;
		current.mNeighbor->GetNeighbors(&neighbors, start->GetPosition());
		for (std::list<WPEdge>::iterator i = neighbors.begin(); i != neighbors.end(); i++)
		{
			(*i).mCost += current.mCost;
			if (!(UpdateEdgeList((*i), &eventList) || UpdateEdgeList((*i), &shortestPaths)))
			{
				eventList.push_back((*i));
			}
		}
		shortestPaths.push_back(current);
		if (current.mNeighbor == start) break;
	}

	ExtractPath(shortestPaths, start, target, path);
}


SGTPathfinder& SGTPathfinder::Instance()
{
	static SGTPathfinder TheOneAndOnly;
	return TheOneAndOnly;
};