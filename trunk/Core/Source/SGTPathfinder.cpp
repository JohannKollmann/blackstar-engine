
#include "SGTPathfinder.h"
#include "SGTGameObject.h"
#include "OgrePhysX.h"
#include "SGTMain.h"

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

SGTGOCWaypoint* SGTPathfinder::GetNextWP(Ogre::Vector3 position, std::vector<SGTGOCWaypoint*> excludeList)
{
	if (mWaynet.size() == 0) return 0;
	std::list<SGTGOCWaypoint*>::iterator shortest = mWaynet.begin();
	float shortest_distance = (*shortest)->GetPosition().squaredDistance(position);
	for (std::list<SGTGOCWaypoint*>::iterator i = ++mWaynet.begin(); i != mWaynet.end(); i++)
	{
		bool exclude = false;
		for (std::vector<SGTGOCWaypoint*>::iterator ex = excludeList.begin(); ex != excludeList.end(); ex++)
		{
			if ((*i) == (*ex))
			{
				exclude = true;
				break;
			}
		}
		if (exclude) continue;
		float distance = (*i)->GetPosition().squaredDistance(position);
		if (distance < shortest_distance)
		{
			shortest_distance = distance;
			shortest = i;
		}
	}
	return (*shortest);
}

bool SGTPathfinder::FindPath(Ogre::Vector3 position, Ogre::String targetWP, std::vector<Ogre::Vector3> *path, NxActor *actor)
{
	SGTGOCWaypoint *target = GetWPByName(targetWP);
	if (target == 0)
	{
		Ogre::LogManager::getSingleton().logMessage("Error: Could not find target Waypoint: '" + targetWP + "'");
		return false;
	}

	//Find the next waypoint which is connected to the target waypoint
	std::vector<SGTGOCWaypoint*> excludeList;
	int maxSearches = 10;
	for (int i = 0; i < maxSearches; i++)
	{
		SGTGOCWaypoint *start = GetNextWP(position, excludeList);
		if (!start) return false;
		excludeList.push_back(start);

		if (actor)
		{
			//Test, whether we can reach the waypoint
			Ogre::Vector3 origin = position + Ogre::Vector3(0, 0.5, 0);
			Ogre::Vector3 motion = start->GetPosition() - origin;
			int maxNumResult  = 10;
			NxSweepQueryHit *sqh_result = new NxSweepQueryHit[maxNumResult];
			NxU32 numHits = actor->linearSweep(OgrePhysX::Convert::toNx(motion), NX_SF_STATICS|NX_SF_ALL_HITS, 0, maxNumResult, sqh_result, 0);
			bool obstacleHit = false;
			for (NxU32 i = 0; i < numHits; i++)
			{
				NxSweepQueryHit hit = sqh_result[i];
				Ogre::Vector3 normal = OgrePhysX::Convert::toOgre(hit.normal);
				if (normal.angleBetween(Ogre::Vector3(0,1,0)).valueDegrees() > 60)
				{
					obstacleHit = true;
					break;
				}
			}
			delete sqh_result;
			if (obstacleHit) continue;
		}

		if (FindPath(start, target, path)) return true;
	}
	
	return false;
}

bool SGTPathfinder::FindPath(Ogre::String startWP, Ogre::String targetWP, std::vector<Ogre::Vector3> *path)
{
	SGTGOCWaypoint *start = GetWPByName(startWP);
	SGTGOCWaypoint *target = GetWPByName(targetWP);
	if (start == 0)
	{
		Ogre::LogManager::getSingleton().logMessage("Error: Could not find start Waypoint: '" + startWP + "'");
		return false;
	}
	if (target == 0)
	{
		Ogre::LogManager::getSingleton().logMessage("Error: Could not find target Waypoint: '" + targetWP + "'");
		return false;
	}
	return FindPath(start, target, path);
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

bool SGTPathfinder::ExtractPath(std::list<WPEdge> paths, SGTGOCWaypoint *start, SGTGOCWaypoint *target, std::vector<Ogre::Vector3> *returnpath)
{
	SGTGOCWaypoint *current = start;
	while (current != target)
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
			return false;
		}
	}
	returnpath->push_back(target->GetPosition());
	return true;
}

bool SGTPathfinder::FindPath(SGTGOCWaypoint *start, SGTGOCWaypoint *target, std::vector<Ogre::Vector3> *path)
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

	return ExtractPath(shortestPaths, start, target, path);
}


SGTPathfinder& SGTPathfinder::Instance()
{
	static SGTPathfinder TheOneAndOnly;
	return TheOneAndOnly;
};