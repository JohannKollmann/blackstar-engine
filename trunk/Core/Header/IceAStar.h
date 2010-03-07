
#pragma once

#include "IceIncludes.h"
#include <list>
#include "IceGOCIntern.h"
#include "NxPhysics.h"
#include "Ice3D.h"

namespace Ice
{
	class DllExport AStarNode
	{
	public:
		virtual ~AStarNode() {}
		AStarNode() { closed = false; }
		virtual void GetEdgesAStar(std::vector<AStarEdge> &edges, Ogre::Vector3 targetPos) = 0;

		bool closed; //for algorithm
	};

	class DllExport AStarNode3D : public AStarNode, public SimplePoint3D
	{
	protected:
		std::vector<AStarNode3D*> mNeighbours;

	public:
		AStarNode3D() {}
		AStarNode3D(Ogre::Vector3 position) : SimplePoint3D(position) {}
		virtual ~AStarNode3D() {}
		void GetEdgesAStar(std::vector<AStarEdge> &edges, Ogre::Vector3 targetPos);
		void AddNeighbour(AStarNode3D *neighbour, bool undirected = true);
	};

	class DllExport AStarEdge
	{
	public:
		AStarNode *mFrom;
		AStarNode *mNeighbor;
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