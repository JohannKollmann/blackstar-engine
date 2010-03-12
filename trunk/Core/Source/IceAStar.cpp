
#include "IceAStar.h"
#include "IceGameObject.h"
#include "OgrePhysX.h"
#include "IceMain.h"
#include "IceGOCPhysics.h"
#include "IceNavigationMesh.h"

namespace Ice
{

	void AStarNode3D::GetEdgesAStar(std::vector<AStarEdge> &edges, Ogre::Vector3 targetPos)
	{
		for (std::vector<AStarNode3D*>::iterator i = mNeighbours.begin(); i != mNeighbours.end(); i++)
		{
			if ((*i)->IsBlocked()) continue;
			AStarEdge edge;
			edge.mCost = mPosition.distance((*i)->mPosition);
			edge.mFrom = this;
			edge.mNeighbor = (*i);
			edge.mCostOffset = (*i)->mPosition.distance(targetPos);
			edges.push_back(edge);
		}
	}
	void AStarNode3D::AddNeighbour(AStarNode3D *neighbour, bool undirected)
	{
		mNeighbours.push_back(neighbour);
		if (undirected) neighbour->mNeighbours.push_back(this);
	}

	void AStarNode3D::AddBlocker(void *blocker)
	{
		for (std::vector<void*>::iterator i = mBlockers.begin(); i != mBlockers.end(); i++)
		{
			if ((*i) == blocker) return;
		}
		mBlockers.push_back(blocker);
	}
	void AStarNode3D::RemoveBlocker(void *blocker)
	{
		std::vector<void*>::iterator i = mBlockers.begin();
		bool found = false;
		for (; i != mBlockers.end(); i++)
		{
			if ((*i) == blocker)
			{
				found = true;
				break;
			}
		}
		if (found) mBlockers.erase(i);
	}

	/*GOCWaypoint* Pathfinder::GetNextWP(Ogre::Vector3 position, std::vector<GOCWaypoint*> excludeList)
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
	}*/


	//Methoden für A*
	bool AStar::UpdateEdgeList(AStarEdge &e, std::vector<AStarEdge> &edges)
	{
		for (std::vector<AStarEdge>::iterator i = edges.begin(); i != edges.end(); i++)
		{
			if (i->mNeighbor == e.mNeighbor)
			{
				if (e.mCost < i->mCost)
				{
					i->mFrom = e.mFrom;
					i->mCost = e.mCost;
				}
				return true;
			}
		}
		return false;
	}

	bool AStar::ExtractPath(std::vector<AStarEdge> paths, AStarNode3D *start, AStarNode3D *target, std::vector<AStarNode3D*> &returnpath)
	{
		AStarNode3D *current = start;
		while (current != target)
		{
			bool found = false;
			for (std::vector<AStarEdge>::iterator i = paths.begin(); i != paths.end(); i++)
			{
				if (i->mNeighbor == current)
				{
					returnpath.push_back(current);
					current = (AStarNode3D*)i->mFrom;
					found = true;
					break;
				}
			}
			if (!found)
			{
				//Ogre::LogManager::getSingleton().logMessage("Error: Could not find path for " + start->GetOwner()->GetName() + " - " + target->GetOwner()->GetName());
				return false;
			}
		}
		returnpath.push_back(target);
		return true;
	}

	bool AStar::FindPath(AStarNode3D *start, AStarNode3D *target, std::vector<AStarNode3D*> &path)
	{
		std::vector<AStarEdge> eventList;
		std::vector<AStarEdge> shortestPaths;
		target->GetEdgesAStar(eventList, start->GetGlobalPosition());		//Wir suchen vom Ziel aus nach dem Startknoten
		target->closed = true;
		std::make_heap(eventList.begin(), eventList.end(), std::greater<AStarEdge>());
		while (!eventList.empty())
		{
			std::pop_heap(eventList.begin(), eventList.end(), std::greater<AStarEdge>());
			AStarEdge current = eventList.back();
			eventList.pop_back();
			std::vector<AStarEdge> neighbors;
			current.mNeighbor->GetEdgesAStar(neighbors, start->GetGlobalPosition());
			for (std::vector<AStarEdge>::iterator i = neighbors.begin(); i != neighbors.end(); i++)
			{
				if (i->mNeighbor->closed) continue;
				(*i).mCost += current.mCost;
				if (!(UpdateEdgeList((*i), eventList)))// || UpdateEdgeList((*i), &shortestPaths)))
				{
					eventList.push_back((*i));
					std::push_heap(eventList.begin(), eventList.end(), std::greater<AStarEdge>());
				}
				else
					std::make_heap(eventList.begin(), eventList.end(), std::greater<AStarEdge>());
			}
			current.mNeighbor->closed = true;
			shortestPaths.push_back(current);
			if (current.mNeighbor == start)
				break;
		}

		target->closed = false;
		for (std::vector<AStarEdge>::iterator i = shortestPaths.begin(); i != shortestPaths.end(); i++)
		{
			i->mNeighbor->closed = false;
		}

		return ExtractPath(shortestPaths, start, target, path);
	}

};