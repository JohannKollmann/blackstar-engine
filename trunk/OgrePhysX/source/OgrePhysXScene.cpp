
#include "OgrePhysXScene.h"
#include "OgrePhysXWorld.h"
#include "OgrePhysXRagdoll.h"
#include "extensions/PxDefaultCpuDispatcher.h"
#include "PxScene.h"
#include "PxMaterial.h"
#include "extensions/PxExtensionsAPI.h"
#include "common/PxRenderBuffer.h" 

namespace OgrePhysX
{

	Scene::Scene(void)
	{
		mVisualDebuggerEnabled = false;
		mDebugNode = nullptr;
		PxSceneDesc desc(World::getSingleton().getSDK()->getTolerancesScale());
		desc.gravity = PxVec3(0, -9.81f, 0);
		desc.filterShader = &PxDefaultSimulationFilterShader;
		create(desc);
	}
	Scene::Scene(PxSceneDesc &desc)
	{
		mVisualDebuggerEnabled = false;
		mDebugNode = nullptr;
		create(desc);
	}

	Scene::~Scene(void)
	{
		mPxScene->release();
	}

	void Scene::create(PxSceneDesc &desc)
	{
		desc.cpuDispatcher = PxDefaultCpuDispatcherCreate(4);
		if(!desc.cpuDispatcher)
		{
			Ogre::LogManager::getSingleton().logMessage("PxDefaultCpuDispatcherCreate failed!");
			return;
		}

		if (!World::getSingleton().getSDK())
		{
			Ogre::LogManager::getSingleton().logMessage("[OgrePhysX] Error: Cannot create scene because World is not initialised properly.");
			return;
		}
		mPxScene = World::getSingleton().getSDK()->createScene(desc);
		mTimeAccu = 0.0f;
		mFrameTime = 1/80.0f;
		mSimulationListener = nullptr;
	}

	Actor<PxRigidDynamic> Scene::createRigidDynamic(PxGeometry &geometry, float density, const Ogre::Vector3 &position, const Ogre::Quaternion &orientation)
	{
		return createRigidDynamic(geometry, density, World::getSingleton().getDefaultMaterial(), position, orientation);
	}
	Actor<PxRigidDynamic> Scene::createRigidDynamic(PxGeometry &geometry, float density, PxMaterial &material, const Ogre::Vector3 &position, const Ogre::Quaternion &orientation)
	{
		PxRigidDynamic *pxActor = PxCreateDynamic(*getPxPhysics(), PxTransform(toPx(position), toPx(orientation)), geometry, material, density);
		pxActor->setActorFlag(PxActorFlag::eVISUALIZATION, true);
		mPxScene->addActor(*pxActor);
		Actor<PxRigidDynamic> actor(pxActor);
		return actor;
	}

	Actor<PxRigidStatic> Scene::createRigidStatic(Ogre::MeshPtr mesh, Cooker::Params &cookerParams, const Ogre::Vector3 &position, const Ogre::Quaternion &orientation)
	{
		Cooker::AddedMaterials addedMaterials;
		cookerParams.materials(World::getSingleton().getOgreMaterialNames());
		PxRigidStatic *pxActor = getPxPhysics()->createRigidStatic(PxTransform(toPx(position), toPx(orientation)));
		PxTriangleMeshGeometry geometry = PxTriangleMeshGeometry(Cooker::getSingleton().createPxTriangleMesh(mesh, cookerParams, &addedMaterials));
		pxActor->createShape(
			geometry,
			addedMaterials.materials, addedMaterials.materialCount);
		pxActor->setActorFlag(PxActorFlag::eVISUALIZATION, true);
		mPxScene->addActor(*pxActor);
		Actor<PxRigidStatic> actor(pxActor);
		return actor;
	}

	Actor<PxRigidStatic> Scene::createRigidStatic(PxGeometry &geometry, const Ogre::Vector3 &position, const Ogre::Quaternion &orientation)
	{
		return createRigidStatic(geometry, World::getSingleton().getDefaultMaterial(), position, orientation);
	}
	Actor<PxRigidStatic> Scene::createRigidStatic(PxGeometry &geometry, PxMaterial &material, const Ogre::Vector3 &position, const Ogre::Quaternion &orientation)
	{
		PxRigidStatic *pxActor = getPxPhysics()->createRigidStatic(PxTransform(toPx(position), toPx(orientation)));
		pxActor->createShape(geometry, material);
		pxActor->setActorFlag(PxActorFlag::eVISUALIZATION, true);
		mPxScene->addActor(*pxActor);
		Actor<PxRigidStatic> actor(pxActor);
		return actor;
	}

	void Scene::removeActor(PxActor *actor)
	{
		mPxScene->removeActor(*actor);
	}

