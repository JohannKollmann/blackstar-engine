/*
This source file is part of OgrePhysX.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "OgrePhysXScene.h"
#include "OgrePhysXWorld.h"
#include "OgrePhysXRagdoll.h"
#include "OgrePhysXDestructible.h"
#include "extensions/PxDefaultCpuDispatcher.h"
#include "PxScene.h"
#include "PxMaterial.h"
#include "extensions/PxExtensionsAPI.h"
#include "common/PxRenderBuffer.h" 

namespace OgrePhysX
{

	Scene::Scene(Ogre::SceneManager *ogreSceneMgr) : mOgreSceneMgr(ogreSceneMgr)
	{
		PxSceneDesc desc(World::getSingleton().getPxPhysics()->getTolerancesScale());
		desc.gravity = PxVec3(0, -9.81f, 0);
		desc.filterShader = &PxDefaultSimulationFilterShader;
		create(desc);
	}
	Scene::Scene(Ogre::SceneManager *ogreSceneMgr, PxSceneDesc &desc) : mOgreSceneMgr(ogreSceneMgr)
	{
		create(desc);
	}

	Scene::~Scene()
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

	Actor<PxRigidDynamic> Scene::createRigidDynamic(PxGeometry &geometry, float density, const PxTransform &shapeOffset, const PxTransform &actorPose)
	{
		return createRigidDynamic(geometry, density, World::getSingleton().getDefaultMaterial(), shapeOffset, actorPose);
	}
	Actor<PxRigidDynamic> Scene::createRigidDynamic(PxGeometry &geometry, float density, PxMaterial &material, const PxTransform &shapeOffset, const PxTransform &actorPose)
	{
		PxRigidDynamic *pxActor = PxCreateDynamic(*getPxPhysics(), actorPose, geometry, material, density, shapeOffset);
		mPxScene->addActor(*pxActor);
		Actor<PxRigidDynamic> actor(pxActor);
		return actor;
	}

	Actor<PxRigidDynamic> Scene::createRigidDynamic(Ogre::Entity *entity, float density, const Ogre::Vector3 &scale, const PxTransform &actorPose)
	{
		return createRigidDynamic(entity, density, World::getSingleton().getDefaultMaterial(), scale, actorPose);
	}

	Actor<PxRigidDynamic> Scene::createRigidDynamic(Ogre::Entity *entity, float density, PxMaterial &material, const Ogre::Vector3 &scale, const PxTransform &actorPose)
	{
		PxTransform shapeOffset = PxTransform::createIdentity();

		Cooker::Params params;
		params.scale(scale);
		if (entity->getBoundingBox().getSize().x * scale.x > 0.4f &&
			entity->getBoundingBox().getSize().y * scale.y > 0.4f &&
			entity->getBoundingBox().getSize().z * scale.z > 0.4f)
			return createRigidDynamic(Geometry::convexMeshGeometry(entity->getMesh(), params), density, material, shapeOffset, actorPose); 

		//mesh is too small for convex mesh
		PxBoxGeometry boxGeometry = Geometry::boxGeometry(entity, shapeOffset, scale);
		return createRigidDynamic(boxGeometry, density, material, shapeOffset, actorPose); 
	}

	Actor<PxRigidStatic> Scene::createRigidStatic(Ogre::Entity *entity, Cooker::Params &cookerParams, const PxTransform &actorPose)
	{
		Cooker::AddedMaterials addedMaterials;
		cookerParams.materials(World::getSingleton().getOgreMaterialNames());
		PxRigidStatic *pxActor = getPxPhysics()->createRigidStatic(actorPose);
		PxTriangleMeshGeometry geometry = PxTriangleMeshGeometry(Cooker::getSingleton().createPxTriangleMesh(entity->getMesh(), cookerParams, &addedMaterials));
		
		if (addedMaterials.materialCount > 0)
			pxActor->createShape(geometry, addedMaterials.materials, addedMaterials.materialCount);
		else
			pxActor->createShape(geometry, World::getSingleton().getDefaultMaterial());

		mPxScene->addActor(*pxActor);
		Actor<PxRigidStatic> actor(pxActor);
		return actor;
	}

	Actor<PxRigidStatic> Scene::createRigidStatic(PxGeometry &geometry, const PxTransform &actorPose)
	{
		return createRigidStatic(geometry, World::getSingleton().getDefaultMaterial(), actorPose);
	}
	Actor<PxRigidStatic> Scene::createRigidStatic(PxGeometry &geometry, PxMaterial &material, const PxTransform &actorPose)
	{
		PxRigidStatic *pxActor = getPxPhysics()->createRigidStatic(actorPose);
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

	Destructible* Scene::createDestructible(const Ogre::String &meshSplitConfigFile, PxMaterial &material, float breakForce, float breakTorque, float density, const Ogre::Vector3 &scale)
	{
		Destructible *destructible = new Destructible(this, meshSplitConfigFile, breakForce, breakTorque, density, material, scale);
		mOgrePhysXBindings.push_back(destructible);
		return destructible;
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
		outHit.hitActorUserData = hit.shape->getActor().userData;
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
		outHit.hitActorUserData = hit.shape->getActor().userData;
		return true;
	}


}