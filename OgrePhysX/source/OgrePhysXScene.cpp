
#include "OgrePhysXScene.h"
#include "OgrePhysXWorld.h"
#include "OgrePhysXRagdoll.h"
#include "OgrePhysXContactReport.h"
#include "OgrePhysXTriggerReport.h"

namespace OgrePhysX
{

	Scene::Scene(void)
	{
		NxSceneDesc desc;
		desc.gravity = NxVec3(0, -9.81f, 0);
		desc.simType = NX_SIMULATION_SW;
		create(desc);
	}
	Scene::Scene(NxSceneDesc &desc)
	{
		create(desc);
	}

	Scene::~Scene(void)
	{
		if (!World::getSingleton().getSDK())
			World::getSingleton().getSDK()->releaseScene(*mNxScene);
	}

	void Scene::create(NxSceneDesc &desc)
	{
		if (!World::getSingleton().getSDK())
		{
			Ogre::LogManager::getSingleton().logMessage("[OgrePhysX] Error: Cannot create scene because World is not initialised properly.");
			return;
		}
		mNxScene = World::getSingleton().getSDK()->createScene(desc);
		mNxScene->setTiming(1/80.0f, 12, NX_TIMESTEP_FIXED);
		mTimeAccu = 0.0f;
		mFrameTime = 1/80.0f;
		mSimulationListener = nullptr;

		NxMaterial* defaultMaterial = mNxScene->getMaterialFromIndex(0);
		defaultMaterial->setRestitution(0.1f);
		defaultMaterial->setStaticFriction(0.5f);
		defaultMaterial->setDynamicFriction(0.5f);
	}

	void Scene::bindMaterial(Ogre::String matName, NxMaterialIndex physXMat)
	{
		std::map<Ogre::String, NxMaterialIndex>::iterator i  = mMaterialBindings.find(matName);
		if (i != mMaterialBindings.end()) i->second = physXMat;
		else 
			mMaterialBindings.insert(std::make_pair<Ogre::String, NxMaterialIndex>(matName, physXMat));
	}

	NxMaterialIndex Scene::getMaterialBinding(Ogre::String matName)
	{
		std::map<Ogre::String, NxMaterialIndex>::iterator i  = mMaterialBindings.find(matName);
		if (i != mMaterialBindings.end()) return i->second;
		return 0;
	}

	std::map<Ogre::String, NxMaterialIndex>& Scene::getMaterialBindings()
	{
		return mMaterialBindings;
	}

	Actor* Scene::createActor(PrimitiveShape& shape, Ogre::Vector3 position, Ogre::Quaternion orientation)
	{
		Actor *actor = new Actor(mNxScene, shape, position, orientation);
		mActors.push_back(actor);
		return actor;
	}
	RenderedActor* Scene::createRenderedActor(PointRenderable *pointRenderable, PrimitiveShape& shape, Ogre::Vector3 position, Ogre::Quaternion orientation)
	{
		RenderedActor *actor = new RenderedActor(mNxScene, pointRenderable, shape, position, orientation);
		mOgrePhysXBindings.push_back(actor);
		return actor;
	}
	Actor* Scene::createActor(BaseMeshShape& shape, Ogre::Vector3 position, Ogre::Quaternion orientation)
	{
		Actor *actor = new Actor(mNxScene, shape, position, orientation);
		mActors.push_back(actor);
		return actor;
	}
	RenderedActor* Scene::createRenderedActor(PointRenderable *pointRenderable, BaseMeshShape& shape, Ogre::Vector3 position, Ogre::Quaternion orientation)
	{
		RenderedActor *actor = new RenderedActor(mNxScene, pointRenderable, shape, position, orientation);
		mOgrePhysXBindings.push_back(actor);
		return actor;
	}
	void Scene::destroyActor(Actor* actor)
	{
		for (std::list<Actor*>::iterator i = mActors.begin(); i != mActors.end(); i++)
		{
			if ((*i) == actor)
			{
				mActors.erase(i);
				break;
			}
		}
		delete actor;
	}
	void Scene::destroyRenderedActor(RenderedActor* actor)
	{
		for (std::list<RenderableBinding*>::iterator i = mOgrePhysXBindings.begin(); i != mOgrePhysXBindings.end(); i++)
		{
			if ((*i) == actor)
			{
				mOgrePhysXBindings.erase(i);
				break;
			}
		}
		delete actor;
	}

	void Scene::setContactReport(ContactReportListener *crl)
	{
		mNxScene->setUserContactReport(new ContactReport(crl));
	}
	void Scene::setTriggerReport(TriggerReportListener *trl)
	{
		mNxScene->setUserTriggerReport(new TriggerReport(trl));
	}

