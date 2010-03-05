
#pragma once

#include "IceIncludes.h"
#include "Ice3D.h"
#include "IceAStar.h"
#include "NxCooking.h"
#include "OgrePhysX.h"

namespace Ice
{

	class DllExport NavigationMesh : public LoadSave::Saveable
	{
	private:

		class PathNodeTree
		{
		protected:
			Ogre::AxisAlignedBox mBox;
			PathNodeTree(Ogre::AxisAlignedBox box) : mBox(box) {}

			static const float BOXSIZE_MIN;

		public:
			virtual ~PathNodeTree() {}

			static PathNodeTree* New(Ogre::AxisAlignedBox box);

			bool HasPoint(Ogre::Vector3 position);
			bool ContainsBox(Ogre::AxisAlignedBox &box);

			virtual void AddPathNode(AStarNode3D *node) = 0;
			virtual void GetPathNodes(Ogre::AxisAlignedBox box, std::vector<AStarNode3D*> &oResult) = 0;
			virtual AStarNode3D* GetNearestPathNode(Ogre::Vector3 position) = 0;
		};
		class PathNodeTreeNode : public PathNodeTree
		{
		private:
			PathNodeTree *mChildren[8];
			bool mEmpty;

		public:
			PathNodeTreeNode(Ogre::AxisAlignedBox box);
			~PathNodeTreeNode();
			void AddPathNode(AStarNode3D *node);
			void GetPathNodes(Ogre::AxisAlignedBox box, std::vector<AStarNode3D*> &oResult);
			AStarNode3D* GetNearestPathNode(Ogre::Vector3 position);
		};
		class PathNodeTreeLeaf : public PathNodeTree
		{
		private:
			std::vector<AStarNode3D*> mPathNodes;
		public:
			PathNodeTreeLeaf(Ogre::AxisAlignedBox box);
			~PathNodeTreeLeaf();
			void AddPathNode(AStarNode3D *node);
			void GetPathNodes(Ogre::AxisAlignedBox box, std::vector<AStarNode3D*> &oResult);
			AStarNode3D* GetNearestPathNode(Ogre::Vector3 position);
		};

		bool mNeedsUpdate;
		OgrePhysX::Actor *mPhysXActor;
		NxTriangleMeshShape *mPhysXMeshShape;
		PathNodeTree *mPathNodeTree;

		float mMaxWaynodeDist;
		float mMinBorder;

		void rasterNodes();
		void bakePhysXMesh();

		class NxUserIntReport : public NxUserEntityReport<NxU32>
		{
			bool onEvent  (NxU32  nbEntities, NxU32 *entities)
			{
				return true;
			}
		};

	public:

		std::vector<int> mIndexBuffer;
		std::vector<Ice::Point3D*> mVertexBuffer;

		NavigationMesh();
		~NavigationMesh();

		void Clear();

		void ShortestPath(Ogre::Vector3 from, Ogre::Vector3 to, std::vector<AStarNode3D*> &oPath);

		/*
		Adds the vertices to the vertex buffer if needed and adds the triangle to the indexbuffer
		*/
		void AddTriangle(Ice::Point3D* vertex1, Ice::Point3D* vertex2, Ice::Point3D* vertex3);

		/*
		Removes the vertex from the mesh and all Triangles using it
		*/
		void RemoveVertex(Ice::Point3D* vertex);

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
}