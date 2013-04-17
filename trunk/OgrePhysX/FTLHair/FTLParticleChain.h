
#pragma once 

#include "OgrePhysXClasses.h"
#include "OgrePhysXRenderableBinding.h"
#include "Ogre.h"

namespace OgrePhysX
{
	class OgrePhysXClass ParticleChain
	{
	protected:
		struct Particle
		{
			Ogre::Vector3 force;
			Ogre::Vector3 velocity;
			Ogre::Vector3 position;
		};

		std::vector<Particle> mParticles;

		float mParticleDist;	// distance between two particles in the chain

		float mParticleMass;
		float mParticleMassInv;

		float mTimestep;
		float mTimestepAccu;

		unsigned int mStepCounter;
		Ogre::Vector3 mWindForce;

		Scene *mPhysXScene;

	public:
		/**
		@param from Position of the first (leader) particle.
		@param to Position of the last particle.
		@param numParticles Number of particles, must be at least 2.
		*/
		ParticleChain(const Ogre::Vector3 &from, const Ogre::Vector3 &to, int numParticles, Scene *scene);
		virtual ~ParticleChain() {}

		/**
		Solved the position constraints for one particle.
		@pre particleIter must have a precedessor.
		@return The correction vector.
		*/
		Ogre::Vector3 computeCollisionCorrection(const Ogre::Vector3 &position);

		void setParticleMass(float mass) { mParticleMass = mass; mParticleMassInv = 1.0f / mParticleMass; }

		void computeForces();

		/// Performs a simulation step.
		void simulate(float timeStep);

		/// Sets the position of the first particle.
		void setPosition(const Ogre::Vector3 &position);

		const std::vector<Particle>& getParticles() const { return mParticles; }

	protected:
		virtual void simulateStep() = 0;
	};

	class OgrePhysXClass FTLParticleChain : public ParticleChain
	{
	private:
		float mPBDDamping1;
		float mPBDDamping2;
	protected:
		void simulateStep();

	public:
		FTLParticleChain(const Ogre::Vector3 &from, const Ogre::Vector3 &to, int numParticles, Scene *scene) : ParticleChain(from, to, numParticles, scene) { setPBDDamping(0.0f, 0.9f); }
		~FTLParticleChain() {}
		void setPBDDamping(float pbdDamping1, float pbdDamping2 = 0.95f) { mPBDDamping1 = pbdDamping1; mPBDDamping2 = pbdDamping2; }

		Ogre::Vector3 computeCorrectionVector(const std::vector<Particle>::iterator &particleIter);
	};

	class OgrePhysXClass SpringParticleChain : public ParticleChain
	{
	private:
		float mSpringDamping;
		float mSpringStiffness;
	protected:
		void simulateStep();

	public:
		SpringParticleChain(const Ogre::Vector3 &from, const Ogre::Vector3 &to, int numParticles, Scene *scene) : ParticleChain(from, to, numParticles, scene), mSpringDamping(1.0f), mSpringStiffness(100.0f) {}
		~SpringParticleChain() {}

		void setSpringDamping(float damping) { mSpringDamping = damping; }
		void setSpringStiffness(float stiffness) { mSpringStiffness = stiffness; }
	};

	class OgrePhysXClass ParticleChainDebugVisual : public RenderableBinding
	{
	private:
		std::vector<std::pair<Ogre::SceneNode*, Ogre::Entity*> > mSpheres;
		ParticleChain *mParticleChain;
		Ogre::SceneManager *mOgreSceneMgr;

	public:
		ParticleChainDebugVisual(ParticleChain *chain, Ogre::SceneManager *ogreSceneMgr);
		~ParticleChainDebugVisual();

		const std::vector<std::pair<Ogre::SceneNode*, Ogre::Entity*> >& getNodesAndEntities() { return mSpheres; }

		void sync();
	};
}