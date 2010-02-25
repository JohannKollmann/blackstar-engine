
#include "IcePathfinder.h"
#include "IceGameObject.h"
#include "OgrePhysX.h"
#include "IceMain.h"
#include "IceGOCPhysics.h"
#include "IceNavigationMesh.h"

namespace Ice
{

Pathfinder::Pathfinder(void)
{
}

Pathfinder::~Pathfinder(void)
{
}

void Pathfinder::RegisterWaypoint(GOCWaypoint *waypoint)
{
	mWaynet.push_back(waypoint);
}

void Pathfinder::UnregisterWaypoint(GOCWaypoint *waypoint)
{
	mWaynet.remove(waypoint);
}

GOCWaypoint* Pathfinder::GetWPByName(Ogre::String name)
{
	for (std::list<GOCWaypoint*>::iterator i = mWaynet.begin(); i != mWaynet.end(); i++)
	{
		if ((*i)->GetOwner()->GetName() == name) return (*i);
	}
	Ogre::LogManager::getSingleton().logMessage("Error: Waypoint " + name + " doesn't exist!");
	return 0;
}

GOCWaypoint* Pathfinder::GetNextWP(Ogre::Vector3 position, std::vector<GOCWaypoint*> excludeList)
{
	if (mWaynet.size() == 0) return 0;
	std::list<GOCWaypoint*>::iterator shortest = mWaynet.begin();
	float shortest_distance = (*shortest)->GetPosition().squaredDistance(position);
	for (std::list<GOCWaypoint*>::iterator i = ++mWaynet.begin(); i != mWaynet.end(); i++)
	{
		bool exclude = false;
		for (std::vector<GOCWaypoint*>::iterator ex = excludeList.begin(); ex != excludeList.end(); ex++)
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

bool Pathfinder::FindPath(Ogre::Vector3 position, Ogre::String targetWP, std::vector<Ogre::Vector3> *path, NxActor *actor)
{
	GOCWaypoint *target = GetWPByName(targetWP);
	if (target == 0)
	{
		Ogre::LogManager::getSingleton().logMessage("Error: Could not find target Waypoint: '" + targetWP + "'");
		return false;
	}

	//Find the next waypoint which is connected to the target waypoint
	std::vector<GOCWaypoint*> excludeList;
	int maxSearches = 10;
	for (int i = 0; i < maxSearches; i++)
	{
		GOCWaypoint *start = GetNextWP(position, excludeList);
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
				if (hit.hitShape->getGroup() == CollisionGroups::DEFAULT)
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

bool Pathfinder::FindPath(Ogre::String startWP, Ogre::String targetWP, std::vector<Ogre::Vector3> *path)
{
	GOCWaypoint *start = GetWPByName(startWP);
	GOCWaypoint *target = GetWPByName(targetWP);
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
bool Pathfinder::UpdateEdgeList(WPEdge &e, std::vector<WPEdge> *WPEdges)
{
	for (std::vector<WPEdge>::iterator i = WPEdges->begin(); i != WPEdges->end(); i++)
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

bool Pathfinder::ExtractPath(std::vector<WPEdge> paths, GOCWaypoint *start, GOCWaypoint *target, std::vector<Ogre::Vector3> *returnpath)
{
	GOCWaypoint *current = start;
	while (current != target)
	{
		bool found = false;
		for (std::vector<WPEdge>::iterator i = paths.begin(); i != paths.end(); i++)
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

bool Pathfinder::FindPath(GOCWaypoint *start, GOCWaypoint *target, std::vector<Ogre::Vector3> *path)
{
	std::vector<WPEdge> eventList;
	std::vector<WPEdge> shortestPaths;
	target->GetNeighbors(&eventList, start->GetPosition());		//Wir suchen vom Ziel aus nach dem Startknoten
	std::make_heap(eventList.begin(), eventList.end(), std::greater<WPEdge>());
	while (!eventList.empty())
	{
		std::pop_heap(eventList.begin(), eventList.end(), std::greater<WPEdge>());
		WPEdge current = eventList.back();
		eventList.pop_back();
		std::vector<WPEdge> neighbors;
		current.mNeighbor->GetNeighbors(&neighbors, start->GetPosition());
		for (std::vector<WPEdge>::iterator i = neighbors.begin(); i != neighbors.end(); i++)
		{
			(*i).mCost += current.mCost;
			if (!(UpdateEdgeList((*i), &eventList)))// || UpdateEdgeList((*i), &shortestPaths)))
			{
				eventList.push_back((*i));
				std::push_heap(eventList.begin(), eventList.end(), std::greater<WPEdge>());
			}
			else
				std::make_heap(eventList.begin(), eventList.end(), std::greater<WPEdge>());
		}
		shortestPaths.push_back(current);
		if (current.mNeighbor == start) break;
	}

	return ExtractPath(shortestPaths, start, target, path);
}


Pathfinder& Pathfinder::Instance()
{
	static Pathfinder TheOneAndOnly;
	return TheOneAndOnly;
};

};