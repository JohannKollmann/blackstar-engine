
#include "IceNavigationMesh.h"

namespace Ice
{

	void NavigationMesh::AddTriangle(Ogre::SharedPtr<Ice::Point3D*> vertex1, Ogre::SharedPtr<Ice::Point3D*> vertex2, Ogre::SharedPtr<Ice::Point3D*> vertex3)
	{
	}
	void NavigationMesh::RemoveVertex(Ogre::SharedPtr<Ice::Point3D*> vertex)
	{
	}

	NxTriangleMesh* NavigationMesh::GetPhysXMesh()
	{
		return 0;
	}

	void NavigationMesh::Save(LoadSave::SaveSystem& mgr)
	{
	}
	void NavigationMesh::Load(LoadSave::LoadSystem& mgr)
	{
	}

	TriangleNode::TriangleNode()
	{
	}
	TriangleNode::~TriangleNode()
	{
		for (int i = 0; i < 3; i++) DisconnectNode((TriEdges)i);
	}

	void TriangleNode::_notifyNeighbor(TriangleNode* n, TriEdges edgeLocation, TriEdges other_edgeLocation)
	{
		mEdges[edgeLocation].neighbor = n;
		mEdges[edgeLocation].neighborEdge = other_edgeLocation;
	}
	void TriangleNode::_notifyNeighborDestruction(TriEdges edgeLocation)
	{
		mEdges[edgeLocation].neighbor = 0;
	}
	void TriangleNode::ConnectNode(TriangleNode* n, TriEdges edgeLocation, TriEdges other_edgeLocation)
	{
		_notifyNeighbor(n, edgeLocation, other_edgeLocation);
		n->_notifyNeighbor(this, other_edgeLocation, edgeLocation);
	}
	void TriangleNode::DisconnectNode(TriEdges edgeLocation)
	{
		if (mEdges[edgeLocation].neighbor) mEdges[edgeLocation].neighbor->_notifyNeighborDestruction(mEdges[edgeLocation].neighborEdge);
		_notifyNeighborDestruction(edgeLocation);
	}

	void TriangleNode::SetTriangle(Ogre::SharedPtr<Ice::Point3D> point1, Ogre::SharedPtr<Ice::Point3D> point2, Ogre::SharedPtr<Ice::Point3D> point3)
	{
		mPoint1 = point1;
		mPoint2 = point2;
		mPoint3 = point3;
	}

	Ogre::Vector3 TriangleNode::GetCenter()
	{
		return mCenter;
	}
	void TriangleNode::ComputeCenter()
	{
		//not correct, but should work for most cases
		Ogre::Vector3 mid1 = mPoint2->GetGlobalPosition() + (0.5f * (mPoint1->GetGlobalPosition() - mPoint2->GetGlobalPosition()));
		mCenter = mid1 + (0.5f * (mPoint3->GetGlobalPosition() - mid1));
	}

	void TriangleNode::GetEdgesAStar(std::vector<AStarEdge> &edges, Ogre::Vector3 targetPos)
	{
		for (int i = 0; i < 3; i++)
		{
			AStarEdge edge;
			if (mEdges[i].neighbor)
			{
				edge.mCost = mCenter.distance(mEdges[i].neighbor->GetCenter());
				edge.mFrom = this;
				edge.mNeighbor = mEdges[i].neighbor;
				edge.mCostOffset = mEdges[i].neighbor->GetCenter().distance(targetPos);
				edges.push_back(edge);
			}
		}
	}

}