	RenderedActorBinding* Scene::createRenderedActorBinding(Actor<PxRigidDynamic> &actor, PointRenderable *pointRenderable)
	{
		return createRenderedActorBinding(actor.getPxActor(), pointRenderable);
	}
	RenderedActorBinding* Scene::createRenderedActorBinding(PxRigidDynamic *actor, PointRenderable *pointRenderable)
	{
		RenderedActorBinding *renderedActor = new RenderedActorBinding(actor, pointRenderable);
		mOgrePhysXBindings.push_back(renderedActor);
		return renderedActor;
	}
	void Scene::destroyRenderableBinding(RenderableBinding* binding)
	{
		for (std::list<RenderableBinding*>::iterator i = mOgrePhysXBindings.begin(); i != mOgrePhysXBindings.end(); i++)
		{
			if ((*i) == binding)
			{
				mOgrePhysXBindings.erase(i);
				break;
			}
		}
		delete binding;
	}

	Ragdoll* Scene::createRagdollBinding(Ogre::Entity *entity, Ogre::SceneNode *node)
	{
		Ragdoll *ragdoll = new Ragdoll(getPxScene(), entity, node);
		mOgrePhysXBindings.push_back(ragdoll);
		return ragdoll;
	}

	void Scene::syncRenderables()
	{
		for (std::list<RenderableBinding*>::iterator i = mOgrePhysXBindings.begin(); i != mOgrePhysXBindings.end(); i++)
		{
			(*i)->sync();
		}
	}

	PxScene* Scene::getPxScene()
	{
		return mPxScene;
	}

	void Scene::simulate(float time)
	{
		mTimeAccu += time;
		int numSubSteps = 0;
		while (mTimeAccu >= mFrameTime && numSubSteps < 12)
		{
			if (mSimulationListener) mSimulationListener->onBeginSimulate(mFrameTime);
			mPxScene->simulate(mFrameTime);
			if (mSimulationListener) mSimulationListener->onSimulate(mFrameTime);
			mPxScene->fetchResults(true);
			mTimeAccu -= mFrameTime;
			if (mSimulationListener) mSimulationListener->onEndSimulate(mFrameTime);
		}
	}

	bool Scene::raycastClosest(const Ogre::Vector3 &origin, const Ogre::Vector3 &unitDir, float maxDistance, Scene::RaycastHit &outHit)
	{
		PxRaycastHit hit;
		if (!mPxScene->raycastSingle(toPx(origin), toPx(unitDir), maxDistance, PxSceneQueryFlag::eIMPACT|PxSceneQueryFlag::eNORMAL|PxSceneQueryFlag::eDISTANCE, hit))
			return false;
		outHit.distance = hit.distance;
		outHit.normal = toOgre(hit.normal);
		outHit.position = toOgre(hit.impact);
		outHit.hitActorUserData = hit.actor->userData;
		return true;
	}
	bool Scene::raycastAny(const Ogre::Vector3 &origin, const Ogre::Vector3 &unitDir, float maxDistance)
	{
		PxRaycastHit hit;
		return mPxScene->raycastAny(toPx(origin), toPx(unitDir), maxDistance, hit);
	}

