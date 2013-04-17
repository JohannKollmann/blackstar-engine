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

#pragma once

#include "OgrePhysXClasses.h"
#include "OgrePhysXRenderableBinding.h"
#include "OgrePhysXRenderedActorBinding.h"
#include "OgrePhysXActor.h"
#include "OgrePhysXCooker.h"
#include "PxPhysicsAPI.h"
#include "../FTLHair/FTLParticleChain.h"

namespace OgrePhysX
{

	class OgrePhysXClass Scene
	{
		friend class World;

	public:
		class SimulationListener
		{
		public:
			/**
			Called right before PxScene::simulate.
			*/
			virtual void onBeginSimulate(float time) {}

			/**
			Called right after PxScene::simulate.
			That means that it is executed while the simulation is in progress.
			*/
			virtual void onSimulate(float time) {}

			/**
			Called right after PxScene::fetchResults.
			*/
			virtual void onEndSimulate(float time) {}
		};

	private:
		PxScene *mPxScene;
		Ogre::SceneManager *mOgreSceneMgr;

		float mTimeAccu;
		float mFrameTime;

		std::vector<RenderableBinding*> mOgrePhysXBindings;

		std::vector<ParticleChain*> mParticleChains;

		SimulationListener *mSimulationListener;

		void create(PxSceneDesc &desc);

		Scene(Ogre::SceneManager *ogreSceneMgr);
		Scene(Ogre::SceneManager *ogreSceneMgr, PxSceneDesc &desc);
		~Scene();

	public:

		PxScene* getPxScene();

		/**
		Creates a new rigid dynamic actor with a specified collision shape and a default material and adds it to the scene.
		@return An Actor object wrapping the created PxActor.
		*/
		Actor<PxRigidDynamic> createRigidDynamic(PxGeometry &geometry, float density, const PxTransform &shapePose = PxTransform::createIdentity(), const PxTransform &actorPose = PxTransform::createIdentity());

		/**
		Creates a new rigid dynamic actor with a specified collision shape and material and adds it to the scene.
		@return An Actor object wrapping the created PxActor.
		*/
		Actor<PxRigidDynamic> createRigidDynamic(PxGeometry &geometry, float density, PxMaterial &material, const PxTransform &shapePose = PxTransform::createIdentity(), const PxTransform &actorPose = PxTransform::createIdentity());

		/**
		Creates a new rigid dynamic actor with a collision shape matching the given entity.
		@return An Actor object wrapping the created PxActor.
		*/
		Actor<PxRigidDynamic> createRigidDynamic(Ogre::Entity *entity, float density, const Ogre::Vector3 &scale = Ogre::Vector3(1,1,1), const PxTransform &actorPose = PxTransform::createIdentity());

		/**
		Creates a new rigid dynamic actor with a collision shape matching the given entity.
		@return An Actor object wrapping the created PxActor.
		*/
		Actor<PxRigidDynamic> createRigidDynamic(Ogre::Entity *entity, float density, PxMaterial &material, const Ogre::Vector3 &scale = Ogre::Vector3(1,1,1), const PxTransform &actorPose = PxTransform::createIdentity());

		/**
		Creates a new rigid static actor with a triangle mesh collision shape and adds it to the scene.
		The material bindings specified via World::registerMaterialName are used for Ogre -> PhysX material mapping.
		@return An Actor object wrapping the created PxActor.
		*/
		Actor<PxRigidStatic> createRigidStatic(Ogre::Entity *entity, Cooker::Params &cookerParams = Cooker::Params(), const PxTransform &actorPose = PxTransform::createIdentity());

		/**
		Creates a new rigid static actor with a specified collision shape and a default material and adds it to the scene.		
		@return An Actor object wrapping the created PxActor.
		*/
		Actor<PxRigidStatic> createRigidStatic(PxGeometry &geometry, const PxTransform &shapePose = PxTransform::createIdentity(), const PxTransform &actorPose = PxTransform::createIdentity());

