
#include "IceNavigationMesh.h"
#include "NXU_Streaming.h"
#include "NXU_Streaming.cpp"
#include "NXU_File.cpp"
#include <NxShapeDesc.h> 
#include <NxTriangleMeshShapeDesc.h> 
#include <NxTriangleMeshShape.h>
#include "IceGameObject.h"
#include "IceAIManager.h"
#include "IceMain.h"
#include "IceGameObject.h"
#include "IceGOCView.h"
#include "IceGOCPhysics.h"

namespace Ice
{

	const float NavigationMesh::NODE_DIST = 1.0f;
	const float NavigationMesh::NODE_EXTENT = 0.5f;
	const float NavigationMesh::NODE_HEIGHT = 1.8f;
	const float NavigationMesh::NODE_BORDER = 0.45f;
	const float NavigationMesh::MAX_HEIGHT_DIST_BETWEEN_NODES = 0.75f;
	const float NavigationMesh::MAX_STEP_HEIGHT = 0.35f;

	const float NavigationMesh::PathNodeTree::BOXSIZE_MIN = 10.0f;

	NavigationMesh::PathNodeTree::PathNodeTree(Ogre::AxisAlignedBox box) : mNodeCount(0), mBox(box), mBorderBox(box.getMinimum() + Ogre::Vector3(-NODE_EXTENT,-NODE_EXTENT,-NODE_EXTENT), box.getMaximum() + Ogre::Vector3(NODE_EXTENT,NODE_EXTENT,NODE_EXTENT))
	{
	}

	NavigationMesh::PathNodeTree* NavigationMesh::PathNodeTree::New(Ogre::AxisAlignedBox box)
	{
		//Convert it to a cube
		Ogre::Vector3 extents = box.getHalfSize();
		float max = extents.x > extents.y ? extents.x : extents.y;
		max = max > extents.z ? max : extents.z;
		Ogre::Vector3 center = box.getCenter();
		Ogre::Vector3 vMax(max, max, max);
		Ogre::Vector3 vMin(-max, -max, -max);
		Ogre::AxisAlignedBox cube(center + vMin, center + vMax);


		if (box.getSize().x < NavigationMesh::PathNodeTree::BOXSIZE_MIN)
			return ICE_NEW PathNodeTreeLeaf(cube);
		return ICE_NEW PathNodeTreeNode(cube);
	}

	bool NavigationMesh::PathNodeTree::HasPoint(const Ogre::Vector3 &point)
	{
		return mBox.intersects(point);
	}
	bool NavigationMesh::PathNodeTree::ContainsBox(const Ogre::AxisAlignedBox &box)
	{
		return mBox.intersects(box);
	}

