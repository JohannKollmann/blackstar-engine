
#include "FTLParticleChain.h"
#include "OgrePhysXScene.h"
#include "OgrePhysXGeometry.h"

namespace OgrePhysX
{

#define FTL_TIMESTEP 0.01666666f

	__forceinline float rayPlaneIntersection(const Ogre::Vector3 &rayOrigin, const Ogre::Vector3 &rayDir, const Ogre::Vector3 &planeNormal, const Ogre::Vector3 &planePos)
	{
		float denom = rayDir.dotProduct(planeNormal);
		if (Ogre::Math::RealEqual(denom, 0.0f, 0.000001f)) denom = 0.000001f;
		return -(rayOrigin - planePos).dotProduct(planeNormal) / denom;
	}

	__forceinline Ogre::Vector3 projectPointOnLine(const Ogre::Vector3 &p, const Ogre::Vector3 &line1, const Ogre::Vector3 &line2)
	{
		Ogre::Vector3 rayDir = line2 - line1;
		float t = rayPlaneIntersection(line1, rayDir, rayDir, p);
		return line1 + rayDir * t;
	}

	FTLParticleChain::FTLParticleChain(const Ogre::Vector3 &from, const Ogre::Vector3 &to, int numParticles, Scene *scene)
	{
		if (numParticles < 2) numParticles = 2;
		mPhysXScene = scene;
		mParticles.resize(numParticles);
		Ogre::Vector3 dir = (to - from);
		float chainLength = dir.normalise();
		mParticleDist = chainLength / (numParticles - 1);
		Ogre::Vector3 pos = from;
		for (auto i = mParticles.begin(); i != mParticles.end(); ++i)
		{
			i->position = pos;
			pos += dir * mParticleDist;
			i->velocity = Ogre::Vector3(0, 0, 0);
			i->force = Ogre::Vector3(0, 0, 0);
		}
		mParticleMass = 0.01f;
		mParticleMassInv = 1.0f / mParticleMass;
		mPBDDamping1 = 0.0f;
		mPBDDamping2 = 0.95f;
		mPointDamping = 0.0f;
		mTimestep = 0;
		mTimestepAccu = 0.0f;
		mWindForce = Ogre::Vector3(Ogre::Math::RangeRandom(-8.0f, 8.0f), Ogre::Math::RangeRandom(-4.0f, 4.0f), Ogre::Math::RangeRandom(-8.0f, 8.0f));
	}

	Ogre::Vector3 FTLParticleChain::computeLinePenaltyForce(const std::vector<Particle>::iterator &particleIter)
	{
		Particle &prev = *(particleIter-1);
		Particle &curr = *particleIter;
		Particle &succ = *(particleIter+1);
		Ogre::Vector3 projected = projectPointOnLine(curr.position, prev.position, succ.position);
		return (projected - curr.position);
	}

	void FTLParticleChain::computeForces()
	{
		mTimestep++;
		if (mTimestep % 50 == 0)
			mWindForce = Ogre::Vector3(Ogre::Math::RangeRandom(4.0f, 8.0f), Ogre::Math::RangeRandom(-2.0f, 2.0f), Ogre::Math::RangeRandom(-2.0f, 0.0f));

		for (auto i = mParticles.begin() + 1; i != mParticles.end(); ++i)
		{
			i->force = Ogre::Vector3(0, -9.81f * mParticleMass, 0);
		}

		for (auto i = mParticles.begin() + (mParticles.size() / 2); i != mParticles.end(); ++i)
		{
			//i->force += mWindForce * mParticleMass;
		}
		for (auto i = mParticles.begin() + 1; i != mParticles.end(); ++i)
			i->force -= i->velocity * mPointDamping;

	}

	Ogre::Vector3 FTLParticleChain::computeCorrectionVector(const std::vector<Particle>::iterator &particleIter)
	{
		Particle &leader = *(particleIter-1);
		Ogre::Vector3 distVec = particleIter->position - leader.position;
		float dist = distVec.normalise();
		return distVec * (mParticleDist - dist);
	}

	Ogre::Vector3 FTLParticleChain::computePredCorrectionVector(const std::vector<Particle>::iterator &particleIter, const Ogre::Vector3 &predPosition)
	{
		Particle &leader = *(particleIter-1);
		Ogre::Vector3 distVec = predPosition - leader.position;
		float dist = distVec.normalise();
		return distVec * (mParticleDist - dist);
	}

