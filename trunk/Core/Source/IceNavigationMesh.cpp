
#include "IceNavigationMesh.h"
#include "OgrePhysX.h"
#include "NXU_Streaming.h"
#include "NXU_Streaming.cpp"
#include "NXU_File.cpp"
#include "IceGameObject.h"
#include "IceAIManager.h"

namespace Ice
{

	NavigationMesh::NavigationMesh()
	{
		mPhysXNeedsUpdate = true;
		mPhysXMesh = 0;
	}
	NavigationMesh::~NavigationMesh()
	{
		Clear();
	}

	void NavigationMesh::Clear()
	{
		for (std::vector<Ice::Point3D*>::iterator i = mVertexBuffer.begin(); i != mVertexBuffer.end(); i++)
		{
			delete (*i);
		}
		mVertexBuffer.clear();
		mIndexBuffer.clear();
		mPhysXNeedsUpdate = true;
	}

	void NavigationMesh::AddTriangle(Ice::Point3D* vertex1, Ice::Point3D* vertex2, Ice::Point3D* vertex3)
	{
		int index1 = -1;
		int index2 = -1;
		int index3 = -1;
		int n = 0;
		for (std::vector<Ice::Point3D*>::iterator i = mVertexBuffer.begin(); i != mVertexBuffer.end(); i++)
		{
			if ((*i) == vertex1) index1 = n;
			if ((*i) == vertex2) index2 = n;
			if ((*i) == vertex3) index3 = n;
			n++;
		}
		if (index1 == -1)
		{
			mVertexBuffer.push_back(vertex1);
			index1 = n++;
		}
		if (index2 == -1)
		{
			mVertexBuffer.push_back(vertex2);
			index2 = n++;
		}
		if (index3 == -1)
		{
			mVertexBuffer.push_back(vertex3);
			index3 = n++;
		}

		mIndexBuffer.push_back(index1);
		mIndexBuffer.push_back(index2);
		mIndexBuffer.push_back(index3);

		mPhysXNeedsUpdate = true;
	}
	void NavigationMesh::RemoveVertex(Ice::Point3D* vertex)
	{
		int index = 0;
		std::vector<Ice::Point3D*>::iterator i = mVertexBuffer.begin();
		for (; i != mVertexBuffer.end(); i++)
		{
			if ((*i) == vertex) break;
			index++;
		}
		mVertexBuffer.erase(i);

		std::vector<int>::iterator x = mIndexBuffer.begin();
		for (int i = 0; i < ((int)mIndexBuffer.size()-2); i+=3)
		{
			if (mIndexBuffer[i] == index || mIndexBuffer[i+1] == index || mIndexBuffer[i+2] == index)
			{
				std::vector<int>::iterator to = x;
				to++; to++; to++;
				mIndexBuffer.erase(x, to);
				int test = mIndexBuffer.size();
				x = mIndexBuffer.begin();
				i = 0;
				continue;
			}
			x++; x++; x++;
		}
		for (int i = 0; i < (int)mIndexBuffer.size(); i++)
		{
			if (mIndexBuffer[i] > index) mIndexBuffer[i] = mIndexBuffer[i]-1;
		}

		mPhysXNeedsUpdate = true;
	}

	void NavigationMesh::bakePhysXMesh()
	{
		NxArray<NxVec3> vertices(mVertexBuffer.size());
		NxArray<NxU32> indices(mIndexBuffer.size());
		int i = 0;
		for (std::vector<Ice::Point3D*>::iterator x = mVertexBuffer.begin(); x != mVertexBuffer.end(); x++)
			vertices[i++] = OgrePhysX::Convert::toNx((*x)->GetGlobalPosition());
		i = 0;
		for (std::vector<int>::iterator x = mIndexBuffer.begin(); x != mIndexBuffer.end(); x++)
			indices[i++] = (*x);

		NxTriangleMeshDesc meshDesc;
		meshDesc.numVertices                = vertices.size();
		meshDesc.numTriangles               = indices.size();
		meshDesc.materialIndexStride		= sizeof(NxMaterialIndex);
		meshDesc.pointStrideBytes           = sizeof(NxVec3);
		meshDesc.triangleStrideBytes        = 3 * sizeof(NxU32);
		meshDesc.points = &vertices[0].x;
		meshDesc.triangles = &indices[0];
		meshDesc.materialIndices = 0;
		meshDesc.flags = 0;

		NXU::MemoryWriteBuffer stream;
		OgrePhysX::World::getSingleton().getCookingInterface()->NxCookTriangleMesh(meshDesc, stream);
		mPhysXMesh = OgrePhysX::World::getSingleton().getSDK()->createTriangleMesh(NXU::MemoryReadBuffer(stream.data));
	}

	NxTriangleMesh* NavigationMesh::GetPhysXMesh()
	{
		if (mPhysXNeedsUpdate)
		{
			bakePhysXMesh();
			mPhysXNeedsUpdate = false;
		}
		return 0;
	}

	void NavigationMesh::Save(LoadSave::SaveSystem& mgr)
	{
		std::vector<Ogre::Vector3> rawVertices;
		for (std::vector<Ice::Point3D*>::iterator x = mVertexBuffer.begin(); x != mVertexBuffer.end(); x++)
			rawVertices.push_back((*x)->GetGlobalPosition());

		mgr.SaveAtom("std::vector<Ogre::Vector3>", &rawVertices, "Vertices");
		mgr.SaveAtom("std::vector<int>", &mIndexBuffer, "Indices");
	}
	void NavigationMesh::Load(LoadSave::LoadSystem& mgr)
	{
		std::vector<Ogre::Vector3> rawVertices;
		mgr.LoadAtom("std::vector<Ogre::Vector3>", &rawVertices);
		if (!AIManager::Instance().GetWayMeshLoadingMode())
		{
			for (std::vector<Ogre::Vector3>::iterator x = rawVertices.begin(); x != rawVertices.end(); x++)
				mVertexBuffer.push_back(new Ice::SimplePoint3D((*x)));
		}
		else
		{
			for (std::vector<Ogre::Vector3>::iterator x = rawVertices.begin(); x != rawVertices.end(); x++)
			{
				Ice::GameObject *go = new Ice::GameObject(-1);
				go->SetGlobalPosition(*x);
				mVertexBuffer.push_back(go);
			}
		}

		mgr.LoadAtom("std::vector<int>", &mIndexBuffer);
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