	void Scene::initVisualDebugger(Ogre::SceneManager *sceneMgr, Ogre::uint32 debugGeometryVisibilityFlags, bool enabled)
	{
		mOgreSceneMgr = sceneMgr;
		mVisualDebuggerVisibilityFlags = debugGeometryVisibilityFlags;

		if (!mDebugNode)
		{
			mDebugNode = sceneMgr->getRootSceneNode()->createChildSceneNode();

			//create debug line geometry
			mDebugLines = sceneMgr->createManualObject();
			mDebugLines->setCastShadows(false);
			mDebugLines->setVisibilityFlags(mVisualDebuggerVisibilityFlags);
			mDebugNode->attachObject(mDebugLines);

			//create wire material if necessary
			if (!Ogre::MaterialManager::getSingleton().resourceExists("OgrePhysXVisualDebugger_MeshMat"))
			{
				Ogre::MaterialPtr wireMat = Ogre::MaterialManager::getSingleton().create("OgrePhysXVisualDebugger_MeshMat", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
				wireMat->setAmbient(Ogre::ColourValue(0,0,1));
				wireMat->setDiffuse(Ogre::ColourValue(0,0,1));
				//wireMat->getTechnique(0)->getPass(0)->setPolygonMode(Ogre::PM_WIREFRAME);
			}

			//create mesh
			mDebugMesh = Ogre::MeshManager::getSingleton().createManual("OgrePhysXVisualDebugger_Mesh", "General");
			mDebugMesh->createSubMesh("main");

			Ogre::VertexData* data = new Ogre::VertexData();
			mDebugMesh->sharedVertexData = data;
			data->vertexCount = 0;
			Ogre::VertexDeclaration* decl = data->vertexDeclaration;
			decl->addElement(0, 0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);

			mDebugMesh->getSubMesh(0)->indexData->indexStart = 0;
			mDebugMesh->getSubMesh(0)->indexData->indexCount = 0;		
			mDebugMesh->getSubMesh(0)->setMaterialName("OgrePhysXVisualDebugger_MeshMat");

			Ogre::AxisAlignedBox bounds(Ogre::Vector3(-10000,-10000,-10000), Ogre::Vector3(10000,10000,10000));
			mDebugMesh->_setBounds(bounds);

			//create entity
			mDebugMeshEntity = sceneMgr->createEntity("OgrePhysXDebugMeshEntity", "OgrePhysXVisualDebugger_Mesh");
			mDebugMeshEntity->setCastShadows(false);
			mDebugMeshEntity->setVisibilityFlags(mVisualDebuggerVisibilityFlags);

			mDebugNode->attachObject(mDebugMeshEntity);
		}

		setVisualDebuggerEnabled(enabled);
	}
	void Scene::setVisualDebuggerEnabled(bool enabled)
	{
		mVisualDebuggerEnabled = enabled;

		float factor = 0;
		if (enabled) factor = 1.05f;
		mPxScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, factor);
		mPxScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, factor);
	}

	void Scene::renderDebugGeometry()
	{
		if (!mVisualDebuggerEnabled) return;

		mPxScene->getRenderBuffer();

		//update mesh
		Ogre::VertexData* data = mDebugMesh->sharedVertexData;
		Ogre::VertexDeclaration* decl = data->vertexDeclaration;

		unsigned int numtriangles = mPxScene->getRenderBuffer().getNbTriangles();
		unsigned int numIndices = mPxScene->getRenderBuffer().getNbTriangles()*3;
		unsigned int numVertices = numIndices;

		unsigned int numPoints = mPxScene->getRenderBuffer().getNbPoints();

		unsigned int numLines = mPxScene->getRenderBuffer().getNbLines();

		if (numLines > 0)
		{
			unsigned int numLineVertices = 0;
			if (mDebugLines->getNumSections() == 1) numLineVertices = mDebugLines->getSection(0)->getRenderOperation()->vertexData->vertexCount;
			if (numLines != numLineVertices / 2)
			{
				mDebugLines->clear();
				mDebugLines->begin("OgrePhysXVisualDebugger_MeshMat", Ogre::RenderOperation::OT_LINE_LIST);
				for (unsigned int i = 0; i < numLines; ++i)
				{
					mDebugLines->position(toOgre(mPxScene->getRenderBuffer().getLines()[i].pos0));
					mDebugLines->position(toOgre(mPxScene->getRenderBuffer().getLines()[i].pos1));
				}
				mDebugLines->end();
			}
		}

		if (numVertices > 0)
		{
			Ogre::HardwareVertexBufferSharedPtr vBuf;
			if (data->vertexCount != numVertices)
			{
				Ogre::HardwareVertexBufferSharedPtr vBuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
				decl->getVertexSize(0), numVertices, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
				Ogre::VertexBufferBinding* bind = data->vertexBufferBinding;
				bind->setBinding(0, vBuf);
			}
			else vBuf = data->vertexBufferBinding->getBuffer(0);

			data->vertexCount = mPxScene->getRenderBuffer().getNbPoints();

			float* afVertexData=(float*)vBuf->lock(Ogre::HardwareBuffer::HBL_DISCARD);
			unsigned int index = 0;
			for (unsigned int i = 0; i < numtriangles; ++i)
			{
				PxVec3 pos0 = mPxScene->getRenderBuffer().getTriangles()[i].pos0;
				PxVec3 pos1 = mPxScene->getRenderBuffer().getTriangles()[i].pos1;
				PxVec3 pos2 = mPxScene->getRenderBuffer().getTriangles()[i].pos2;
				afVertexData[index++] = pos0.x; afVertexData[index++] = pos0.y; afVertexData[index++] = pos0.z;
				afVertexData[index++] = pos1.x; afVertexData[index++] = pos1.y; afVertexData[index++] = pos1.z;
				afVertexData[index++] = pos2.x; afVertexData[index++] = pos2.y; afVertexData[index++] = pos2.z;
			}
			vBuf->unlock();

			Ogre::HardwareIndexBufferSharedPtr iBuf;
			if (mDebugMesh->getSubMesh(0)->indexData->indexCount != numIndices)
			{
				iBuf = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
					Ogre::HardwareIndexBuffer::IT_32BIT, numIndices, Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
				mDebugMesh->getSubMesh(0)->indexData->indexBuffer = iBuf;
			}
			else iBuf = mDebugMesh->getSubMesh(0)->indexData->indexBuffer;
			mDebugMesh->getSubMesh(0)->indexData->indexCount = numIndices;

			unsigned int* aiIndexBuf=(unsigned int*)iBuf->lock(Ogre::HardwareBuffer::HBL_DISCARD);
			index = 0;
			for (unsigned int i = 0; i < numIndices; ++i)
			{
				aiIndexBuf[i] = index++;
			}
			iBuf->unlock();
		}
	}

}