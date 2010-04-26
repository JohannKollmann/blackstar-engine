
#pragma once

#include "IceIncludes.h"
#include "Ice3D.h"
#include "IceAStar.h"
#include "NxCooking.h"
#include "OgrePhysX.h"
#include "IceMessageSystem.h"

namespace Ice
{

	class DllExport NavigationMesh : public LoadSave::Saveable, public MessageListener
	{
	private:
		static const float NODE_DIST;
		static const float NODE_EXTENT;
		static const float NODE_BORDER;

	private:

		class PathNodeTree
		{
		protected:
			Ogre::AxisAlignedBox mBox;
			Ogre::AxisAlignedBox mBorderBox;
			PathNodeTree(Ogre::AxisAlignedBox box);

			static const float BOXSIZE_MIN;
			static const float BOX_BORDER;

		public:
			virtual ~PathNodeTree() {}

			static PathNodeTree* New(Ogre::AxisAlignedBox box);

			bool HasPoint(const Ogre::Vector3 &position);
			bool ContainsBox(const Ogre::AxisAlignedBox &box);
			Ogre::AxisAlignedBox& GetBox() { return mBox; }
			Ogre::AxisAlignedBox& GetBorderBox() { return mBorderBox; }

			virtual void AddPathNode(AStarNode3D *node) = 0;
			virtual void GetPathNodes(const Ogre::AxisAlignedBox &box, std::vector<AStarNode3D*> &oResult, bool getBlocked = false) = 0;

			virtual void InjectObstacle(void *identifier, const Ogre::AxisAlignedBox &box) = 0;
			virtual void RemoveObstacle(void *identifier, const Ogre::AxisAlignedBox &box) = 0;
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
			void GetPathNodes(const Ogre::AxisAlignedBox &box, std::vector<AStarNode3D*> &oResult, bool getBlocked = false);

			void InjectObstacle(void *identifier, const Ogre::AxisAlignedBox &box);
			void RemoveObstacle(void *identifier, const Ogre::AxisAlignedBox &box);
		};
		class PathNodeTreeLeaf : public PathNodeTree
		{
		private:
			std::vector<AStarNode3D*> mPathNodes;
		public:
			PathNodeTreeLeaf(Ogre::AxisAlignedBox box);
			~PathNodeTreeLeaf();
			void AddPathNode(AStarNode3D *node);
			void GetPathNodes(const Ogre::AxisAlignedBox &box, std::vector<AStarNode3D*> &oResult, bool getBlocked = false);
			AStarNode3D* GetNearestPathNode(Ogre::Vector3 position);

			void InjectObstacle(void *identifier, const Ogre::AxisAlignedBox &box);
			void RemoveObstacle(void *identifier, const Ogre::AxisAlignedBox &box);
		};

		bool mNeedsUpdate;
		bool mDestroyingNavMesh;
		OgrePhysX::Actor *mPhysXActor;
		NxTriangleMeshShape *mPhysXMeshShape;
		PathNodeTree *mPathNodeTree;

		struct MinMax
		{
			float min;
			float max;
		};
		MinMax getMinMax(const std::vector<float> &vals);

		void rasterNodes();
		bool quadTest(Ogre::Vector3 center, float size, float rayDist);
		AStarNode3D* quadTestCreate(Ogre::Vector3 center, float size, float rayDist);
		void rasterNodeRow(std::vector<AStarNode3D*> &result, Ogre::Vector3 rayOrigin, float subTest, float rayDist);
		bool checkNodeConnection(AStarNode3D *n1, AStarNode3D *n2);
		void addMatchingNeighbours(std::vector<AStarNode3D*> base, std::vector<AStarNode3D*> add);
		AStarNode3D* rasterNode(Ogre::Vector3 rayOrigin, float subTest, float rayDist);
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
		void Reset();
		void Update();

		/*
		Retrieves the shortest path (if existing) between from and to
		*/
		void ShortestPath(Ogre::Vector3 from, Ogre::Vector3 to, std::vector<AStarNode3D*> &oPath);

		/*
		Tests whether the whole box is within the navmesh.
		*/
		bool TestPathVolume(const Ogre::AxisAlignedBox &box);

		bool TestLinearPath(Ogre::Vector3 from, Ogre::Vector3 to, float pathBorder = NODE_BORDER);
		
		/*
		Adds the vertices to the vertex buffer if needed and adds the triangle to the indexbuffer
		*/
		void AddTriangle(Ice::Point3D* vertex1, Ice::Point3D* vertex2, Ice::Point3D* vertex3);

		/*
		Removes the vertex from the mesh and all Triangles using it
		*/
		void RemoveVertex(Ice::Point3D* vertex);

		void ReceiveMessage(Msg &msg);

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