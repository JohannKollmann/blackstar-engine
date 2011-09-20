
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
		PxSceneDesc desc(World::getSingleton().getPxPhysics()->getTolerancesScale());
		desc.gravity = PxVec3(0, -9.81f, 0);
		desc.filterShader = &PxDefaultSimulationFilterShader;
		create(desc);
	}
	Scene::Scene(PxSceneDesc &desc)
	{
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

		if (!World::getSingleton().getPxPhysics())
		{
			Ogre::LogManager::getSingleton().logMessage("[OgrePhysX] Error: Cannot create scene because World is not initialised properly.");
			return;
		}
		mPxScene = World::getSingleton().getPxPhysics()->createScene(desc);
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
		
		if (addedMaterials.materialCount > 0)
			pxActor->createShape(geometry, addedMaterials.materials, addedMaterials.materialCount);
		else
			pxActor->createShape(geometry, World::getSingleton().getDefaultMaterial());

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
		for (auto i = mOgrePhysXBindings.begin(); i != mOgrePhysXBindings.end(); i++)
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
		for (auto i = mOgrePhysXBindings.begin(); i != mOgrePhysXBindings.end(); i++)
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

	bool Scene::raycastClosest(const Ogre::Vector3 &origin, const Ogre::Vector3 &unitDir, float maxDistance, PxSceneQueryFilterData &queryFilterData)
	{
		PxRaycastHit hit;
		return mPxScene->raycastSingle(toPx(origin), toPx(unitDir), maxDistance, PxSceneQueryFlag::eIMPACT|PxSceneQueryFlag::eNORMAL|PxSceneQueryFlag::eDISTANCE, hit, queryFilterData);
	}
	bool Scene::raycastClosest(const Ogre::Vector3 &origin, const Ogre::Vector3 &unitDir, float maxDistance, RaycastHit &outHit, PxSceneQueryFilterData &queryFilterData)
	{
		PxRaycastHit hit;
		if (!mPxScene->raycastSingle(toPx(origin), toPx(unitDir), maxDistance, PxSceneQueryFlag::eIMPACT|PxSceneQueryFlag::eNORMAL|PxSceneQueryFlag::eDISTANCE, hit, queryFilterData))
			return false;
		outHit.distance = hit.distance;
		outHit.normal = toOgre(hit.normal);
		outHit.position = toOgre(hit.impact);
		outHit.hitActorUserData = hit.actor->userData;
		return true;
	}

	bool Scene::raycastAny(const Ogre::Vector3 &origin, const Ogre::Vector3 &unitDir, float maxDistance, PxSceneQueryFilterData &queryFilterData)
	{
		PxRaycastHit hit;
		return mPxScene->raycastAny(toPx(origin), toPx(unitDir), maxDistance, hit, queryFilterData);
	}
	bool Scene::raycastAny(const Ogre::Vector3 &origin, const Ogre::Vector3 &unitDir, float maxDistance, RaycastHit &outHit, PxSceneQueryFilterData &queryFilterData)
	{
		PxRaycastHit hit;
		if (!mPxScene->raycastAny(toPx(origin), toPx(unitDir), maxDistance, hit, queryFilterData))
			return false;
		outHit.distance = hit.distance;
		outHit.normal = toOgre(hit.normal);
		outHit.position = toOgre(hit.impact);
		outHit.hitActorUserData = hit.actor->userData;
		return true;
	}


}