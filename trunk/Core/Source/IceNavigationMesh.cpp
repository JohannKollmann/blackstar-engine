
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

	const float NavigationMesh::NODE_DIST = 1.5f;
	const float NavigationMesh::NODE_EXTENT = 0.8f;
	const float NavigationMesh::NODE_BORDER = 0.4f;

	const float NavigationMesh::PathNodeTree::BOXSIZE_MIN = 10.0f;

	NavigationMesh::PathNodeTree::PathNodeTree(Ogre::AxisAlignedBox box) : mBox(box), mBorderBox(box.getMinimum() + Ogre::Vector3(-NODE_EXTENT,-NODE_EXTENT,-NODE_EXTENT), box.getMaximum() + Ogre::Vector3(NODE_EXTENT,NODE_EXTENT,NODE_EXTENT))
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
			return new PathNodeTreeLeaf(cube);
		return new PathNodeTreeNode(cube);
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
		memset(mChildren, 0, 32);
		mEmpty = true;
	}
	NavigationMesh::PathNodeTreeNode::~PathNodeTreeNode()
	{
		for (int i = 0; i < 8; i++)
		{
			if (mChildren[i]) delete mChildren[i];
		}
	}
	void NavigationMesh::PathNodeTreeNode::AddPathNode(AStarNode3D *node)
	{
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
				if (createLeafs) mChildren[i] = new PathNodeTreeLeaf(subBox);
				else mChildren[i] = new PathNodeTreeNode(subBox);
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

	NavigationMesh::PathNodeTreeLeaf::PathNodeTreeLeaf(Ogre::AxisAlignedBox box) : PathNodeTree(box)
	{
	}
	NavigationMesh::PathNodeTreeLeaf::~PathNodeTreeLeaf()
	{
		for (std::vector<AStarNode3D*>::iterator i = mPathNodes.begin(); i != mPathNodes.end(); i++)
			delete (*i);
	}
	void NavigationMesh::PathNodeTreeLeaf::AddPathNode(AStarNode3D *node)
	{
		mPathNodes.push_back(node);
	}
	void NavigationMesh::PathNodeTreeLeaf::GetPathNodes(const Ogre::AxisAlignedBox &box, std::vector<AStarNode3D*> &oResult, bool getBlocked)
	{
		for (std::vector<AStarNode3D*>::iterator i = mPathNodes.begin(); i != mPathNodes.end(); i++)
		{
			if ((*i)->volume.intersects(box) && (getBlocked || !(*i)->IsBlocked())) oResult.push_back(*i);
		}
	}
	void NavigationMesh::PathNodeTreeLeaf::InjectObstacle(void *identifier, const Ogre::AxisAlignedBox &box)
	{
		for (std::vector<AStarNode3D*>::iterator i = mPathNodes.begin(); i != mPathNodes.end(); i++)
		{
			if ((*i)->volume.intersects(box))
				(*i)->AddBlocker(identifier);
		}
	}
	void NavigationMesh::PathNodeTreeLeaf::RemoveObstacle(void *identifier, const Ogre::AxisAlignedBox &box)
	{
		for (std::vector<AStarNode3D*>::iterator i = mPathNodes.begin(); i != mPathNodes.end(); i++)
		{
			//if ((*i)->volume.intersects(box))
				(*i)->RemoveBlocker(identifier);
		}
	}

	NavigationMesh::NavigationMesh()
	{
		mNeedsUpdate = true;
		mDestroyingNavMesh = false;
		mPhysXActor = 0;
		mPhysXMeshShape = 0;
		mPathNodeTree = 0;

		MessageSystem::Instance().JoinNewsgroup(this, "ACOTR_ONSLEEP");
		MessageSystem::Instance().JoinNewsgroup(this, "ACOTR_ONWAKE");
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
			delete (*i);
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
			mPhysXActor = 0;
		}
		if (mPathNodeTree)
		{
			delete mPathNodeTree;
			mPathNodeTree = 0;
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

		Ogre::Vector3 minOffset(-2, -2, -2);
		Ogre::Vector3 maxOffset(2, 2, 2);
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
		AStarNode3D *fromNode = fromNodes[0];
		AStarNode3D *toNode = toNodes[0];
		for (unsigned int i = 1; i < fromNodes.size(); i++)
		{
			if (fromNodes[i]->GetGlobalPosition().distance(to) < fromNode->GetGlobalPosition().distance(to))
				fromNode = fromNodes[i];
		}
		for (unsigned int i = 1; i < toNodes.size(); i++)
		{
			if (toNodes[i]->GetGlobalPosition().distance(to) < toNode->GetGlobalPosition().distance(to))
				toNode = toNodes[i];
		}
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

	bool NavigationMesh::quadTest(Ogre::Vector3 center, float size, float rayDist)
	{
		OgrePhysX::Scene::QueryHit dummyQuery;
		if (!Ice::Main::Instance().GetPhysXScene()->raycastClosestShape(dummyQuery, Ogre::Ray(center + Ogre::Vector3(size, 0, 0), Ogre::Vector3(0,-1,0)), NX_STATIC_SHAPES, 1<<CollisionGroups::AI, rayDist)) return false;
		if (!Ice::Main::Instance().GetPhysXScene()->raycastClosestShape(dummyQuery, Ogre::Ray(center + Ogre::Vector3(-size, 0, 0), Ogre::Vector3(0,-1,0)), NX_STATIC_SHAPES, 1<<CollisionGroups::AI, rayDist)) return false;
		if (!Ice::Main::Instance().GetPhysXScene()->raycastClosestShape(dummyQuery, Ogre::Ray(center + Ogre::Vector3(0, 0, size), Ogre::Vector3(0,-1,0)), NX_STATIC_SHAPES, 1<<CollisionGroups::AI, rayDist)) return false;
		if (!Ice::Main::Instance().GetPhysXScene()->raycastClosestShape(dummyQuery, Ogre::Ray(center + Ogre::Vector3(0, 0, -size), Ogre::Vector3(0,-1,0)), NX_STATIC_SHAPES, 1<<CollisionGroups::AI, rayDist)) return false;
		return true;
	}

	AStarNode3D* NavigationMesh::quadTestCreate(Ogre::Vector3 center, float size, float rayDist)
	{
		OgrePhysX::Scene::QueryHit query;
		if (!Ice::Main::Instance().GetPhysXScene()->raycastClosestShape(query, Ogre::Ray(center, Ogre::Vector3(0,-1,0)), NX_STATIC_SHAPES, 1<<CollisionGroups::AI, rayDist)) return 0;
		if (!quadTest(center, size, rayDist)) return 0;
		AStarNode3D *node = new AStarNode3D(query.point);
		float height = 1.8f;
		node->volume = Ogre::AxisAlignedBox(node->GetGlobalPosition() + Ogre::Vector3(-NODE_EXTENT, -height, -NODE_EXTENT), 
			node->GetGlobalPosition() + Ogre::Vector3(NODE_EXTENT, height, NODE_EXTENT));
		return node;
	}

	AStarNode3D* NavigationMesh::rasterNode(Ogre::Vector3 rayOrigin, float subTest, float rayDist)
	{
		AStarNode3D *node = 0;
		if (node = quadTestCreate(rayOrigin, NODE_BORDER, rayDist))
			return node;

		if (node = quadTestCreate(rayOrigin + Ogre::Vector3(subTest, 0, subTest), NODE_BORDER, rayDist))
			return node;
		if (node = quadTestCreate(rayOrigin + Ogre::Vector3(subTest, 0, -subTest), NODE_BORDER, rayDist))
			return node;
		if (node = quadTestCreate(rayOrigin + Ogre::Vector3(-subTest, 0, subTest), NODE_BORDER, rayDist))
			return node;
		if (node = quadTestCreate(rayOrigin + Ogre::Vector3(-subTest, 0, -subTest), NODE_BORDER, rayDist))
			return node;
		return 0;
	}

	void NavigationMesh::rasterNodeRow(std::vector<AStarNode3D*> &result, Ogre::Vector3 rayOrigin, float subTest, float rayDist)
	{
		while (AStarNode3D *node = rasterNode(rayOrigin, subTest, rayDist))
		{
			mPathNodeTree->AddPathNode(node);
			/*GameObject *go = new GameObject();
			go->AddComponent(new MeshDebugRenderable("sphere.25cm.mesh"));
			go->SetGlobalPosition(node->GetGlobalPosition());*/
			result.push_back(node);
			rayOrigin.y = node->GetGlobalPosition().y - 1;
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
				if (heightDist < 1)
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
		for (float x = box.getMinimum().x; x < box.getMaximum().x; x += 1.0f)
		{
			for (float z = box.getMinimum().z; z < box.getMaximum().z; z += 1.0f)
			{
				Ogre::AxisAlignedBox subBox(x, box.getMinimum().y, z, x+1.5f, box.getMaximum().y, z+1.0f);
				if (!mPhysXMeshShape->checkOverlapAABB(OgrePhysX::Convert::toNx(subBox)))
					return false;
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
		mid.y += 0.5f;
		bool test = quadTest(mid, NODE_BORDER, 2.0f);
		if (!test)
			return false;
		return test;
	}

	void NavigationMesh::rasterNodes()
	{
		if (!mPhysXMeshShape) return;

		if (mPathNodeTree)
		{
			delete mPathNodeTree;
			mPathNodeTree = 0;
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

				Ogre::Vector3 origin = Ogre::Vector3(x, maxY, 0);
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
	}

	void NavigationMesh::ReceiveMessage(Msg &msg)
	{
		if (!mPathNodeTree) return;
		if (msg.mNewsgroup == "ACOTR_ONSLEEP")
		{
			NxActor *a = (NxActor*)msg.rawData;
			if (!a) return;
			//if (a->getGroup() != Ice::CollisionGroups::DEFAULT) return;
			NxBounds3 nxBounds;
			a->getShapes()[0]->getWorldBounds(nxBounds);
			Ogre::AxisAlignedBox box = OgrePhysX::Convert::toOgre(nxBounds);
			mPathNodeTree->InjectObstacle(a, box);
		}
		if (msg.mNewsgroup == "ACOTR_ONWAKE")
		{
			NxActor *a = (NxActor*)msg.rawData;
			if (!a) return;
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
				mVertexBuffer.push_back(new Ice::SimplePoint3D((*x)));
		}
		else
		{
			for (std::vector<Ogre::Vector3>::iterator x = rawVertices.begin(); x != rawVertices.end(); x++)
			{
				Ice::GameObject *go = new Ice::GameObject();
				go->SetGlobalPosition(*x);
				mVertexBuffer.push_back(go);
			}
		}

		mgr.LoadAtom("std::vector<int>", &mIndexBuffer);

		Update();
	}

}