
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

	const float NavigationMesh::PathNodeTree::BOXSIZE_MIN = 10;

	NavigationMesh::PathNodeTree::PathNodeTree(Ogre::AxisAlignedBox box) : mBox(box)
	{
		mBorderBox = mBox;
		mBorderBox.scale(Ogre::Vector3(1.2f, 1.2f, 1.2f));
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
				break;
			}
		}
	}
	void NavigationMesh::PathNodeTreeNode::GetPathNodes(Ogre::AxisAlignedBox box, std::vector<AStarNode3D*> &oResult)
	{
		if (mEmpty) return;
		for (int i = 0; i < 8; i++)
		{
			if (mChildren[i]->ContainsBox(box))
				mChildren[i]->GetPathNodes(box, oResult);
		}
	}
	void NavigationMesh::PathNodeTreeNode::InjectObstacle(void *identifier, const Ogre::AxisAlignedBox &box)
	{
		if (mEmpty) return;
		for (int i = 0; i < 8; i++)
		{
			//if (mChildren[i]->ContainsBox(box))
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
	void NavigationMesh::PathNodeTreeLeaf::GetPathNodes(Ogre::AxisAlignedBox box, std::vector<AStarNode3D*> &oResult)
	{
		for (std::vector<AStarNode3D*>::iterator i = mPathNodes.begin(); i != mPathNodes.end(); i++)
		{
			if (box.intersects((*i)->GetGlobalPosition()) && !(*i)->IsBlocked()) oResult.push_back(*i);
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
		mPhysXActor = 0;
		mPhysXMeshShape = 0;
		mPathNodeTree = 0;
		mMinBorder = 0.3f;
		mMaxWaynodeDist = 1.5f;

		MessageSystem::Instance().JoinNewsgroup(this, "ACOTR_ONSLEEP");
		MessageSystem::Instance().JoinNewsgroup(this, "ACOTR_ONWAKE");
	}
	NavigationMesh::~NavigationMesh()
	{
		if (mPhysXActor) Main::Instance().GetPhysXScene()->destroyActor(mPhysXActor);
		Clear();

		MessageSystem::Instance().QuitNewsgroup(this, "ACOTR_ONSLEEP");
		MessageSystem::Instance().QuitNewsgroup(this, "ACOTR_ONWAKE");
	}

	void NavigationMesh::Clear()
	{
		if (mPathNodeTree)
		{
			delete mPathNodeTree;
			mPathNodeTree = 0;
		}
		for (std::vector<Ice::Point3D*>::iterator i = mVertexBuffer.begin(); i != mVertexBuffer.end(); i++)
		{
			delete (*i);
		}
		mVertexBuffer.clear();
		mIndexBuffer.clear();
		mNeedsUpdate = true;
	}

	void NavigationMesh::ShortestPath(Ogre::Vector3 from, Ogre::Vector3 to, std::vector<AStarNode3D*> &oPath)
	{
		if (mNeedsUpdate)
		{
			Clear();
			bakePhysXMesh();
			rasterNodes();
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
		for (int i = 1; i < fromNodes.size(); i++)
		{
			if (fromNodes[i]->GetGlobalPosition().distance(from) < fromNode->GetGlobalPosition().distance(from))
				fromNode = fromNodes[i];
		}
		for (int i = 1; i < toNodes.size(); i++)
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
		NxArray<NxVec3> vertices(mVertexBuffer.size());
		NxArray<NxU32> indices(mIndexBuffer.size() * 2);
		int i = 0;
		for (std::vector<Ice::Point3D*>::iterator x = mVertexBuffer.begin(); x != mVertexBuffer.end(); x++)
			vertices[i++] = OgrePhysX::Convert::toNx((*x)->GetGlobalPosition());
		i = 0;
		for (unsigned int x = 0; x < mIndexBuffer.size()-2; x += 3)
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
		float nodeExtent = 0.8f;
		float height = 1.8f;
		node->volume = Ogre::AxisAlignedBox(node->GetGlobalPosition() + Ogre::Vector3(-nodeExtent, -height, -nodeExtent), 
			node->GetGlobalPosition() + Ogre::Vector3(nodeExtent, height, nodeExtent));
		return node;
	}

	AStarNode3D* NavigationMesh::rasterNode(Ogre::Vector3 rayOrigin, float subTest, float rayDist)
	{
		AStarNode3D *node = 0;
		if (node = quadTestCreate(rayOrigin, mMinBorder, rayDist))
			return node;

		if (node = quadTestCreate(rayOrigin + Ogre::Vector3(subTest, 0, subTest), mMinBorder, rayDist))
			return node;
		if (node = quadTestCreate(rayOrigin + Ogre::Vector3(subTest, 0, -subTest), mMinBorder, rayDist))
			return node;
		if (node = quadTestCreate(rayOrigin + Ogre::Vector3(-subTest, 0, subTest), mMinBorder, rayDist))
			return node;
		if (node = quadTestCreate(rayOrigin + Ogre::Vector3(-subTest, 0, -subTest), mMinBorder, rayDist))
			return node;
		return 0;
	}

	void NavigationMesh::rasterNodeRow(std::vector<AStarNode3D*> &result, Ogre::Vector3 rayOrigin, float subTest, float rayDist)
	{
		while (AStarNode3D *node = rasterNode(rayOrigin, subTest, rayDist))
		{
			mPathNodeTree->AddPathNode(node);
			GameObject *go = new GameObject();
			go->AddComponent(new MeshDebugRenderable("sphere.25cm.mesh"));
			go->SetGlobalPosition(node->GetGlobalPosition());
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

	bool NavigationMesh::TestPathVolume(Ogre::AxisAlignedBox box)
	{
		return true;
	}

	bool NavigationMesh::checkNodeConnection(AStarNode3D *n1, AStarNode3D *n2)
	{
		Ogre::Vector3 mid = ((n1->GetGlobalPosition() - n2->GetGlobalPosition()) * 0.5f) + n2->GetGlobalPosition();
		mid.y += 0.5f;
		bool test = quadTest(mid, mMinBorder, 2.0f);
		if (!test)
			return false;
		return test;
	}

	void NavigationMesh::rasterNodes()
	{
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

		int xNumSamples = (int)Ogre::Math::Ceil(((maxZ - minZ) / mMaxWaynodeDist));
		/*int xNumSamplesByte = xNumSamples * 4;
		AStarNode3D **lastRow = new AStarNode3D*[xNumSamples];
		memset(lastRow, 0, xNumSamplesByte);
		AStarNode3D **currRow = new AStarNode3D*[xNumSamples];
		memset(currRow, 0, xNumSamplesByte);
		//for (int i = 0; i < xNumSamples; i++) lastRow[i] = 0;*/
		int rowIndex = 0;

		std::vector< std::vector<AStarNode3D*> > lastRow;
		std::vector< std::vector<AStarNode3D*> > currRow;
		for (int i = 0; i < xNumSamples; i++) lastRow.push_back(std::vector<AStarNode3D*>());
		for (int i = 0; i < xNumSamples; i++) currRow.push_back(std::vector<AStarNode3D*>());

		float subTest = mMaxWaynodeDist / 4;

			for (float x = minX; x < maxX; x+=mMaxWaynodeDist)
			{
				for (int i = 0; i < xNumSamples; i++)
				{
					lastRow[i].clear();
					lastRow[i].assign(currRow[i].begin(), currRow[i].end());
				}
				for (int i = 0; i < xNumSamples; i++) currRow[i].clear();
				rowIndex = 0;

				Ogre::Vector3 origin = Ogre::Vector3(x, maxY, 0);
				for (float z = minZ; z < maxZ; z+=mMaxWaynodeDist)
				{
					origin.z = z;
					rasterNodeRow(currRow[rowIndex], origin, subTest, castDist);
						//Add neighbours
						if (rowIndex > 0)
						{
							addMatchingNeighbours(currRow[rowIndex], currRow[rowIndex-1]);//node->AddNeighbour(currRow[rowIndex-1]);
							addMatchingNeighbours(currRow[rowIndex], lastRow[rowIndex-1]);//node->AddNeighbour(lastRow[rowIndex-1]);
						}
						addMatchingNeighbours(currRow[rowIndex], lastRow[rowIndex]);//node->AddNeighbour(lastRow[rowIndex]);
						if (rowIndex < xNumSamples-1)
						{
							addMatchingNeighbours(currRow[rowIndex], lastRow[rowIndex+1]);//node->AddNeighbour(lastRow[rowIndex+1]);
						}
						//currRow[rowIndex] = node;
						//mPathNodeTree->AddPathNode(node);
					/*OgrePhysX::Scene::QueryHit query;
					if (Ice::Main::Instance().GetPhysXScene()->raycastClosestShape(query, Ogre::Ray(origin, Ogre::Vector3(0,-1,0)), NX_STATIC_SHAPES, 1<<CollisionGroups::AI, castDist))
					{
						OgrePhysX::Scene::QueryHit dummyQuery;
						if (Ice::Main::Instance().GetPhysXScene()->raycastClosestShape(dummyQuery, Ogre::Ray(origin + Ogre::Vector3(mMinBorder, 0, 0), Ogre::Vector3(0,-1,0)), NX_STATIC_SHAPES, 1<<CollisionGroups::AI, castDist)
						&& Ice::Main::Instance().GetPhysXScene()->raycastClosestShape(dummyQuery, Ogre::Ray(origin + Ogre::Vector3(-mMinBorder, 0, 0), Ogre::Vector3(0,-1,0)), NX_STATIC_SHAPES, 1<<CollisionGroups::AI, castDist)
						&& Ice::Main::Instance().GetPhysXScene()->raycastClosestShape(dummyQuery, Ogre::Ray(origin + Ogre::Vector3(0, 0, mMinBorder), Ogre::Vector3(0,-1,0)), NX_STATIC_SHAPES, 1<<CollisionGroups::AI, castDist)
						&& Ice::Main::Instance().GetPhysXScene()->raycastClosestShape(dummyQuery, Ogre::Ray(origin + Ogre::Vector3(0, 0, -mMinBorder), Ogre::Vector3(0,-1,0)), NX_STATIC_SHAPES, 1<<CollisionGroups::AI, castDist))
						{
							GameObject *go = new GameObject();
							go->AddComponent(new MeshDebugRenderable("sphere.25cm.mesh"));
							go->SetGlobalPosition(query.point);
							AStarNode3D *node = new AStarNode3D(query.point);
							//Add neighbours
							if (rowIndex > 0)
							{
								if (currRow[rowIndex-1]) node->AddNeighbour(currRow[rowIndex-1]);
								if (lastRow[rowIndex-1]) node->AddNeighbour(lastRow[rowIndex-1]);
							}
							if (lastRow[rowIndex]) node->AddNeighbour(lastRow[rowIndex]);
							if (rowIndex < xNumSamples-1)
							{
								if (lastRow[rowIndex+1]) node->AddNeighbour(lastRow[rowIndex+1]);
							}
							currRow[rowIndex] = node;
							mPathNodeTree->AddPathNode(node);
						}
					}*/
					rowIndex++;
				}
			}
	}

	void NavigationMesh::ReceiveMessage(Msg &msg)
	{
		if (msg.mNewsgroup == "ACOTR_ONSLEEP")
		{
			NxActor *a = (NxActor*)msg.rawData;
			if (a->getGroup() != Ice::CollisionGroups::DEFAULT) return;
			NxBounds3 nxBounds;
			a->getShapes()[0]->getWorldBounds(nxBounds);
			Ogre::AxisAlignedBox box = OgrePhysX::Convert::toOgre(nxBounds);
			mPathNodeTree->InjectObstacle(a, box);
		}
		if (msg.mNewsgroup == "ACOTR_ONWAKE")
		{
			NxActor *a = (NxActor*)msg.rawData;
			if (a->getGroup() != Ice::CollisionGroups::DEFAULT) return;
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
		Clear();

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

		bakePhysXMesh();
		rasterNodes();
		//bakeWaynet();
		mNeedsUpdate = false;
	}

}