	Ragdoll* Scene::createRagdoll(Ogre::Entity *ent, Ogre::SceneNode *node, NxCollisionGroup boneCollisionGroup)
	{
		Ragdoll *rag = new Ragdoll(getNxScene(), ent, node, boneCollisionGroup);
		mOgrePhysXBindings.push_back(rag);
		return rag;
	}
	void Scene::destroyRagdoll(Ragdoll *rag)
	{
		mOgrePhysXBindings.remove(rag);
		delete rag;
	}

	void Scene::syncRenderables()
	{
		for (std::list<RenderableBinding*>::iterator i = mOgrePhysXBindings.begin(); i != mOgrePhysXBindings.end(); i++)
		{
			(*i)->sync();
		}
	}

	bool Scene::raycastAnyBounds(const Ogre::Ray &ray, NxShapesType shapeTypes, NxU32 groups, NxReal maxDist, NxU32 hintFlags, const NxGroupsMask *groupsMask)
	{
		NxRay nRay(Convert::toNx(ray.getOrigin()), Convert::toNx(ray.getDirection()));
		return mNxScene->raycastAnyBounds(nRay, shapeTypes, groups, maxDist, groupsMask);
	}
	bool Scene::raycastAnyShape(const Ogre::Ray &ray, NxShapesType shapeTypes, NxU32 groups, NxReal maxDist, NxU32 hintFlags, const NxGroupsMask *groupsMask)
	{
		NxRay nRay(Convert::toNx(ray.getOrigin()), Convert::toNx(ray.getDirection()));
		return mNxScene->raycastAnyShape(nRay, shapeTypes, groups, maxDist, groupsMask);
	}
	bool Scene::raycastClosestBounds(QueryHit &result, const Ogre::Ray &ray, NxShapesType shapeTypes, NxU32 groups, NxReal maxDist, NxU32 hintFlags, const NxGroupsMask *groupsMask)
	{
		NxRay nRay(Convert::toNx(ray.getOrigin()), Convert::toNx(ray.getDirection()));
		NxRaycastHit hit;
		result.hitActor = nullptr;
		NxShape *shape = mNxScene->raycastClosestBounds(nRay, shapeTypes, hit, groups, maxDist, hintFlags, groupsMask);
		if (shape)
		{
			result.hitActor = &shape->getActor();
			result.distance = hit.distance;
			result.normal = Convert::toOgre(hit.worldNormal);
			result.point = Convert::toOgre(hit.worldImpact);
			return true;
		}
		return false;
	}
	bool Scene::raycastClosestShape(QueryHit &result, const Ogre::Ray &ray, NxShapesType shapeTypes, NxU32 groups, NxReal maxDist, NxU32 hintFlags, const NxGroupsMask *groupsMask)
	{
		NxRay nRay(Convert::toNx(ray.getOrigin()), Convert::toNx(ray.getDirection()));
		NxRaycastHit hit;
		result.hitActor = nullptr;
		NxShape *shape = mNxScene->raycastClosestShape(nRay, shapeTypes, hit, groups, maxDist, hintFlags, groupsMask);
		if (shape)
		{
			result.hitActor = &shape->getActor();
			result.distance = hit.distance;
			result.normal = Convert::toOgre(hit.worldNormal);
			result.point = Convert::toOgre(hit.worldImpact);
			return true;
		}
		return false;
	}
	void Scene::raycastAllBounds(std::vector<QueryHit> &result, const Ogre::Ray &ray, NxShapesType shapeTypes, NxU32 groups, NxReal maxDist, NxU32 hintFlags, const NxGroupsMask *groupsMask)
	{
		NxRay nRay(Convert::toNx(ray.getOrigin()), Convert::toNx(ray.getDirection()));
		RaycastReport report(result);
		mNxScene->raycastAllBounds(nRay, report, shapeTypes, groups, maxDist, hintFlags, groupsMask);
	}
	void Scene::raycastAllShapes(std::vector<QueryHit> &result, const Ogre::Ray &ray, NxShapesType shapeTypes, NxU32 groups, NxReal maxDist, NxU32 hintFlags, const NxGroupsMask *groupsMask)
	{
		NxRay nRay(Convert::toNx(ray.getOrigin()), Convert::toNx(ray.getDirection()));
		RaycastReport report(result);
		mNxScene->raycastAllShapes(nRay, report, shapeTypes, groups, maxDist, hintFlags, groupsMask);
	}
	bool Scene::RaycastReport::onHit(const NxRaycastHit &hit)
	{
		QueryHit sHit;
		sHit.distance = hit.distance;
		sHit.normal = Convert::toOgre(hit.worldNormal);
		sHit.point = Convert::toOgre(hit.worldImpact);
		sHit.hitActor = &hit.shape->getActor();
		mReport->push_back(sHit);
		return true;
	}

