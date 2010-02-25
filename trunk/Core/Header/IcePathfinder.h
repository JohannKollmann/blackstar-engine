
#pragma once

#include "IceIncludes.h"
#include <list>
#include "IceGOCIntern.h"
#include "NxPhysics.h"

namespace Ice
{
	class AStarNode
	{
	public:
		virtual void GetEdgesAStar(std::vector<AStarEdge> &edges, Ogre::Vector3 targetPos) = 0;
	};

	class AStarEdge
	{
	public:
		AStarNode *mFrom;
		AStarNode *mNeighbor;
		float mCost;
		float mCostOffset;		//for Heuristic
		
		bool operator < (const AStarEdge &rhs) const
		{
			return ((mCost + mCostOffset) < (rhs.mCost + mCostOffset));
		}
		bool operator > (const AStarEdge &rhs) const
		{
			return ((mCost + mCostOffset) > (rhs.mCost + mCostOffset));
		}
	};

	class DllExport Pathfinder
	{
	private:
		std::list<GOCWaypoint*> mWaynet;

		std::vector<TriangleNode*> mNavMesh;

		bool UpdateEdgeList(WPEdge &e, std::vector<WPEdge> *WPEdges);
		bool ExtractPath(std::vector<WPEdge> paths, GOCWaypoint *start, GOCWaypoint *target, std::vector<Ogre::Vector3> *returnpath);

	public:
		Pathfinder(void);
		~Pathfinder(void);

		void RegisterWaypoint(GOCWaypoint *waypoint);
		void UnregisterWaypoint(GOCWaypoint *waypoint);

		GOCWaypoint* GetWPByName(Ogre::String name);
		GOCWaypoint* GetNextWP(Ogre::Vector3 position, std::vector<GOCWaypoint*> excludeList);

		bool FindPath(Ogre::Vector3 position, Ogre::String targetWP, std::vector<Ogre::Vector3> *path, NxActor *actor = 0);
		bool FindPath(Ogre::String startWP, Ogre::String targetWP, std::vector<Ogre::Vector3> *path);
		bool FindPath(GOCWaypoint *start, GOCWaypoint *target, std::vector<Ogre::Vector3> *path);

		//Singleton
		static Pathfinder& Instance();
	};

}