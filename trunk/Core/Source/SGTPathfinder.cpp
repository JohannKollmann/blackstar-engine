
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

bool SGTPathfinder::updateEdgeList(WPEdge &e, std::list<WPEdge> *WPEdges)
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

WPEdge SGTPathfinder::getBestEdge(std::list<WPEdge> *WPEdges)
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

void SGTPathfinder::FindPath(Ogre::String startWP, Ogre::String targetWP, std::vector<Ogre::Vector3> *path)
{
}

void SGTPathfinder::FindPath(SGTGOCWaypoint *start, SGTGOCWaypoint *target, std::vector<Ogre::Vector3> *path)
{
	std::list<WPEdge> eventList;
	std::list<WPEdge> shortestPaths;
	target->GetNeighbors(&eventList, start->GetPosition());
	while (eventList.size() > 0)
	{
		WPEdge current = getBestEdge(&eventList);
		std::list<WPEdge> neighbors;
		current.mNeighbor->GetNeighbors(&neighbors, start->GetPosition());
		for (std::list<WPEdge>::iterator i = neighbors.begin(); i != eventList.end(); i++)
		{
			(*i).mCost += current.mCost;
			if (!(updateEdgeList((*i), &eventList) || updateEdgeList((*i), &shortestPaths)))
			{
				eventList.push_back((*i));
			}
		}
		shortestPaths.push_back(current);
		if (current.mNeighbor == start) break;
	}

}

SGTPathfinder& SGTPathfinder::Instance()
{
	static SGTPathfinder TheOneAndOnly;
	return TheOneAndOnly;
};