	NxForceField* Scene::createForceField(BoxShape &shape, NxForceFieldDesc &fieldDesc, NxForceFieldLinearKernelDesc &linearKernelDesc)
	{
		NxForceFieldLinearKernel* linearKernel = mNxScene->createForceFieldLinearKernel(linearKernelDesc);
		fieldDesc.kernel = linearKernel;
		NxForceField *forceField = mNxScene->createForceField(fieldDesc);

		NxBoxForceFieldShapeDesc boxDesc;
		NxBoxShapeDesc *desc = (NxBoxShapeDesc*)shape.getDesc();
		boxDesc.dimensions = desc->dimensions;
		boxDesc.pose = desc->localPose;
		forceField->getIncludeShapeGroup().createShape(boxDesc);

		return forceField;
	}
	NxForceField* Scene::createForceField(SphereShape &shape, NxForceFieldDesc &fieldDesc, NxForceFieldLinearKernelDesc &linearKernelDesc)
	{
		NxForceFieldLinearKernel* linearKernel = mNxScene->createForceFieldLinearKernel(linearKernelDesc);
		fieldDesc.kernel = linearKernel;
		NxForceField *forceField = mNxScene->createForceField(fieldDesc);

		NxSphereForceFieldShapeDesc sphereDesc;
		NxSphereShapeDesc *desc = (NxSphereShapeDesc*)shape.getDesc();
		sphereDesc.radius = desc->radius;
		sphereDesc.pose = desc->localPose;
		forceField->getIncludeShapeGroup().createShape(sphereDesc);

		return forceField;
	}
	NxForceField* Scene::createForceField(CapsuleShape &shape, NxForceFieldDesc &fieldDesc, NxForceFieldLinearKernelDesc &linearKernelDesc)
	{
		NxForceFieldLinearKernel* linearKernel = mNxScene->createForceFieldLinearKernel(linearKernelDesc);
		fieldDesc.kernel = linearKernel;
		NxForceField *forceField = mNxScene->createForceField(fieldDesc);

		NxCapsuleForceFieldShapeDesc capsuleDesc;
		NxCapsuleShapeDesc *desc = (NxCapsuleShapeDesc*)shape.getDesc();
		capsuleDesc.radius = desc->radius;
		capsuleDesc.height = desc->height;
		capsuleDesc.pose = desc->localPose;
		forceField->getIncludeShapeGroup().createShape(capsuleDesc);

		return forceField;
	}
	NxForceField* Scene::createForceField(RTConvexMeshShape &shape, NxForceFieldDesc &fieldDesc, NxForceFieldLinearKernelDesc &linearKernelDesc)
	{
		NxForceFieldLinearKernel* linearKernel = mNxScene->createForceFieldLinearKernel(linearKernelDesc);
		fieldDesc.kernel = linearKernel;
		NxForceField *forceField = mNxScene->createForceField(fieldDesc);

		NxConvexForceFieldShapeDesc convexDesc;
		NxConvexShapeDesc *desc = (NxConvexShapeDesc*)shape.getDesc();
		convexDesc.meshData = desc->meshData;
		convexDesc.pose = desc->localPose;
		forceField->getIncludeShapeGroup().createShape(convexDesc);

		return forceField;
	}
	void Scene::destroyForcefield(NxForceField *forceField)
	{
		NxForceFieldLinearKernel *kernel = (NxForceFieldLinearKernel*)forceField->getForceFieldKernel();
		mNxScene->releaseForceField(*forceField);
		mNxScene->releaseForceFieldLinearKernel(*kernel);
	}

	NxScene* Scene::getNxScene()
	{
		return mNxScene;
	}

	void Scene::simulate(float time)
	{
		mTimeAccu += time;
		int numSubSteps = 0;
		while (mTimeAccu >= mFrameTime && numSubSteps < 12)
		{
			if (mSimulationListener) mSimulationListener->onBeginSimulate(mFrameTime);
			mNxScene->simulate(mFrameTime);
			if (mSimulationListener) mSimulationListener->onSimulate(mFrameTime);
			mNxScene->flushStream();
			mNxScene->fetchResults(NX_RIGID_BODY_FINISHED, true);
			mTimeAccu -= mFrameTime;
			if (mSimulationListener) mSimulationListener->onEndSimulate(mFrameTime);
		}
	}

}