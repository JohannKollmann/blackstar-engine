
#pragma once

#include "IceIncludes.h"
#include "Ice3D.h"
#include "IcePathfinder.h"
#include "NxCooking.h"

namespace Ice
{

	class DllExport NavigationMesh : public LoadSave::Saveable
	{
	private:
		bool mPhysXNeedsUpdate;
		NxTriangleMesh *mPhysXMesh;

		void bakePhysXMesh();

	public:
		NavigationMesh();
		~NavigationMesh();

		void Clear();

		/*
		Adds the vertices to the vertex buffer if needed and adds the triangle to the indexbuffer
		*/
		void AddTriangle(Ice::Point3D* vertex1, Ice::Point3D* vertex2, Ice::Point3D* vertex3);

		/*
		Removes the vertex from the mesh and all Triangles using it
		*/
		void RemoveVertex(Ice::Point3D* vertex);

		std::vector<int> mIndexBuffer;
		std::vector<Ice::Point3D*> mVertexBuffer;

		NxTriangleMesh* GetPhysXMesh();

		//Load / Save
		std::string& TellName()
		{
			static std::string name = "NavigationMesh"; return name;
		};
		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "NavigationMesh"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; }
		static LoadSave::Saveable* NewInstance() { return new NavigationMesh; }

	};
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