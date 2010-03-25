
#pragma once

#include "IceIncludes.h"
#include <list>
#include "IceGOCWaypoint.h"
#include "NxPhysics.h"
#include "Ice3D.h"

namespace Ice
{

	class DllExport AStarNode3D : public SimplePoint3D
	{
	protected:
		std::vector<AStarNode3D*> mNeighbours;
		std::vector<void*> mBlockers;

	public:
		AStarNode3D() {}
		AStarNode3D(Ogre::Vector3 position) : SimplePoint3D(position) { closed = false; }
		virtual ~AStarNode3D() {}
		void GetEdgesAStar(std::vector<AStarEdge> &edges, Ogre::Vector3 targetPos);
		void AddNeighbour(AStarNode3D *neighbour, bool undirected = true);

		bool closed; //for algorithm

		Ogre::AxisAlignedBox volume;
		
		bool IsBlocked() { return mBlockers.size() > 0; }
		void AddBlocker(void *blocker);
		void RemoveBlocker(void *blocker);
	};

	class DllExport AStarEdge
	{
	public:
		AStarNode3D *mFrom;
		AStarNode3D *mNeighbor;
		float mCost;
		float mCostOffset;		//for Heuristic
		
		bool operator < (const AStarEdge &rhs) const
		{
			return ((mCost + mCostOffset) < (rhs.mCost + rhs.mCostOffset));
		}
		bool operator > (const AStarEdge &rhs) const
		{
			return ((mCost + mCostOffset) > (rhs.mCost + rhs.mCostOffset));
		}
	};

	class DllExport AStar
	{
	private:

		static bool UpdateEdgeList(AStarEdge &e, std::vector<AStarEdge> &edges);
		static bool ExtractPath(std::vector<AStarEdge> paths, AStarNode3D *start, AStarNode3D *target, std::vector<AStarNode3D*> &returnpath);

	public:

		//bool FindPath(Ogre::Vector3 position, Ogre::String targetWP, std::vector<Ogre::Vector3> *path, NxActor *actor = 0);
		//bool FindPath(Ogre::String startWP, Ogre::String targetWP, std::vector<Ogre::Vector3> *path);
		static bool FindPath(AStarNode3D *start, AStarNode3D *target, std::vector<AStarNode3D*> &path);
	};

}