	NavigationMesh::PathNodeTreeNode::PathNodeTreeNode(Ogre::AxisAlignedBox box) : PathNodeTree(box)
	{
		mEmpty = true;
	}
	NavigationMesh::PathNodeTreeNode::~PathNodeTreeNode()
	{
	}
	void NavigationMesh::PathNodeTreeNode::AddPathNode(AStarNode3D *node)
	{
		mNodeCount++;
		if (mEmpty)
		{
			mEmpty = false;
			const Ogre::Vector3 *corners = mBox.getAllCorners();
			Ogre::Vector3 center = mBox.getCenter();
			bool createLeafs = mBox.getSize().x < PathNodeTree::BOXSIZE_MIN * 2;
			for (int i = 0; i < 8; i++)
			{
				Ogre::Vector3 min(center.x < corners[i].x ? center.x : corners[i].x, center.y < corners[i].y ? center.y : corners[i].y, center.z < corners[i].z ? center.z : corners[i].z);
				Ogre::Vector3 max(center.x >= corners[i].x ? center.x : corners[i].x, center.y >= corners[i].y ? center.y : corners[i].y, center.z >= corners[i].z ? center.z : corners[i].z);
				Ogre::AxisAlignedBox subBox(min, max);
				if (createLeafs) mChildren[i].reset(new PathNodeTreeLeaf(subBox));
				else mChildren[i].reset(new PathNodeTreeNode(subBox));
			}
		}
		for (int i = 0; i < 8; i++)
		{
			if (mChildren[i]->HasPoint(node->GetGlobalPosition()))
			//if (mChildren[i]->ContainsBox(node->volume))
			{
				mChildren[i]->AddPathNode(node);
				return;
			}
		}
		Ogre::LogManager::getSingleton().logMessage("nah");
	}
	void NavigationMesh::PathNodeTreeNode::GetPathNodes(const Ogre::AxisAlignedBox &box, std::vector<AStarNode3D*> &oResult, bool getBlocked)
	{
		if (mEmpty) return;
		for (int i = 0; i < 8; i++)
		{
			if (mChildren[i]->GetBorderBox().intersects(box))
				mChildren[i]->GetPathNodes(box, oResult, getBlocked);
		}
	}
	void NavigationMesh::PathNodeTreeNode::InjectObstacle(void *identifier, const Ogre::AxisAlignedBox &box)
	{
		if (mEmpty) return;
		for (int i = 0; i < 8; i++)
		{
			if (mChildren[i]->GetBorderBox().intersects(box))
				mChildren[i]->InjectObstacle(identifier, box);
		}
	}
	void NavigationMesh::PathNodeTreeNode::RemoveObstacle(void *identifier, const Ogre::AxisAlignedBox &box)
	{
		if (mEmpty) return;
		for (int i = 0; i < 8; i++)
		{
			if (mChildren[i]->GetBorderBox().intersects(box))
				mChildren[i]->RemoveObstacle(identifier, box);
		}
	}
	void NavigationMesh::PathNodeTreeNode::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("Ogre::Vector3", &mBox.getMinimum(), "Box Minimum");
		mgr.SaveAtom("Ogre::Vector3", &mBox.getMaximum(), "Box Maximum");
		for (int i = 0; i < 8; i++) mgr.SaveObject(mChildren[i].get(), "Child", true);
	}
	void NavigationMesh::PathNodeTreeNode::Load(LoadSave::LoadSystem& mgr)
	{
		Ogre::Vector3 boxMin; Ogre::Vector3 boxMax;
		mgr.LoadAtom("Ogre::Vector3", &boxMin);
		mgr.LoadAtom("Ogre::Vector3", &boxMax);
		mBox = Ogre::AxisAlignedBox(boxMin, boxMax);
		mBorderBox = Ogre::AxisAlignedBox(mBox.getMinimum() + Ogre::Vector3(-NODE_EXTENT,-NODE_EXTENT,-NODE_EXTENT), mBox.getMaximum() + Ogre::Vector3(NODE_EXTENT,NODE_EXTENT,NODE_EXTENT));
		for (int i = 0; i < 8; i++) mChildren[i] = mgr.LoadTypedObject<PathNodeTree>();//std::static_pointer_cast<PathNodeTree, LoadSave::Saveable>(mgr.LoadObject());
	}

	NavigationMesh::PathNodeTreeLeaf::PathNodeTreeLeaf(Ogre::AxisAlignedBox box) : PathNodeTree(box)
	{
	}
	NavigationMesh::PathNodeTreeLeaf::~PathNodeTreeLeaf()
	{
	}
	void NavigationMesh::PathNodeTreeLeaf::AddPathNode(AStarNode3D *node)
	{
		mNodeCount++;
		mPathNodes.push_back(std::shared_ptr<AStarNode3D>(node));
	}
	void NavigationMesh::PathNodeTreeLeaf::GetPathNodes(const Ogre::AxisAlignedBox &box, std::vector<AStarNode3D*> &oResult, bool getBlocked)
	{
		for (auto i = mPathNodes.begin(); i != mPathNodes.end(); i++)
		{
			if ((*i)->volume.intersects(box) && (getBlocked || !(*i)->IsBlocked())) oResult.push_back((*i).get());
		}
	}
	void NavigationMesh::PathNodeTreeLeaf::InjectObstacle(void *identifier, const Ogre::AxisAlignedBox &box)
	{
		for (auto i = mPathNodes.begin(); i != mPathNodes.end(); i++)
		{
			if ((*i)->volume.intersects(box))
				(*i)->AddBlocker(identifier);
		}
	}
	void NavigationMesh::PathNodeTreeLeaf::RemoveObstacle(void *identifier, const Ogre::AxisAlignedBox &box)
	{
		for (auto i = mPathNodes.begin(); i != mPathNodes.end(); i++)
		{
			//if ((*i)->volume.intersects(box))
				(*i)->RemoveBlocker(identifier);
		}
	}
	void NavigationMesh::PathNodeTreeLeaf::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("Ogre::Vector3", &mBox.getMinimum(), "Box Minimum");
		mgr.SaveAtom("Ogre::Vector3", &mBox.getMaximum(), "Box Maximum");
		mgr.SaveAtom("vector<PathNodeTreePtr>", (void*)&mPathNodes, "PathNodes");
	}
	void NavigationMesh::PathNodeTreeLeaf::Load(LoadSave::LoadSystem& mgr)
	{
		Ogre::Vector3 boxMin; Ogre::Vector3 boxMax;
		mgr.LoadAtom("Ogre::Vector3", &boxMin);
		mgr.LoadAtom("Ogre::Vector3", &boxMax);
		mBox = Ogre::AxisAlignedBox(boxMin, boxMax);
		mBorderBox = Ogre::AxisAlignedBox(mBox.getMinimum() + Ogre::Vector3(-NODE_EXTENT,-NODE_EXTENT,-NODE_EXTENT), mBox.getMaximum() + Ogre::Vector3(NODE_EXTENT,NODE_EXTENT,NODE_EXTENT));
		mgr.LoadAtom("vector<PathNodeTreePtr>", (void*)&mPathNodes);
	}

	NavigationMesh::NavigationMesh()
	{
		mNeedsUpdate = true;
		mDestroyingNavMesh = false;
		mPhysXActor = nullptr;
		mPhysXMeshShape = nullptr;
		mPathNodeTree = nullptr;
		mDebugVisual = nullptr;
		mConnectionLinesDebugVisual = nullptr;

		JoinNewsgroup(GlobalMessageIDs::ACTOR_ONSLEEP);
		JoinNewsgroup(GlobalMessageIDs::ACTOR_ONWAKE);
	}
	NavigationMesh::~NavigationMesh()
	{
		Reset();
	}

	void NavigationMesh::Reset()
	{
		Clear();
		mDestroyingNavMesh = true;
		for (std::vector<Ice::Point3D*>::iterator i = mVertexBuffer.begin(); i != mVertexBuffer.end(); i++)
		{
			ICE_DELETE (*i);
		}
		mVertexBuffer.clear();
		mIndexBuffer.clear();
		mDestroyingNavMesh = false;
	}

	void NavigationMesh::Clear()
	{
		if (mPhysXActor)
		{
			Main::Instance().GetPhysXScene()->destroyActor(mPhysXActor);
			mPhysXActor = nullptr;
		}
		if (mPathNodeTree)
		{
			ICE_DELETE mPathNodeTree;
			mPathNodeTree = 0;
		}
		if (mConnectionLinesDebugVisual)
		{
			Main::Instance().GetOgreSceneMgr()->destroyManualObject(mConnectionLinesDebugVisual);
			mConnectionLinesDebugVisual = nullptr;
		}
		if (mDebugVisual)
		{
			Main::Instance().GetOgreSceneMgr()->destroyEntity(mDebugVisual);
			mDebugVisual = nullptr;
		}
		mNeedsUpdate = true;
	}

	void NavigationMesh::Update()
	{
		if (mNeedsUpdate)
		{
			Clear();
			bakePhysXMesh();
			rasterNodes();
			mNeedsUpdate = false;
		}
	}

	void NavigationMesh::ShortestPath(Ogre::Vector3 from, Ogre::Vector3 to, std::vector<AStarNode3D*> &oPath)
	{
		Update();
		if (!mPathNodeTree)
		{
			return;
		}

		Ogre::Vector3 minOffset(-1.5f, -1.5f, -1.5f);
		Ogre::Vector3 maxOffset(1.5f, 1.5f, 1.5f);
		Ogre::AxisAlignedBox boxFrom(from + minOffset, from + maxOffset);
		Ogre::AxisAlignedBox boxTo(to + minOffset, to + maxOffset);
		std::vector<AStarNode3D*> fromNodes;
		std::vector<AStarNode3D*> toNodes;
		mPathNodeTree->GetPathNodes(boxFrom, fromNodes);
		mPathNodeTree->GetPathNodes(boxTo, toNodes);
		if (fromNodes.empty() || toNodes.empty())
		{
			Ogre::LogManager::getSingleton().logMessage("Error in NavigationMesh::ShortestPath: No start/end path nodes found!");
			return;
		}
		AStarNode3D *fromNode = nullptr;
		float shortestFromNodeDist = 99999;
		AStarNode3D *toNode = nullptr;
		float shortestToNodeDist = 99999;
		for (unsigned int i = 0; i < fromNodes.size(); i++)
		{
			float dist = fromNodes[i]->GetGlobalPosition().distance(from);
			if (!fromNodes[i]->IsBlocked() && dist < shortestFromNodeDist)
			{
				shortestFromNodeDist = dist;
				fromNode = fromNodes[i];
			}
		}
		if (!fromNode) IceWarning("Origin is blocked")
		for (unsigned int i = 0; i < toNodes.size(); i++)
		{
			float dist = toNodes[i]->GetGlobalPosition().distance(to);
			if (!toNodes[i]->IsBlocked() && dist < shortestToNodeDist)
			{
				shortestToNodeDist = dist;
				toNode = toNodes[i];
			}
		}
		if (!toNode) IceWarning("Target is blocked")
		AStar::FindPath(fromNode, toNode, oPath);
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

		mNeedsUpdate = true;
	}
	void NavigationMesh::RemoveVertex(Ice::Point3D* vertex)
	{
		if (mDestroyingNavMesh) return;

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

		mNeedsUpdate = true;
	}

	void NavigationMesh::bakePhysXMesh()
	{
		if (mIndexBuffer.size() < 3) return;

		NxArray<NxVec3> vertices(mVertexBuffer.size());
		NxArray<NxU32> indices(mIndexBuffer.size() * 2);
		int i = 0;
		for (std::vector<Ice::Point3D*>::iterator x = mVertexBuffer.begin(); x != mVertexBuffer.end(); x++)
			vertices[i++] = OgrePhysX::Convert::toNx((*x)->GetGlobalPosition());
		i = 0;
		for (int x = 0; x < ((int)mIndexBuffer.size())-2; x += 3)
		{
			indices[i++] = mIndexBuffer[x];
			indices[i++] = mIndexBuffer[x+1];
			indices[i++] = mIndexBuffer[x+2];
			indices[i++] = mIndexBuffer[x+2];	//Double sided
			indices[i++] = mIndexBuffer[x+1];
			indices[i++] = mIndexBuffer[x];
		}

		NxTriangleMeshDesc meshDesc;
		meshDesc.numVertices                = vertices.size();
		meshDesc.numTriangles               = indices.size() / 3;
		meshDesc.materialIndexStride		= sizeof(NxMaterialIndex);
		meshDesc.pointStrideBytes           = sizeof(NxVec3);
		meshDesc.triangleStrideBytes        = 3 * sizeof(NxU32);
		meshDesc.points = &vertices[0].x;
		meshDesc.triangles = &indices[0];
		meshDesc.materialIndices = 0;
		meshDesc.flags = 0;

		NXU::MemoryWriteBuffer stream;
		OgrePhysX::World::getSingleton().getCookingInterface()->NxCookTriangleMesh(meshDesc, stream);
		NxTriangleMesh *physXMesh = OgrePhysX::World::getSingleton().getSDK()->createTriangleMesh(NXU::MemoryReadBuffer(stream.data));
		
		if (mPhysXActor) Main::Instance().GetPhysXScene()->destroyActor(mPhysXActor);
		mPhysXActor = Main::Instance().GetPhysXScene()->createActor(OgrePhysX::CookedMeshShape(physXMesh).group(CollisionGroups::AI));
		mPhysXMeshShape = (NxTriangleMeshShape*)mPhysXActor->getNxActor()->getShapes()[0];
	}

	bool NavigationMesh::borderTest(Ogre::Vector3 targetPoint, float size, float maxHeightDif)
	{
		targetPoint.y += maxHeightDif;
		float rayDist = maxHeightDif*2;
		OgrePhysX::Scene::QueryHit dummyQuery;
		if (!Ice::Main::Instance().GetPhysXScene()->raycastClosestShape(dummyQuery, Ogre::Ray(targetPoint + Ogre::Vector3(size, 0, 0), Ogre::Vector3(0,-1,0)), NX_STATIC_SHAPES, 1<<CollisionGroups::AI, rayDist)) return false;
		if (!Ice::Main::Instance().GetPhysXScene()->raycastClosestShape(dummyQuery, Ogre::Ray(targetPoint + Ogre::Vector3(-size, 0, 0), Ogre::Vector3(0,-1,0)), NX_STATIC_SHAPES, 1<<CollisionGroups::AI, rayDist)) return false;
		if (!Ice::Main::Instance().GetPhysXScene()->raycastClosestShape(dummyQuery, Ogre::Ray(targetPoint + Ogre::Vector3(0, 0, size), Ogre::Vector3(0,-1,0)), NX_STATIC_SHAPES, 1<<CollisionGroups::AI, rayDist)) return false;
		if (!Ice::Main::Instance().GetPhysXScene()->raycastClosestShape(dummyQuery, Ogre::Ray(targetPoint + Ogre::Vector3(0, 0, -size), Ogre::Vector3(0,-1,0)), NX_STATIC_SHAPES, 1<<CollisionGroups::AI, rayDist)) return false;
		return true;
	}

	bool NavigationMesh::checkAgainstLevelMesh(Ogre::Vector3 targetPoint, float extent, float heightOffset)
	{
		//check if the node volume intersetcs with the levelmesh
		Ogre::AxisAlignedBox testVolume = Ogre::AxisAlignedBox(targetPoint + Ogre::Vector3(-extent, heightOffset, -extent), 
			targetPoint + Ogre::Vector3(extent, heightOffset+extent, extent));
		if (Main::Instance().GetPhysXScene()->getNxScene()->checkOverlapAABB(OgrePhysX::Convert::toNx(testVolume), NX_STATIC_SHAPES, 1<<CollisionGroups::LEVELMESH))
			return false;
		testVolume = Ogre::AxisAlignedBox(targetPoint + Ogre::Vector3(-extent/2, heightOffset/2, -NODE_EXTENT/2), 
			targetPoint + Ogre::Vector3(extent/2, heightOffset/2 + extent/2, extent/2));
		if (Main::Instance().GetPhysXScene()->getNxScene()->checkOverlapAABB(OgrePhysX::Convert::toNx(testVolume), NX_STATIC_SHAPES, 1<<CollisionGroups::LEVELMESH))
			return false;

		return true;
	}

	AStarNode3D* NavigationMesh::borderTestCreate(Ogre::Vector3 targetPoint, float size)
	{
		if (!borderTest(targetPoint, size, ((size*2)/NODE_DIST) * MAX_HEIGHT_DIST_BETWEEN_NODES)) return nullptr;

		if (!checkAgainstLevelMesh(targetPoint, NODE_EXTENT, MAX_HEIGHT_DIST_BETWEEN_NODES)) return nullptr;

		//create node
		AStarNode3D *node = ICE_NEW AStarNode3D(targetPoint);
		node->volume = Ogre::AxisAlignedBox(targetPoint + Ogre::Vector3(-NODE_EXTENT, 0, -NODE_EXTENT), 
			targetPoint + Ogre::Vector3(NODE_EXTENT, NODE_HEIGHT, NODE_EXTENT));
		return node;
	}

	AStarNode3D* NavigationMesh::rasterNode(Ogre::Vector3 targetPoint, float subTest)
	{
		AStarNode3D *node = 0;
		if (node = borderTestCreate(targetPoint, NODE_BORDER))
			return node;

		if (node = borderTestCreate(targetPoint + Ogre::Vector3(subTest, 0, subTest), NODE_BORDER))
			return node;
		if (node = borderTestCreate(targetPoint + Ogre::Vector3(subTest, 0, -subTest), NODE_BORDER))
			return node;
		if (node = borderTestCreate(targetPoint + Ogre::Vector3(-subTest, 0, subTest), NODE_BORDER))
			return node;
		if (node = borderTestCreate(targetPoint + Ogre::Vector3(-subTest, 0, -subTest), NODE_BORDER))
			return node;
		return nullptr;
	}

	void NavigationMesh::rasterNodeRow(std::vector<AStarNode3D*> &result, Ogre::Vector3 rayOrigin, float subTest, float rayDist)
	{
		OgrePhysX::Scene::QueryHit query;
		while (Ice::Main::Instance().GetPhysXScene()->raycastClosestShape(query, Ogre::Ray(rayOrigin, Ogre::Vector3(0,-1,0)), NX_STATIC_SHAPES, 1<<CollisionGroups::AI, rayDist))
		{
			AStarNode3D* node = rasterNode(query.point, subTest);
			if (node)
			{
				mPathNodeTree->AddPathNode(node);
				/*GameObject *go = ICE_NEW GameObject();
				go->AddComponent(GOComponentPtr(new GOCMeshRenderable("sphere.25cm.mesh", false)));
				go->SetGlobalPosition(node->GetGlobalPosition());*/
				result.push_back(node);
			}
			rayOrigin.y = query.point.y - 1;
		}
	}

	void NavigationMesh::addMatchingNeighbours(std::vector<AStarNode3D*> base, std::vector<AStarNode3D*> add)
	{
		for (std::vector<AStarNode3D*>::iterator i = base.begin(); i != base.end(); i++)
		{
			for (std::vector<AStarNode3D*>::iterator x = add.begin(); x != add.end(); x++)
			{
				float heightDist = (*i)->GetGlobalPosition().y - (*x)->GetGlobalPosition().y;
				if (heightDist < 0) heightDist *= (-1);
				if (heightDist < MAX_HEIGHT_DIST_BETWEEN_NODES)
				{
					if (checkNodeConnection(*i, *x))
						(*i)->AddNeighbour(*x);
				}
			}
		}
	}

	bool NavigationMesh::TestPathVolume(const Ogre::AxisAlignedBox &box)
	{
		std::vector<AStarNode3D*> nodes;
		mPathNodeTree->GetPathNodes(box, nodes, true);
		for (auto i = nodes.begin(); i != nodes.end(); i++)
		{
			if ((*i)->IsBlocked()) return false;
		}

		OgrePhysX::Scene::QueryHit dummyQuery;
		float rayDist = box.getMaximum().y - box.getMinimum().y;
		for (float x = box.getMinimum().x; x < box.getMaximum().x; x += NODE_BORDER)
		{
			for (float z = box.getMinimum().z; z < box.getMaximum().z; z += NODE_BORDER)
			{
				if (!Ice::Main::Instance().GetPhysXScene()->raycastClosestShape(dummyQuery, Ogre::Ray(Ogre::Vector3(x, box.getMaximum().y, z), Ogre::Vector3(0, -1, 0)), NX_STATIC_SHAPES, 1<<CollisionGroups::AI, rayDist))
					return false;
				/*Ogre::AxisAlignedBox subBox(x, box.getMinimum().y, z, x+1.5f, box.getMaximum().y, z+1.0f);
				if (!mPhysXMeshShape->checkOverlapAABB(OgrePhysX::Convert::toNx(subBox)))
					return false;*/
			}
		}
		return true;
	}

	NavigationMesh::MinMax NavigationMesh::getMinMax(const std::vector<float> &vals)
	{
		MinMax r;
		r.min = 99999;
		r.max = -99999;
		for (auto i = vals.begin(); i != vals.end(); i++)
		{
			if (*i < r.min) r.min = *i;
			if (*i >= r.max) r.max = *i;
		}	
		return r;

	}

	bool NavigationMesh::TestLinearPath(Ogre::Vector3 from, Ogre::Vector3 to, float pathBorder)
	{
		Ogre::Vector3 dir = (to - from).normalisedCopy();
		Ogre::Quaternion q = Ogre::Vector3::UNIT_X.getRotationTo(Ogre::Vector3::UNIT_Z);
		Ogre::Vector3 widthVec = q * dir;
		widthVec *= pathBorder;
		Ogre::Vector3 p1 = from + widthVec;
		Ogre::Vector3 p2 = from - widthVec;
		Ogre::Vector3 p3 = to + widthVec;
		Ogre::Vector3 p4 = to -  widthVec;
		std::vector<float> xParams;xParams.push_back(p1.x);xParams.push_back(p2.x);xParams.push_back(p3.x);xParams.push_back(p4.x);
		std::vector<float> yParams;yParams.push_back(p1.y);yParams.push_back(p2.y);yParams.push_back(p3.y);yParams.push_back(p4.y);
		std::vector<float> zParams;zParams.push_back(p1.z);zParams.push_back(p2.z);zParams.push_back(p3.z);zParams.push_back(p4.z);
		MinMax mmX = getMinMax(xParams);
		MinMax mmY = getMinMax(yParams);
		MinMax mmZ = getMinMax(zParams);
		Ogre::Vector3 min(mmX.min, mmY.min-1, mmZ.min);
		Ogre::Vector3 max(mmX.max, mmY.max+1, mmZ.max);
		Ogre::AxisAlignedBox box(min, max);
		return TestPathVolume(box);
	}

	bool NavigationMesh::checkNodeConnection(AStarNode3D *n1, AStarNode3D *n2)
	{
		Ogre::Vector3 mid = ((n1->GetGlobalPosition() - n2->GetGlobalPosition()) * 0.5f) + n2->GetGlobalPosition();

		//check if connection is above waymesh
		if (!borderTest(mid, NODE_BORDER, ((NODE_BORDER*2)/NODE_DIST) * MAX_HEIGHT_DIST_BETWEEN_NODES)) return false;

		//obstacle check
		//stair sampling via raycasting
		OgrePhysX::Scene::QueryHit dummyQuery;
		Ogre::Vector3 rayOrigin = n1->GetGlobalPosition();
		Ogre::Vector3 rayTarget = n2->GetGlobalPosition();
		if (n2->GetGlobalPosition().y < n1->GetGlobalPosition().y)
		{
			rayOrigin = n2->GetGlobalPosition();
			rayTarget = n1->GetGlobalPosition();
		}
		float rayDist = 0.3f;	//stair width
		float rayDist_total = 0;
		Ogre::Vector3 rayDir = rayTarget - rayOrigin;
		float dist = rayDir.normalise(); dist-=0.05f;
		//height test to find out whether stair sampling is neceessary
		if (rayTarget.y - rayOrigin.y > MAX_STEP_HEIGHT)
		{
			int numSteps = Ogre::Math::Ceil(dist/rayDist);
			for (int i = 0; i < numSteps; i++)
			{
				rayDist_total += rayDist;	
				rayOrigin += Ogre::Vector3(0, MAX_STEP_HEIGHT, 0);
				rayDir = (rayTarget - rayOrigin);
				rayDir.y = 0;
				rayDir.normalise();
				if (Ice::Main::Instance().GetPhysXScene()->raycastClosestShape(dummyQuery, Ogre::Ray(rayOrigin, rayDir), NX_STATIC_SHAPES, 1<<CollisionGroups::LEVELMESH, rayDist))
					return false;
				if (!Ice::Main::Instance().GetPhysXScene()->raycastClosestShape(dummyQuery, Ogre::Ray(rayOrigin+(rayDir*rayDist), Ogre::Vector3::NEGATIVE_UNIT_Y), NX_STATIC_SHAPES, 1<<CollisionGroups::LEVELMESH, 2))
					return false;	
				rayOrigin = dummyQuery.point;
			}
		}

		//IceNote(Ogre::StringConverter::toString(rayDist))

		if (!checkAgainstLevelMesh(mid, NODE_BORDER, ((NODE_BORDER*2)/NODE_DIST) * MAX_HEIGHT_DIST_BETWEEN_NODES)) return false;
		mConnectionLinesDebugVisual->position(n1->GetGlobalPosition());
		mConnectionLinesDebugVisual->position(n2->GetGlobalPosition());
		return true;
	}

	void NavigationMesh::rasterNodes()
	{
		if (!mPhysXMeshShape) return;

		if (!mConnectionLinesDebugVisual) mConnectionLinesDebugVisual = Main::Instance().GetOgreSceneMgr()->createManualObject();
		mConnectionLinesDebugVisual->begin("BlueLine", Ogre::RenderOperation::OT_LINE_LIST);

		if (mPathNodeTree)
		{
			ICE_DELETE mPathNodeTree;
			mPathNodeTree = nullptr;
		}

		NxBounds3 aabb;
		mPhysXMeshShape->getWorldBounds(aabb);
		float maxX = (aabb.max.x > aabb.min.x) ? aabb.max.x : aabb.min.x;
		float maxY = (aabb.max.y > aabb.min.y) ? aabb.max.y : aabb.min.y;
		float maxZ = (aabb.max.z > aabb.min.z) ? aabb.max.z : aabb.min.z;
		float minX = (aabb.max.x <= aabb.min.x) ? aabb.max.x : aabb.min.x;
		float minY = (aabb.max.y <= aabb.min.y) ? aabb.max.y : aabb.min.y;
		float minZ = (aabb.max.z <= aabb.min.z) ? aabb.max.z : aabb.min.z;
		float castDist = (maxY - minY) + 0.2f;

		Ogre::AxisAlignedBox oBox = OgrePhysX::Convert::toOgre(aabb);
		mPathNodeTree = PathNodeTree::New(oBox);

		int xNumSamples = (int)Ogre::Math::Ceil(((maxZ - minZ) / NODE_DIST));
		int rowIndex = 0;

		std::vector< std::vector<AStarNode3D*> > lastRow;
		std::vector< std::vector<AStarNode3D*> > currRow;
		for (int i = 0; i < xNumSamples; i++) lastRow.push_back(std::vector<AStarNode3D*>());
		for (int i = 0; i < xNumSamples; i++) currRow.push_back(std::vector<AStarNode3D*>());

		float subTest = NODE_DIST / 4;

			for (float x = minX; x < maxX; x+=NODE_DIST)
			{
				for (int i = 0; i < xNumSamples; i++)
				{
					lastRow[i].clear();
					lastRow[i].assign(currRow[i].begin(), currRow[i].end());
				}
				for (int i = 0; i < xNumSamples; i++) currRow[i].clear();
				rowIndex = 0;

				Ogre::Vector3 origin = Ogre::Vector3(x, maxY+0.1f, 0);
				for (float z = minZ; z < maxZ; z+=NODE_DIST)
				{
					origin.z = z;
					rasterNodeRow(currRow[rowIndex], origin, subTest, castDist);
						//Add neighbours
						if (rowIndex > 0)
						{
							addMatchingNeighbours(currRow[rowIndex], currRow[rowIndex-1]);
							addMatchingNeighbours(currRow[rowIndex], lastRow[rowIndex-1]);
						}
						addMatchingNeighbours(currRow[rowIndex], lastRow[rowIndex]);
						if (rowIndex < xNumSamples-1)
						{
							addMatchingNeighbours(currRow[rowIndex], lastRow[rowIndex+1]);
						}
					rowIndex++;
				}
			}

			mConnectionLinesDebugVisual->end();
	}

	unsigned int NavigationMesh::GetNodeCount()
	{
		if (mPathNodeTree) return mPathNodeTree->mNodeCount; else return 0;
	}
	void NavigationMesh::ReceiveMessage(Msg &msg)
	{
		if (!mPathNodeTree) return;
		if (msg.typeID == GlobalMessageIDs::ACTOR_ONSLEEP)
		{
			NxActor *a = (NxActor*)msg.rawData;
			if (!a || a->getNbShapes() == 0) return;
			//if (a->getGroup() != Ice::CollisionGroups::DEFAULT) return;
			NxBounds3 nxBounds;
			a->getShapes()[0]->getWorldBounds(nxBounds);
			Ogre::AxisAlignedBox box = OgrePhysX::Convert::toOgre(nxBounds);
			mPathNodeTree->InjectObstacle(a, box);
		}
		if (msg.typeID == GlobalMessageIDs::ACTOR_ONWAKE)
		{
			NxActor *a = (NxActor*)msg.rawData;
			if (!a || a->getNbShapes() == 0) return;
			//if (a->getGroup() != Ice::CollisionGroups::DEFAULT) return;
			NxBounds3 nxBounds;
			a->getShapes()[0]->getWorldBounds(nxBounds);
			Ogre::AxisAlignedBox box = OgrePhysX::Convert::toOgre(nxBounds);
			mPathNodeTree->RemoveObstacle(a, box);
		}
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
		Reset();

		std::vector<Ogre::Vector3> rawVertices;
		mgr.LoadAtom("std::vector<Ogre::Vector3>", &rawVertices);
		if (!AIManager::Instance().GetWayMeshLoadingMode())
		{
			for (std::vector<Ogre::Vector3>::iterator x = rawVertices.begin(); x != rawVertices.end(); x++)
				mVertexBuffer.push_back(ICE_NEW Ice::SimplePoint3D((*x)));
		}
		else
		{
			for (std::vector<Ogre::Vector3>::iterator x = rawVertices.begin(); x != rawVertices.end(); x++)
			{
				Ice::GameObject *go = ICE_NEW Ice::GameObject();
				go->SetGlobalPosition(*x);
				mVertexBuffer.push_back(go);
			}
		}

		mgr.LoadAtom("std::vector<int>", &mIndexBuffer);

		Update();
	}

	void NavigationMesh::ImportOgreMesh(Ogre::MeshPtr mesh)
	{
		Reset();
		OgrePhysX::CookerParams cp;
		cp.backfaces(false);
		OgrePhysX::Cooker::MeshInfo meshInfo;
		OgrePhysX::Cooker::getSingleton().getMeshInfo(mesh, cp, meshInfo);
		OgrePhysX::Cooker::getSingleton().mergeVertices(meshInfo);
		_cutBadPolys(meshInfo);
		OgrePhysX::Cooker::getSingleton().mergeVertices(meshInfo);
		for (unsigned int i = 0; i < meshInfo.numVertices; i++)
		{
			Point3D *v = new SimplePoint3D();/*SceneManager::Instance().CreateGameObject();*/v->SetGlobalPosition(OgrePhysX::Convert::toOgre(meshInfo.vertices[i]));
			mVertexBuffer.push_back(v);
		}
		for (unsigned int i = 0; i < meshInfo.numTriangles*3; i+=3)
		{
			mIndexBuffer.push_back(meshInfo.indices[i]);
			mIndexBuffer.push_back(meshInfo.indices[i+1]);
			mIndexBuffer.push_back(meshInfo.indices[i+2]);
		}
		mNeedsUpdate = true;
		Update();
	}

	void NavigationMesh::_cutBadPolys(OgrePhysX::Cooker::MeshInfo &meshInfo)
	{
		NxArray<bool> neededVertices;
		neededVertices.resize(meshInfo.numVertices, false);
		NxArray<NxVec3> newVertices(meshInfo.numVertices);
		int newVertexCount = 0;
		NxArray<NxU32> newIndices(meshInfo.numTriangles*3);
		int newTriCount = 0;
		for (unsigned int i = 0; i < meshInfo.numTriangles*3; i+=3)
		{
			NxVec3 normal = (meshInfo.vertices[meshInfo.indices[i+1]]-meshInfo.vertices[meshInfo.indices[i]]).cross(meshInfo.vertices[meshInfo.indices[i+2]]-meshInfo.vertices[meshInfo.indices[i]]);
			normal.normalize();
			if (normal.dot(NxVec3(0,1,0)) > 0.75f)
			{
				int indexOffset = newTriCount*3;
				newIndices[indexOffset] = meshInfo.indices[i];		neededVertices[meshInfo.indices[i]] = true;
				newIndices[indexOffset+1] = meshInfo.indices[i+1];	neededVertices[meshInfo.indices[i+1]] = true;
				newIndices[indexOffset+2] = meshInfo.indices[i+2];	neededVertices[meshInfo.indices[i+2]] = true;
				newTriCount++;
			}
		}
		/*for (unsigned int i = 0; i < meshInfo.numVertices; i++)
		{
			if (neededVertices[i])
			{
				newVertices[newVertexCount] = meshInfo.vertices[i];
				newVertexCount++;
			}
			else		//decrement indices above the deletes vertex
			{
				for (unsigned int x = 0; x < newTriCount*3; x++)
					if (newIndices[x] >= i) newIndices[x]--;
			}
		}
		meshInfo.numVertices = newVertexCount;
		meshInfo.vertices = newVertices;*/
		meshInfo.numTriangles = newTriCount;
		meshInfo.indices = newIndices;
	}

	Ogre::MeshPtr NavigationMesh::_createOgreMesh(const Ogre::String &resourceName)
	{
		//create wire material if necessary
		if (!Ogre::MaterialManager::getSingleton().resourceExists("DebugBlueWireframe"))
		{
			Ogre::MaterialPtr wireMat = Ogre::MaterialManager::getSingleton().create("DebugBlueWireframe", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
			wireMat->setAmbient(Ogre::ColourValue(0,0,1));
			wireMat->setDiffuse(Ogre::ColourValue(0,0,1));
			//wireMat->getTechnique(0)->getPass(0)->setPolygonMode(Ogre::PM_WIREFRAME);
		}

		Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createManual(resourceName, "General");
		mesh.get()->createSubMesh("main");

		Ogre::VertexData* data = new Ogre::VertexData();
		mesh.get()->sharedVertexData = data;
		data->vertexCount = mVertexBuffer.size();
		Ogre::VertexDeclaration* decl = data->vertexDeclaration;
		decl->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
		Ogre::HardwareVertexBufferSharedPtr vBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
		decl->getVertexSize(0), data->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);

		float* afVertexData=(float*)vBuf->lock(Ogre::HardwareBuffer::HBL_DISCARD);
		int index = 0;
		for (unsigned int i = 0; i < mVertexBuffer.size(); i++)
		{
			afVertexData[index++] = mVertexBuffer[i]->GetGlobalPosition().x;
			afVertexData[index++] = mVertexBuffer[i]->GetGlobalPosition().y;
			afVertexData[index++] = mVertexBuffer[i]->GetGlobalPosition().z;
		}
		vBuf->unlock();

		Ogre::VertexBufferBinding* bind = data->vertexBufferBinding;
		bind->setBinding(0, vBuf);

		Ogre::HardwareIndexBufferSharedPtr iBuf = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
			Ogre::HardwareIndexBuffer::IT_32BIT, mIndexBuffer.size(), Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);

		unsigned int* aiIndexBuf=(unsigned int*)iBuf->lock(Ogre::HardwareBuffer::HBL_DISCARD);
		for (unsigned int i = 0; i < mIndexBuffer.size(); i++)
		{
			aiIndexBuf[i] = mIndexBuffer[i];
		}
		iBuf->unlock();

		mesh->getSubMesh(0)->indexData->indexStart = 0;
		mesh->getSubMesh(0)->indexData->indexCount = mIndexBuffer.size();
		mesh->getSubMesh(0)->indexData->indexBuffer = iBuf;
		mesh->getSubMesh(0)->setMaterialName("DebugBlueWireframe");

		Ogre::AxisAlignedBox bounds(Ogre::Vector3(-1000,-1000,-1000), Ogre::Vector3(1000,1000,1000));
		mesh->_setBounds(bounds);

		return mesh;
	}

	void NavigationMesh::_destroyDebugVisual()
	{
		if (mDebugVisual)
		{
			Main::Instance().GetOgreSceneMgr()->destroyEntity(mDebugVisual);
			mDebugVisual = nullptr;
		}
		if (Ogre::MeshManager::getSingleton().resourceExists("NavMeshVisual"))
			Ogre::MeshManager::getSingleton().remove("NavMeshVisual");
	}

	void NavigationMesh::VisualiseWalkableAreas(bool show)
	{
		_destroyDebugVisual();
		if (Main::Instance().GetOgreSceneMgr()->hasSceneNode("Navmesh_Walkables")) Main::Instance().GetOgreSceneMgr()->destroySceneNode("Navmesh_Walkables");
		if (show)
		{
			_createOgreMesh("NavMeshVisual");
			mDebugVisual = Main::Instance().GetOgreSceneMgr()->createEntity("NavMeshVisual");
			mDebugVisual->setCastShadows(false);
			Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode("Navmesh_Walkables", Ogre::Vector3(0,0.05f, 0))->attachObject(mDebugVisual);
		}
	}
	void NavigationMesh::VisualiseWaymesh(bool show)
	{
		if (Main::Instance().GetOgreSceneMgr()->hasSceneNode("Navmesh_Waymesh")) Main::Instance().GetOgreSceneMgr()->destroySceneNode("Navmesh_Waymesh");
		if (show && mConnectionLinesDebugVisual)
		{
			Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode("Navmesh_Waymesh", Ogre::Vector3(0,0.1f, 0))->attachObject(mConnectionLinesDebugVisual);
		}
	}

}