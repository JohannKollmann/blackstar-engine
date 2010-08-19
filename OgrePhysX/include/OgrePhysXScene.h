#pragma once

#include "OgrePhysXClasses.h"
#include "NxScene.h"
#include "OgrePhysXRenderableBinding.h"
#include "OgrePhysXActor.h"
#include "OgrePhysXRenderedActor.h"

namespace OgrePhysX
{

	class OgrePhysXClass Scene
	{
		friend class World;

	private:
		NxScene *mNxScene;

		std::list<RenderableBinding*> mOgrePhysXBindings;

		std::list<Actor*> mActors;

		std::map<Ogre::String, NxMaterialIndex> mMaterialBindings;

		void create(NxSceneDesc &desc);
		Scene(void);
		Scene(NxSceneDesc &desc);
		~Scene(void);

	public:
		
		NxScene* getNxScene();

		/*
		bindMaterial
		Binds an ogre material to a PhysX material.
		By default, all ogre materials are bound to the physX default material (0).
		Setup your material binds BEFORE you cook meshes.
		*/
		void bindMaterial(Ogre::String matName, NxMaterialIndex physXMat);

		/*
		getMaterialBinding
		returns the material index assigned to the Ogre material.
		returns 0 by default.
		*/
		NxMaterialIndex getMaterialBinding(Ogre::String matName);

		/*
		getMaterialBindings
		return all material bindings.
		*/
		std::map<Ogre::String, NxMaterialIndex>& getMaterialBindings();

		/*
		Creates an actor
		*/
		Actor* createActor(PrimitiveShape& shape, Ogre::Vector3 position = Ogre::Vector3(0,0,0), Ogre::Quaternion orientation = Ogre::Quaternion());
		Actor* createActor(BaseMeshShape& shape, Ogre::Vector3 position = Ogre::Vector3(0,0,0), Ogre::Quaternion orientation = Ogre::Quaternion());

		/*
		Creates an actor and binds a PointRenderable to it.
		Example usage:
		Ogre::SceneNode *node = ...
		Ogre::Entity *ent = ...
		node->attachObject(ent);
		mScene->createRenderedActor(new NodeRenderable(node), PrimitiveShape::Box(ent).density(10));
		*/
		RenderedActor* createRenderedActor(PointRenderable *PointRenderable, PrimitiveShape& shape, Ogre::Vector3 position = Ogre::Vector3(0,0,0), Ogre::Quaternion orientation = Ogre::Quaternion());
		RenderedActor* createRenderedActor(PointRenderable *PointRenderable, BaseMeshShape& shape, Ogre::Vector3 position = Ogre::Vector3(0,0,0), Ogre::Quaternion orientation = Ogre::Quaternion());

		/*
		Destroys an actor. 
		*/
		void destroyActor(Actor* actor);
		void destroyRenderedActor(RenderedActor* actor);

		void setContactReport(ContactReportListener *crl);
		void setTriggerReport(TriggerReportListener *trl);

		/*
		Experimental. Do not use this!
		*/
		Ragdoll* createRagdoll(Ogre::Entity *ent, Ogre::SceneNode *node, NxCollisionGroup boneCollisionGroup = 0);
		void destroyRagdoll(Ragdoll *rag);

		struct QueryHit
		{
			float distance;
			NxActor *hitActor;
			Ogre::Vector3 point;
			Ogre::Vector3 normal;
		};

		//Raycasting - this is only basic wrapping, if you want full control use PhysX directly!
		bool raycastAnyBounds(const Ogre::Ray &ray, NxShapesType shapeTypes = NX_ALL_SHAPES, NxU32 groups=0xffffffff, NxReal maxDist=NX_MAX_F32, NxU32 hintFlags=0xffffffff, const NxGroupsMask *groupsMask=0);
		bool raycastAnyShape(const Ogre::Ray &ray, NxShapesType shapeTypes = NX_ALL_SHAPES, NxU32 groups=0xffffffff, NxReal maxDist=NX_MAX_F32, NxU32 hintFlags=0xffffffff, const NxGroupsMask *groupsMask=0);
		bool raycastClosestBounds(QueryHit &result, const Ogre::Ray &ray, NxShapesType shapeTypes = NX_ALL_SHAPES, NxU32 groups=0xffffffff, NxReal maxDist=NX_MAX_F32, NxU32 hintFlags=0xffffffff, const NxGroupsMask *groupsMask=0);
		bool raycastClosestShape(QueryHit &result, const Ogre::Ray &ray, NxShapesType shapeTypes = NX_ALL_SHAPES, NxU32 groups=0xffffffff, NxReal maxDist=NX_MAX_F32, NxU32 hintFlags=0xffffffff, const NxGroupsMask *groupsMask=0);
		void raycastAllBounds(std::vector<QueryHit> &result, const Ogre::Ray &ray, NxShapesType shapeTypes = NX_ALL_SHAPES, NxU32 groups=0xffffffff, NxReal maxDist=NX_MAX_F32, NxU32 hintFlags=0xffffffff, const NxGroupsMask *groupsMask=0);
		void raycastAllShapes(std::vector<QueryHit> &result, const Ogre::Ray &ray, NxShapesType shapeTypes = NX_ALL_SHAPES, NxU32 groups=0xffffffff, NxReal maxDist=NX_MAX_F32, NxU32 hintFlags=0xffffffff, const NxGroupsMask *groupsMask=0);
		class RaycastReport : public NxUserRaycastReport
		{
		public:
			std::vector<QueryHit> *mReport;
			RaycastReport(std::vector<QueryHit> &report) : mReport(&report) {}
			~RaycastReport() {}
			bool onHit(const NxRaycastHit& hit);
		};

		//Forcefields - untested!
		/*
		Important note: You don't have to fill NxForceFieldDesc::kernel. This method does that for you.
		*/
		NxForceField* createForceField(BoxShape &shape, NxForceFieldDesc &fieldDesc, NxForceFieldLinearKernelDesc &linearKernelDesc);
		NxForceField* createForceField(SphereShape &shape, NxForceFieldDesc &fieldDesc, NxForceFieldLinearKernelDesc &linearKernelDesc);
		NxForceField* createForceField(CapsuleShape &shape, NxForceFieldDesc &fieldDesc, NxForceFieldLinearKernelDesc &linearKernelDesc);
		NxForceField* createForceField(RTConvexMeshShape &shape, NxForceFieldDesc &fieldDesc, NxForceFieldLinearKernelDesc &linearKernelDesc);
		void destroyForcefield(NxForceField *forceField);

		void syncRenderables();
	};

}