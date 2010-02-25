
#pragma once

#include "IceIncludes.h"
#include "Ice3D.h"
#include "IcePathfinder.h"

namespace Ice
{

	class TriangleNode : public AStarNode
	{
	public:
		enum TriEdges
		{
			A = 0,
			B = 1,
			C = 2,
		};
		class TriangleEdge
		{
		public:
			TriangleEdge() : neighbor(0) {}
			TriEdges neighborEdge;
			TriangleNode *neighbor;
		};

	private:
		void _notifyNeighbor(TriangleNode* n, TriEdges edgeLocation, TriEdges other_edgeLocation);
		void _notifyNeighborDestruction(TriEdges edgeLocation);

	protected:
		Ogre::SharedPtr<Ice::Point3D> mPoint1;
		Ogre::SharedPtr<Ice::Point3D> mPoint2;
		Ogre::SharedPtr<Ice::Point3D> mPoint3;
		TriangleEdge mEdges[3];
		Ogre::Vector3 mCenter;

	public:
		TriangleNode();
		~TriangleNode();

		Ogre::Vector3 GetCenter();
		void ComputeCenter();

		void ConnectNode(TriangleNode* n, TriEdges edgeLocation, TriEdges other_edgeLocation);
		void DisconnectNode(TriEdges edgeLocation);

		void SetTriangle(Ogre::SharedPtr<Ice::Point3D> point1, Ogre::SharedPtr<Ice::Point3D> point2, Ogre::SharedPtr<Ice::Point3D> point3);

		void GetEdgesAStar(std::vector<AStarEdge> &edges, Ogre::Vector3 targetPos);
	};

}