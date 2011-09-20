
#include "IceNavigationMesh.h"
#include "IceGameObject.h"
#include "IceAIManager.h"
#include "IceMain.h"
#include "IceGameObject.h"
#include "IceGOCView.h"
#include "IceGOCPhysics.h"
#include "OgrePhysXStreams.h"

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
		Log::Instance().LogMessage("nah");
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
		mVertexBuffer.clear();
		mIndexBuffer.clear();
	}

	void NavigationMesh::Clear()
	{
		if (mPhysXActor)
		{
			Main::Instance().GetPhysXScene()->getPxScene()->removeActor(*mPhysXActor);
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
			Log::Instance().LogMessage("Error in NavigationMesh::ShortestPath: No start/end path nodes found!");
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

	void NavigationMesh::bakePhysXMesh()
	{
		if (mIndexBuffer.size() < 3) return;

		PxTriangleMeshDesc desc;

		desc.points.count = mVertexBuffer.size();
		desc.points.stride = 12;
		float *fVertices = new float[mVertexBuffer.size()*3];
		for (unsigned int i = 0; i < mVertexBuffer.size(); ++i)
		{
			fVertices[i*3] = mVertexBuffer[i].x;
			fVertices[i*3+1] = mVertexBuffer[i].y;
			fVertices[i*3+2] = mVertexBuffer[i].z;
		}
		desc.points.data = fVertices;
		
		desc.triangles.count = mIndexBuffer.size() / 3;
		desc.triangles.stride = 12;
		int *iIndices = new int[mIndexBuffer.size()];
		for (unsigned int i = 0; i < mIndexBuffer.size(); ++i)
			iIndices[i] = mIndexBuffer[i];
		desc.triangles.data = iIndices;

		OgrePhysX::MemoryStream stream;
		OgrePhysX::World::getSingleton().getCookingInterface()->cookTriangleMesh(desc, stream);
		stream.seek(0);

		delete[] fVertices;
		delete[]  iIndices;

		mPhysXActor = OgrePhysX::getPxPhysics()->createRigidStatic(PxTransform());
		mPhysXActor->createShape(PxTriangleMeshGeometry(OgrePhysX::getPxPhysics()->createTriangleMesh(stream)), OgrePhysX::World::getSingleton().getDefaultMaterial());
	}

	bool NavigationMesh::raycastClosest(const Ogre::Vector3 &origin, const Ogre::Vector3 &direction, float maxDist, PxRaycastHit &hit)
	{
		return (Ice::Main::Instance().GetPhysXScene()->getPxScene()->raycastSingle(OgrePhysX::Convert::toPx(origin),
			OgrePhysX::Convert::toPx(direction),
			maxDist,
			PxSceneQueryFlag::eNORMAL,
			hit));
	}

	bool NavigationMesh::borderTest(Ogre::Vector3 targetPoint, float size, float maxHeightDif)
	{
		targetPoint.y += maxHeightDif;
		float rayDist = maxHeightDif*2;
		if (!raycastClosest(targetPoint + Ogre::Vector3(size, 0, 0), Ogre::Vector3(0,-1,0), rayDist)) return false;
		if (!raycastClosest(targetPoint + Ogre::Vector3(-size, 0, 0), Ogre::Vector3(0,-1,0), rayDist)) return false;
		if (!raycastClosest(targetPoint + Ogre::Vector3(0, 0, size), Ogre::Vector3(0,-1,0), rayDist)) return false;
		if (!raycastClosest(targetPoint + Ogre::Vector3(0, 0, -size), Ogre::Vector3(0,-1,0), rayDist)) return false;
		return true;
	}

	bool NavigationMesh::checkAgainstLevelMesh(Ogre::Vector3 targetPoint, float extent, float heightOffset)
	{
		//check if the node volume intersetcs with the levelmesh
		targetPoint.y += heightOffset;
		PxBoxGeometry testVolume(PxVec3(extent, extent, extent));
		PxShape *hit;
		if (Main::Instance().GetPhysXScene()->getPxScene()->overlapAny(testVolume, PxTransform(OgrePhysX::Convert::toPx(targetPoint)), hit))
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
		PxRaycastHit hit;
		while (raycastClosest(rayOrigin, Ogre::Vector3(0,-1,0), rayDist, hit))
		{
			AStarNode3D* node = rasterNode(OgrePhysX::Convert::toOgre(hit.impact), subTest);
			if (node)
			{
				mPathNodeTree->AddPathNode(node);
				result.push_back(node);
			}
			rayOrigin.y = hit.impact.y - 1;
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

		float rayDist = box.getMaximum().y - box.getMinimum().y;
		for (float x = box.getMinimum().x; x < box.getMaximum().x; x += NODE_BORDER)
		{
			for (float z = box.getMinimum().z; z < box.getMaximum().z; z += NODE_BORDER)
			{
				if (raycastClosest(Ogre::Vector3(x, box.getMaximum().y, z),
					Ogre::Vector3::NEGATIVE_UNIT_Y,
					rayDist)) return false;
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

		PxRaycastHit hit;

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
				
				if (raycastClosest(rayOrigin, rayDir, rayDist))
					return false;

				if (!raycastClosest(rayOrigin+(rayDir*rayDist), Ogre::Vector3(0, -1, 0), 2, hit))
					return false;
	
				rayOrigin = OgrePhysX::Convert::toOgre(hit.impact);
			}
		}

		if (!checkAgainstLevelMesh(mid, NODE_BORDER, ((NODE_BORDER*2)/NODE_DIST) * MAX_HEIGHT_DIST_BETWEEN_NODES)) return false;
		mConnectionLinesDebugVisual->position(n1->GetGlobalPosition());
		mConnectionLinesDebugVisual->position(n2->GetGlobalPosition());
		return true;
	}

	void NavigationMesh::rasterNodes()
	{
		if (!mPhysXMesh) return;

		if (!mConnectionLinesDebugVisual) mConnectionLinesDebugVisual = Main::Instance().GetOgreSceneMgr()->createManualObject();
		mConnectionLinesDebugVisual->begin("BlueLine", Ogre::RenderOperation::OT_LINE_LIST);

		if (mPathNodeTree)
		{
			ICE_DELETE mPathNodeTree;
			mPathNodeTree = nullptr;
		}

		PxBounds3 aabb = mPhysXActor->getWorldBounds();
		float maxX = (aabb.maximum.x > aabb.minimum.x) ? aabb.maximum.x : aabb.minimum.x;
		float maxY = (aabb.maximum.y > aabb.minimum.y) ? aabb.maximum.y : aabb.minimum.y;
		float maxZ = (aabb.maximum.z > aabb.minimum.z) ? aabb.maximum.z : aabb.minimum.z;
		float minX = (aabb.maximum.x <= aabb.minimum.x) ? aabb.maximum.x : aabb.minimum.x;
		float minY = (aabb.maximum.y <= aabb.minimum.y) ? aabb.maximum.y : aabb.minimum.y;
		float minZ = (aabb.maximum.z <= aabb.minimum.z) ? aabb.maximum.z : aabb.minimum.z;
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
			PxRigidActor *a = (PxRigidActor*)msg.rawData;
			if (!a || a->getNbShapes() == 0) return;
			PxBounds3 nxBounds = a->getWorldBounds();
			Ogre::AxisAlignedBox box = OgrePhysX::Convert::toOgre(nxBounds);
			mPathNodeTree->InjectObstacle(a, box);
		}
		if (msg.typeID == GlobalMessageIDs::ACTOR_ONWAKE)
		{
			PxRigidActor *a = (PxRigidActor*)msg.rawData;
			PxBounds3 nxBounds = a->getWorldBounds();
			Ogre::AxisAlignedBox box = OgrePhysX::Convert::toOgre(nxBounds);
			mPathNodeTree->RemoveObstacle(a, box);
		}
	}

	void NavigationMesh::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("std::vector<Ogre::Vector3>", &mVertexBuffer, "Vertices");
		mgr.SaveAtom("std::vector<int>", &mIndexBuffer, "Indices");
	}
	void NavigationMesh::Load(LoadSave::LoadSystem& mgr)
	{
		Reset();

		mgr.LoadAtom("std::vector<Ogre::Vector3>", &mVertexBuffer);
		mgr.LoadAtom("std::vector<int>", &mIndexBuffer);

		Update();
	}

	void NavigationMesh::AddOgreMesh(Ogre::MeshPtr mesh, Transformable3D *transform)
	{
		OgrePhysX::Cooker::Params cp;
		cp.backfaces(false);
		OgrePhysX::Cooker::MeshInfo meshInfo;
		OgrePhysX::Cooker::getSingleton().getMeshInfo(mesh, cp, meshInfo);
		OgrePhysX::Cooker::getSingleton().mergeVertices(meshInfo);
		_cutBadPolys(meshInfo);
		OgrePhysX::Cooker::getSingleton().mergeVertices(meshInfo);

		mVertexBuffer.resize(mVertexBuffer.size() + meshInfo.vertices.size());
		mIndexBuffer.resize(mIndexBuffer.size() + meshInfo.indices.size());

		Ogre::Vector3 position(0,0,0);
		Ogre::Vector3 scale(1,1,1);
		Ogre::Quaternion orientation;
		if (transform)
		{
			position = transform->GetGlobalPosition();
			scale = transform->GetGlobalScale();
			orientation = transform->GetGlobalOrientation();
		}

		unsigned int indexOffset = mVertexBuffer.size();
		for (unsigned int i = 0; i < meshInfo.vertices.size(); i++)
		{
			mVertexBuffer.push_back(position + (scale * (orientation * meshInfo.vertices[i])));
		}
		for (unsigned int i = 0; i < meshInfo.indices.size(); i++)
		{
			mIndexBuffer.push_back(meshInfo.indices[i]+indexOffset);
		}
		mNeedsUpdate = true;
	}

	void NavigationMesh::_cutBadPolys(OgrePhysX::Cooker::MeshInfo &meshInfo)
	{
		std::vector<bool> neededVertices;
		neededVertices.resize(meshInfo.vertices.size(), false);

		std::vector<Ogre::Vector3> newVertices(meshInfo.vertices.size());
		unsigned int newVertexCount = 0;
		std::vector<int> newIndices(meshInfo.indices.size());
		unsigned int newTriCount = 0;
		for (unsigned int i = 0; i < meshInfo.indices.size(); i+=3)
		{
			Ogre::Vector3 normal = (meshInfo.vertices[meshInfo.indices[i+1]]-meshInfo.vertices[meshInfo.indices[i]]).crossProduct(meshInfo.vertices[meshInfo.indices[i+2]]-meshInfo.vertices[meshInfo.indices[i]]);
			normal.normalise();
			if (normal.dotProduct(Ogre::Vector3(0,1,0)) > 0.75f)
			{
				int indexOffset = newTriCount*3;
				newIndices[indexOffset] = meshInfo.indices[i];		neededVertices[meshInfo.indices[i]] = true;
				newIndices[indexOffset+1] = meshInfo.indices[i+1];	neededVertices[meshInfo.indices[i+1]] = true;
				newIndices[indexOffset+2] = meshInfo.indices[i+2];	neededVertices[meshInfo.indices[i+2]] = true;
				newTriCount++;
			}
		}
		std::vector<int> finalIndices = newIndices;
		for (unsigned int i = 0; i < meshInfo.vertices.size(); i++)
		{
			if (neededVertices[i])
			{
				newVertices[newVertexCount] = meshInfo.vertices[i];
				newVertexCount++;
			}
			else		//decrement indices above the deletes vertex
			{
				for (unsigned int x = 0; x < newTriCount*3; x++)
					if (newIndices[x] >= (int)i) finalIndices[x]--;
			}
		}
		meshInfo.vertices = newVertices;
		finalIndices.resize(newTriCount*3);
		meshInfo.indices = finalIndices;
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
			afVertexData[index++] = mVertexBuffer[i].x;
			afVertexData[index++] = mVertexBuffer[i].y;
			afVertexData[index++] = mVertexBuffer[i].z;
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