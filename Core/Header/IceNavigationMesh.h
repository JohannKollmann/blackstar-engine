
#pragma once

#include "IceIncludes.h"
#include "Ice3D.h"
#include "IceAStar.h"
#include "NxCooking.h"
#include "OgrePhysX.h"
#include "IceMessageSystem.h"

namespace Ice
{

	class DllExport NavigationMesh : public LoadSave::Saveable, public PhysicsMessageListener
	{
	private:
		static const float NODE_DIST;
		static const float NODE_EXTENT;
		static const float NODE_HEIGHT;
		static const float NODE_BORDER;
		static const float MAX_HEIGHT_DIST_BETWEEN_NODES;
		static const float MAX_STEP_HEIGHT;

	public:

		class PathNodeTree : public Saveable
		{
		protected:
			Ogre::AxisAlignedBox mBox;
			Ogre::AxisAlignedBox mBorderBox;
			PathNodeTree(Ogre::AxisAlignedBox box);
			PathNodeTree() : mNodeCount(0) {}

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

			unsigned int mNodeCount;
		};
		class PathNodeTreeNode : public PathNodeTree
		{
		private:
			std::shared_ptr<PathNodeTree> mChildren[8];
			bool mEmpty;
			PathNodeTreeNode() {}

		public:
			PathNodeTreeNode(Ogre::AxisAlignedBox box);
			~PathNodeTreeNode();
			void AddPathNode(AStarNode3D *node);
			void GetPathNodes(const Ogre::AxisAlignedBox &box, std::vector<AStarNode3D*> &oResult, bool getBlocked = false);

			void InjectObstacle(void *identifier, const Ogre::AxisAlignedBox &box);
			void RemoveObstacle(void *identifier, const Ogre::AxisAlignedBox &box);

			//Load / Save
			std::string& TellName() { static std::string name = "PathNodeTreeNode"; return name; };
			void Save(LoadSave::SaveSystem& mgr);
			void Load(LoadSave::LoadSystem& mgr);
			static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "PathNodeTreeNode"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; }
			static LoadSave::Saveable* NewInstance() { return new PathNodeTreeNode; }
		};
		class PathNodeTreeLeaf : public PathNodeTree
		{
		private:
			std::vector< std::shared_ptr<AStarNode3D> > mPathNodes;
			PathNodeTreeLeaf() {}
		public:
			PathNodeTreeLeaf(Ogre::AxisAlignedBox box);
			~PathNodeTreeLeaf();
			void AddPathNode(AStarNode3D *node);
			void GetPathNodes(const Ogre::AxisAlignedBox &box, std::vector<AStarNode3D*> &oResult, bool getBlocked = false);
			AStarNode3D* GetNearestPathNode(Ogre::Vector3 position);

			void InjectObstacle(void *identifier, const Ogre::AxisAlignedBox &box);
			void RemoveObstacle(void *identifier, const Ogre::AxisAlignedBox &box);

			//Load / Save
			std::string& TellName() { static std::string name = "PathNodeTreeLeaf"; return name; };
			void Save(LoadSave::SaveSystem& mgr);
			void Load(LoadSave::LoadSystem& mgr);
			static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "PathNodeTreeLeaf"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; }
			static LoadSave::Saveable* NewInstance() { return new PathNodeTreeLeaf; }
		};

	private:

		bool mNeedsUpdate;
		bool mDestroyingNavMesh;
		OgrePhysX::Actor *mPhysXActor;
		NxTriangleMeshShape *mPhysXMeshShape;
		PathNodeTree *mPathNodeTree;
		Ogre::Entity *mDebugVisual;
		Ogre::ManualObject *mConnectionLinesDebugVisual;
		Ogre::MeshPtr _createOgreMesh(const Ogre::String &resourceName);
		void _destroyDebugVisual();
		void _cutBadPolys(OgrePhysX::Cooker::MeshInfo &meshInfo);

		struct MinMax
		{
			float min;
			float max;
		};
		MinMax getMinMax(const std::vector<float> &vals);

		void rasterNodes();
		bool borderTest(Ogre::Vector3 targetPoint, float size, float maxHeightDif);
		bool checkAgainstLevelMesh(Ogre::Vector3 targetPoint, float extent, float heightOffset);
		AStarNode3D* borderTestCreate(Ogre::Vector3 targetPoint, float size);
		void rasterNodeRow(std::vector<AStarNode3D*> &result, Ogre::Vector3 rayOrigin, float subTest, float rayDist);
		bool checkNodeConnection(AStarNode3D *n1, AStarNode3D *n2);
		void addMatchingNeighbours(std::vector<AStarNode3D*> base, std::vector<AStarNode3D*> add);
		AStarNode3D* rasterNode(Ogre::Vector3 targetPoint, float subTest);
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

		unsigned int GetNodeCount();

		void VisualiseWaymesh(bool show);
		void VisualiseWalkableAreas(bool show);

		///Destroys the physx navmesh and the navmesh octree.
		void Clear();
		///Calls Clear and resets index and vertex buffer.
		void Reset();
		///Rebuilds the physx mnavesh and the navmesh octree, if necessary.
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
		Adds an ogre mesh to the nav mesh.
		*/
		void ImportOgreMesh(Ogre::MeshPtr mesh);

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