	Ogre::Vector3 FTLParticleChain::computeCollisionCorrection(const std::vector<Particle>::iterator &particleIter)
	{
		//predictor corrector scheme
		Ogre::Vector3 predVelocity = particleIter->velocity + particleIter->force * mParticleMassInv * FTL_TIMESTEP;
		Ogre::Vector3 predPosition = particleIter->position + particleIter->velocity * FTL_TIMESTEP;
		predPosition += computePredCorrectionVector(particleIter, predPosition);

		PxShape *hit = nullptr;
		PxVec3 pxPos = Convert::toPx(predPosition);
		if (mPhysXScene->getPxScene()->overlapAny(PxSphereGeometry(0.05f), PxTransform(pxPos), hit))
		{
			PxVec3 actorCenter = hit->getActor().getWorldBounds().getCenter();
			PxVec3 rayDir = actorCenter - pxPos;
			rayDir.normalize();
			PxVec3 rayOrigin = pxPos - rayDir.multiply(PxVec3(0.2f, 0.2f, 0.2f));
			if (mPhysXScene->getPxScene()->overlapAny(PxSphereGeometry(0.05f), PxTransform(rayOrigin), hit)) return Ogre::Vector3(0, 0, 0);
			PxRaycastHit rayHit;
			if (mPhysXScene->getPxScene()->raycastSingle(rayOrigin, rayDir,  0.2f, PxSceneQueryFlag::eIMPACT|PxSceneQueryFlag::eNORMAL|PxSceneQueryFlag::eDISTANCE, rayHit))
			{
				// particleIter->position = Convert::toOgre(rayHit.impact);
				float penetrationDepth = (pxPos - rayHit.impact).normalize() * 5.0f;
				//if (penetrationDepth <= 0.01f) return Ogre::Vector3(0, 0, 0);
				//Ogre::Vector3 collisionCorrection = Convert::toOgre(rayHit.normal) * penetrationDepth * 0.5f;
				Ogre::Vector3 collisionCorrection = Convert::toOgre(-rayDir) * penetrationDepth;
				return collisionCorrection;
			}
		}
		return Ogre::Vector3(0, 0, 0);
	}

	void FTLParticleChain::simulate(float timeStep)
	{
		mTimestepAccu += timeStep;
		while (mTimestepAccu >= timeStep)
		{
			mTimestepAccu -= FTL_TIMESTEP;
			simulateStep();
		}
	}
	void FTLParticleChain::simulateStep()
	{
		computeForces();

		mParticles.front().position += mParticles.front().velocity * FTL_TIMESTEP;

		auto i = mParticles.begin()+1;
		Ogre::Vector3 oldPos(i->position);
		i->velocity += i->force * FTL_TIMESTEP * mParticleMassInv;
		i->position += i->velocity * FTL_TIMESTEP + computeCollisionCorrection(i);
		Ogre::Vector3 correctionVec = computeCorrectionVector(i);
		for (; i != mParticles.end()-1; ++i)
		{
			i->position += correctionVec;

			auto succ = i + 1;
			Ogre::Vector3 nextOldPos(succ->position);
			Ogre::Vector3 collisionPenalty = computeCollisionCorrection(succ);
			succ->velocity += succ->force * FTL_TIMESTEP * mParticleMassInv;
			succ->position += succ->velocity * FTL_TIMESTEP + collisionPenalty;
			Ogre::Vector3 nextCorrectionVec = computeCorrectionVector(succ);

			i->velocity = (i->position - oldPos - nextCorrectionVec * mPBDDamping2 - correctionVec * mPBDDamping1) / FTL_TIMESTEP;

			correctionVec = nextCorrectionVec;
			oldPos = nextOldPos;
		}

		// perform update for last particle
		i->position += correctionVec;
		i->velocity = (i->position - oldPos - correctionVec * (mPBDDamping1 + mPBDDamping2)) / FTL_TIMESTEP;
	}

	void FTLParticleChain::setPosition(const Ogre::Vector3 &position)
	{
		mParticles.front().position = position;
	}

	FTLParticleChainDebugVisual::FTLParticleChainDebugVisual(FTLParticleChain *chain, Ogre::SceneManager *ogreSceneMgr) : mParticleChain(chain), mOgreSceneMgr(ogreSceneMgr)
	{
		mSpheres.resize(chain->getParticles().size());
		auto ip = mParticleChain->getParticles().begin();
		for (auto i = mSpheres.begin(); i != mSpheres.end(); ++i)
		{
			i->second = mOgreSceneMgr->createEntity("sphere.50cm.mesh");
			i->first = mOgreSceneMgr->getRootSceneNode()->createChildSceneNode(ip->position);
			i->first->setScale(Ogre::Vector3(0.1f, 0.1f, 0.1f));
			i->first->attachObject(i->second);
			++ip;
		}
	}

	FTLParticleChainDebugVisual::~FTLParticleChainDebugVisual()
	{
		for (auto i = mSpheres.begin(); i != mSpheres.end(); ++i)
		{
			mOgreSceneMgr->destroySceneNode(i->first);
			mOgreSceneMgr->destroyEntity(i->second);
		}
	}

	void FTLParticleChainDebugVisual::sync()
	{
		auto ip = mParticleChain->getParticles().begin();
		for (auto i = mSpheres.begin(); i != mSpheres.end(); ++i)
		{
			i->first->setPosition(ip->position);
			++ip;
		}
	}
}