		/**
		Creates a new rigid static actor with a specified collision shape and material and adds it to the scene.
		@return An Actor object wrapping the created PxActor.
		*/
		Actor<PxRigidStatic> createRigidStatic(PxGeometry &geometry, PxMaterial &material, const PxTransform &shapePose = PxTransform::createIdentity(), const PxTransform &actorPose = PxTransform::createIdentity());

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
		Creates a destructible dynamic rendered actor.
		*/
		Destructible* createDestructible(const Ogre::String &meshSplitConfigFile, float breakForce = 1000.0f, float breakTorque = 1000.0f, float density = 100.0f, const Ogre::Vector3 &scale = Ogre::Vector3(1,1,1));
		Destructible* createDestructible(const Ogre::String &meshSplitConfigFile, PxMaterial &material, float breakForce = 1000.0f, float breakTorque = 1000.0f, float density = 100.0f, const Ogre::Vector3 &scale = Ogre::Vector3(1,1,1));

		/**
		Binds an Ogre skeleton to a set of actors.
		-- not working properly at the moment :-(
		*/
		Ragdoll* createRagdollBinding(Ogre::Entity *entity, Ogre::SceneNode *node);

		ParticleChain* createFTLParticleChain(const Ogre::Vector3 &from, const Ogre::Vector3 &to, int numParticles)
		{
			ParticleChain *chain = new FTLParticleChain(from, to, numParticles, this);
			mParticleChains.push_back(chain);
			return chain;
		}

		ParticleChain* createSpringParticleChain(const Ogre::Vector3 &from, const Ogre::Vector3 &to, int numParticles)
		{
			ParticleChain *chain = new SpringParticleChain(from, to, numParticles, this);
			mParticleChains.push_back(chain);
			return chain;
		}

		ParticleChainDebugVisual* createParticleChainDebugVisual(ParticleChain *particleChain)
		{
			ParticleChainDebugVisual *visual = new ParticleChainDebugVisual(particleChain, mOgreSceneMgr);
			mOgrePhysXBindings.push_back(visual);
			return visual;
		}

		void destroyParticleChain(ParticleChain *chain)
		{
			for (auto i = mParticleChains.begin(); i != mParticleChains.end(); ++i)
			{
				if ((*i) == chain)
				{
					mParticleChains.erase(i);
					break;
				}
			}
			delete chain;
		}

		/**
		Destroys ONLY the renderable binding, not the physx actor.
		*/
		void destroyRenderableBinding(RenderableBinding* binding);

		void setSimulationListener(SimulationListener *listener) { mSimulationListener = listener; }

		void syncRenderables();

		void simulate(float time);

		Ogre::SceneManager* getOgreSceneMgr() { return mOgreSceneMgr; }

		struct RaycastHit
		{
			float distance;
			Ogre::Vector3 position;
			Ogre::Vector3 normal;
			void *hitActorUserData;
		};
		/**
		These methods only provide limited functionality, for full power use PhysX directly.
		@return true of the ray hit something, otherweise false.
		*/
		bool raycastClosest(const Ogre::Vector3 &origin, const Ogre::Vector3 &unitDir, float maxDistance, PxSceneQueryFilterData &queryFilterData = PxSceneQueryFilterData());
		bool raycastAny(const Ogre::Vector3 &origin, const Ogre::Vector3 &unitDir, float maxDistance, PxSceneQueryFilterData &queryFilterData = PxSceneQueryFilterData());

		bool raycastClosest(const Ogre::Vector3 &origin, const Ogre::Vector3 &unitDir, float maxDistance, RaycastHit &outHit, PxSceneQueryFilterData &queryFilterData = PxSceneQueryFilterData());
		bool raycastAny(const Ogre::Vector3 &origin, const Ogre::Vector3 &unitDir, float maxDistance, RaycastHit &outHit, PxSceneQueryFilterData &queryFilterData = PxSceneQueryFilterData());
	};

}