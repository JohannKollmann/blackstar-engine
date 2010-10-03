
#include "IceAStar.h"
#include "IceGameObject.h"
#include "OgrePhysX.h"
#include "IceMain.h"
#include "IceGOCPhysics.h"
#include "IceNavigationMesh.h"
#include "IceAIManager.h"

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

	//Methoden für A*
	bool AStar::UpdateEdgeList(AStarEdge &e, std::map<AStarNode3D*, AStarEdge*> &edgeNeighborLookup)
	{
		auto find = edgeNeighborLookup.find(e.mNeighbor);
		if (find != edgeNeighborLookup.end())
		{
			if (e.mCost < find->second->mCost)
			{
				find->second->mFrom = e.mFrom;
				find->second->mCost = e.mCost;
			}
			return true;
		}
		return false;
	}

	bool AStar::ExtractPath(std::vector<AStarEdge*> paths, AStarNode3D *start, AStarNode3D *target, std::vector<AStarNode3D*> &returnpath)
	{
		AStarNode3D *current = start;
		while (current != target)
		{
			bool found = false;
			for (auto i = paths.begin(); i != paths.end(); i++)
			{
				if ((*i)->mNeighbor == current)
				{
					returnpath.push_back(current);
					current = (AStarNode3D*)(*i)->mFrom;
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
		int allEdgesMax = AIManager::Instance().GetNavigationMesh()->GetNodeCount()*8;
		std::vector<AStarEdge> allEdges; allEdges.reserve(allEdgesMax);
		std::map<AStarNode3D*, AStarEdge*> eventListNeighborLookup;
		std::vector<AStarEdge*> eventList;
		std::vector<AStarEdge*> shortestPaths;
		target->GetEdgesAStar(allEdges, start->GetGlobalPosition());		//Wir suchen vom Ziel aus nach dem Startknoten
		for (int ni = 0; ni < allEdges.size(); ni++) eventListNeighborLookup.insert(std::make_pair<AStarNode3D*, AStarEdge*>(allEdges[ni].mNeighbor, &allEdges[ni]));
		for (int ni = 0; ni < allEdges.size(); ni++) eventList.push_back(&allEdges[ni]);
		target->closed = true;
		std::make_heap(eventList.begin(), eventList.end(), std::greater<AStarEdge*>());
		while (!eventList.empty())
		{
			std::pop_heap(eventList.begin(), eventList.end(), std::greater<AStarEdge*>());
			AStarEdge *current = eventList.back();
			std::map<AStarNode3D*, AStarEdge*>::iterator iPop = eventListNeighborLookup.find(current->mNeighbor);
			IceAssert(iPop != eventListNeighborLookup.end())
			eventListNeighborLookup.erase(iPop);
			eventList.pop_back();

			int neighborStart = allEdges.size();
			current->mNeighbor->GetEdgesAStar(allEdges, start->GetGlobalPosition());
			IceAssert(allEdges.size() < allEdgesMax)
			for (int i = neighborStart; i < allEdges.size(); i++)
			{
				if (allEdges[i].mNeighbor->closed) continue;
				allEdges[i].mCost += current->mCost;
				if (!(UpdateEdgeList(allEdges[i], eventListNeighborLookup)))// || UpdateEdgeList((*i), &shortestPaths)))
				{
					eventList.push_back(&allEdges[i]);
					IceAssert(eventList.back()->mNeighbor != nullptr)
					//IceAssert(eventListNeighborLookup.find(eventList.back()->mNeighbor) == eventListNeighborLookup.end())
					eventListNeighborLookup.insert(std::make_pair<AStarNode3D*, AStarEdge*>(eventList.back()->mNeighbor, eventList.back()));
					std::push_heap(eventList.begin(), eventList.end(), std::greater<AStarEdge*>());
				}
				else
					std::make_heap(eventList.begin(), eventList.end(), std::greater<AStarEdge*>());
			}
			current->mNeighbor->closed = true;
			shortestPaths.push_back(current);
			if (current->mNeighbor == start)
				break;
		}

		target->closed = false;
		for (auto i = shortestPaths.begin(); i != shortestPaths.end(); i++)
		{
			(*i)->mNeighbor->closed = false;
		}

		return ExtractPath(shortestPaths, start, target, path);
	}

};