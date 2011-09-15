#pragma once

#include "OgrePhysXClasses.h"
#include "OgrePhysXRenderableBinding.h"
#include "OgrePhysXRenderedActorBinding.h"
#include "OgrePhysXActor.h"
#include "OgrePhysXCooker.h"

namespace OgrePhysX
{

	class OgrePhysXClass Scene
	{
		friend class World;

	public:
		class SimulationListener
		{
		public:
			virtual void onBeginSimulate(float time) {}
			virtual void onSimulate(float time) {}
			virtual void onEndSimulate(float time) {}
		};

	private:
		PxScene *mPxScene;

		Ogre::SceneManager *mOgreSceneMgr;
		bool mVisualDebuggerEnabled;
		Ogre::uint32 mVisualDebuggerVisibilityFlags;
		Ogre::SceneNode *mDebugNode;
		Ogre::MeshPtr mDebugMesh;
		Ogre::Entity *mDebugMeshEntity;
		Ogre::ManualObject *mDebugLines;

		float mTimeAccu;
		float mFrameTime;

		std::list<RenderableBinding*> mOgrePhysXBindings;

		SimulationListener *mSimulationListener;

		void create(PxSceneDesc &desc);
		Scene(void);
		Scene(PxSceneDesc &desc);
		~Scene(void);

	public:
		
		PxScene* getPxScene();

		/**
		Creates a new rigid dynamic actor with a specified collision shape and a default material and adds it to the scene.
		@return An Actor object wrapping the created PxActor.
		*/
		Actor<PxRigidDynamic> createRigidDynamic(PxGeometry &geometry, float density, const Ogre::Vector3 &position = Ogre::Vector3(0,0,0), const Ogre::Quaternion &orientation = Ogre::Quaternion());
	
		/**
		Creates a new rigid dynamic actor with a specified collision shape and material and adds it to the scene.
		@return An Actor object wrapping the created PxActor.
		*/
		Actor<PxRigidDynamic> createRigidDynamic(PxGeometry &geometry, float density, PxMaterial &material, const Ogre::Vector3 &position = Ogre::Vector3(0,0,0), const Ogre::Quaternion &orientation = Ogre::Quaternion());

		/**
		Creates a new rigid static actor with a triangle mesh collision shape and adds it to the scene.
		The material bindings specified via World::registerMaterialName are used for Ogre -> PhysX material mapping.
		@return An Actor object wrapping the created PxActor.
		*/
		Actor<PxRigidStatic> createRigidStatic(Ogre::MeshPtr mesh, Cooker::Params &cookerParams = Cooker::Params(), const Ogre::Vector3 &position = Ogre::Vector3(0,0,0), const Ogre::Quaternion &orientation = Ogre::Quaternion());

		/**
		Creates a new rigid static actor with a specified collision shape and a default material and adds it to the scene.		
		@return An Actor object wrapping the created PxActor.
		*/
		Actor<PxRigidStatic> createRigidStatic(PxGeometry &geometry, const Ogre::Vector3 &position = Ogre::Vector3(0,0,0), const Ogre::Quaternion &orientation = Ogre::Quaternion());

		/**
		Creates a new rigid static actor with a specified collision shape and material and adds it to the scene.
		@return An Actor object wrapping the created PxActor.
		*/
		Actor<PxRigidStatic> createRigidStatic(PxGeometry &geometry, PxMaterial &material, const Ogre::Vector3 &position = Ogre::Vector3(0,0,0), const Ogre::Quaternion &orientation = Ogre::Quaternion());

		///Removes an actor from the scene.
		template<class T>
			void removeActor(Actor<T> &actor)
			{
				removeActor(actor.getPxActor());
				actor.setPxActor(nullptr);
			}

		///Removes an from the scene.
		void removeActor(PxActor *actor);

		/**
		Binds a PointRenderable to an actor.
		*/
		RenderedActorBinding* createRenderedActorBinding(Actor<PxRigidDynamic> &actor, PointRenderable *pointRenderable);
		RenderedActorBinding* createRenderedActorBinding(PxRigidDynamic *actor, PointRenderable *pointRenderable);

		/**
		Binds an Ogre skeleton to a set of actors.
		*/
		Ragdoll* createRagdollBinding(Ogre::Entity *entity, Ogre::SceneNode *node);

		/**
		Destroys ONLY the renderable binding, not the physx actor.
		*/
		void destroyRenderableBinding(RenderableBinding* binding);

		void setSimulationListener(SimulationListener *listener) { mSimulationListener = listener; }

		void syncRenderables();

		void simulate(float time);

		struct RaycastHit
		{
			float distance;
			Ogre::Vector3 position;
			Ogre::Vector3 normal;
			void *hitActorUserData;
		};
		/**
		These methods only provide minimalistic functionality, for full power use PhysX directly.
		@return true of the ray hit something, otherweise false.
		*/
		bool raycastClosest(const Ogre::Vector3 &origin, const Ogre::Vector3 &unitDir, float maxDistance, RaycastHit &outHit);
		bool raycastAny(const Ogre::Vector3 &origin, const Ogre::Vector3 &unitDir, float maxDistance);

		void initVisualDebugger(Ogre::SceneManager *sceneMgr, Ogre::uint32 debugGeometryVisibilityFlags, bool enabled = true);
		void setVisualDebuggerEnabled(bool enabled);

		void renderDebugGeometry();
	};

}