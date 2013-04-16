
#pragma once 

#include "OgrePhysXClasses.h"
#include "OgrePhysXRenderableBinding.h"
#include "Ogre.h"

namespace OgrePhysX
{
	class OgrePhysXClass FTLParticleChain
	{
	private:
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
		float mPBDDamping1;
		float mPBDDamping2;
		float mPointDamping;

		float mTimestepAccu;

		unsigned int mTimestep;
		Ogre::Vector3 mWindForce;

		Scene *mPhysXScene;

		void simulateStep();

	public:
		/**
		@param from Position of the first (leader) particle.
		@param to Position of the last particle.
		@param numParticles Number of particles, must be at least 2.
		*/
		FTLParticleChain(const Ogre::Vector3 &from, const Ogre::Vector3 &to, int numParticles, Scene *scene);

		/**
		Solved the position constraints for one particle.
		@pre particleIter must have a precedessor.
		@return The correction vector.
		*/
		Ogre::Vector3 computeCorrectionVector(const std::vector<Particle>::iterator &particleIter);
		Ogre::Vector3 computePredCorrectionVector(const std::vector<Particle>::iterator &particleIter, const Ogre::Vector3 &predPosition);
		Ogre::Vector3 computeCollisionCorrection(const std::vector<Particle>::iterator &particleIter);

		Ogre::Vector3 computeLinePenaltyForce(const std::vector<Particle>::iterator &particleIter);

		void setParticleMass(float mass) { mParticleMass = mass; mParticleMassInv = 1.0f / mParticleMass; }
		void setPBDDamping(float pbdDamping1, float pbdDamping2 = 0.95f) { mPBDDamping1 = pbdDamping1; mPBDDamping2 = pbdDamping2; }
		void setPointDamping(float pointDamping) { mPointDamping = pointDamping; }

		void computeForces();

		/// Performs a simulation step.
		void simulate(float timeStep);

		/// Sets the position of the first particle.
		void setPosition(const Ogre::Vector3 &position);

		const std::vector<Particle>& getParticles() const { return mParticles; }
	};

	class OgrePhysXClass FTLParticleChainDebugVisual : public RenderableBinding
	{
	private:
		std::vector<std::pair<Ogre::SceneNode*, Ogre::Entity*> > mSpheres;
		FTLParticleChain *mParticleChain;
		Ogre::SceneManager *mOgreSceneMgr;

	public:
		FTLParticleChainDebugVisual(FTLParticleChain *chain, Ogre::SceneManager *ogreSceneMgr);
		~FTLParticleChainDebugVisual();

		const std::vector<std::pair<Ogre::SceneNode*, Ogre::Entity*> >& getNodesAndEntities() { return mSpheres; }

		void sync();
	};
}