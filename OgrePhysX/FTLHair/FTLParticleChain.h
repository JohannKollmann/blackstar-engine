
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

		struct CollisionConstraint
		{
			Ogre::Vector3 closestSurfacePoint;
			Ogre::Vector3 normal;
		};

		std::vector<Particle> mParticles;

		float mParticleDist;	// distance between two particles in the chain

		float mParticleMass;
		float mParticleMassInv;

		float mChainStiffness;

		float mPBDPointDamping;
		float mPointDamping;

		float mFriction;

		float mTimestep;
		float mTimestepAccu;

		unsigned int mStepCounter;
		Ogre::Vector3 mWindForce;

		Scene *mPhysXScene;

		virtual float getParticleMass(const std::vector<Particle>::iterator &particleIter) { return mParticleMass; }

		/**
		Solved the position constraints for one particle.
		@pre particleIter must have a precedessor.
		@return The correction vector.
		*/
		Ogre::Vector3 computeFTLCorrectionVector(const std::vector<Particle>::iterator &particleIter);

		bool checkPenetration(const Ogre::Vector3 &position, Ogre::Vector3 &closestSurfacePos, Ogre::Vector3 &collisionNormal);

		Ogre::Vector3 computeCollisionCorrection(const Ogre::Vector3 &position, const CollisionConstraint &cc);

		void addExternalForces();
		void addChainStiffnessForces(const std::vector<Particle> &predictions, std::vector<Particle> &outParticles);

		Ogre::Vector3 computeFrictionDamping(const Ogre::Vector3 &normal, const Ogre::Vector3 &velocity);

	public:
		/**
		@param from Position of the first (leader) particle.
		@param to Position of the last particle.
		@param numParticles Number of particles, must be at least 2.
		*/
		ParticleChain(const Ogre::Vector3 &from, const Ogre::Vector3 &to, int numParticles, Scene *scene);
		virtual ~ParticleChain() {}

		void setParticleMass(float mass) { mParticleMass = mass; mParticleMassInv = 1.0f / mParticleMass; }

		void setChainStiffness(float chainStiffness) { mChainStiffness = chainStiffness; }

		void setPointDamping(float pointDamping) { mPointDamping = pointDamping; }

		void setPBDPointDamping(float pbdPointDamping) { mPBDPointDamping = pbdPointDamping; }

		void setFriction(float friction) { mFriction = friction; }

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
		float mFTLDamping;

	protected:
		void simulateStep();

	public:
		FTLParticleChain(const Ogre::Vector3 &from, const Ogre::Vector3 &to, int numParticles, Scene *scene) : ParticleChain(from, to, numParticles, scene), mFTLDamping(0.9f) {}
		~FTLParticleChain() {}
		void setFTLDamping(float ftlDamping) {  mFTLDamping = ftlDamping; }
	};

	class OgrePhysXClass PBDParticleChain : public ParticleChain
	{
	private:
		int mIterationCount;
		float mOverRelaxation;

	protected:
		void simulateStep();

	public:
		PBDParticleChain(const Ogre::Vector3 &from, const Ogre::Vector3 &to, int numParticles, Scene *scene) : ParticleChain(from, to, numParticles, scene), mIterationCount(4) {}
		~PBDParticleChain() {}

		void setIterationCount(int numIterations) { mIterationCount = numIterations; }
		void setOverRelaxation(float overRelaxation) { mOverRelaxation = overRelaxation; }
	};

	class OgrePhysXClass SpringParticleChain : public ParticleChain
	{
	private:
		float mSpringDamping;
		float mSpringStiffness;

	protected:
		float getParticleMass(const std::vector<Particle>::iterator &particleIter);
		void simulateStep();

	public:
		SpringParticleChain(const Ogre::Vector3 &from, const Ogre::Vector3 &to, int numParticles, Scene *scene) : ParticleChain(from, to, numParticles, scene), mSpringDamping(0.5f), mSpringStiffness(100.0f) {}
		~SpringParticleChain() {}

		void setSpringDamping(float damping) { mSpringDamping = damping; }
		void setSpringStiffness(float stiffness) { mSpringStiffness = stiffness; }

		void performDistanceProjection(Particle &particle1, Particle &particle2);
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