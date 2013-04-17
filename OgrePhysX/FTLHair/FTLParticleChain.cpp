
#include "FTLParticleChain.h"
#include "OgrePhysXScene.h"
#include "OgrePhysXGeometry.h"

namespace OgrePhysX
{
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

	ParticleChain::ParticleChain(const Ogre::Vector3 &from, const Ogre::Vector3 &to, int numParticles, Scene *scene)
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
		mStepCounter = 0;
		mTimestep = 0.01f;
		mTimestepAccu = 0.0f;
		mWindForce = Ogre::Vector3(Ogre::Math::RangeRandom(-8.0f, 8.0f), Ogre::Math::RangeRandom(-4.0f, 4.0f), Ogre::Math::RangeRandom(-8.0f, 8.0f));
	}

	void ParticleChain::computeForces()
	{
		mStepCounter++;
		if (mStepCounter % 50 == 0)
			mWindForce = Ogre::Vector3(Ogre::Math::RangeRandom(4.0f, 8.0f), Ogre::Math::RangeRandom(-2.0f, 2.0f), Ogre::Math::RangeRandom(-2.0f, 0.0f));

		for (auto i = mParticles.begin() + 1; i != mParticles.end(); ++i)
		{
			i->force = Ogre::Vector3(0, -9.81f * mParticleMass, 0);
		}

		for (auto i = mParticles.begin() + (mParticles.size() / 2); i != mParticles.end(); ++i)
		{
			//i->force += mWindForce * mParticleMass;
		}

	}

	Ogre::Vector3 ParticleChain::computeCollisionCorrection(const Ogre::Vector3 &position)
	{
		PxShape *hit = nullptr;
		PxVec3 pxPos = Convert::toPx(position);
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
				float penetrationDepth = (pxPos - rayHit.impact).normalize() * 5.0f;
				//if (penetrationDepth <= 0.01f) return Ogre::Vector3(0, 0, 0);
				//Ogre::Vector3 collisionCorrection = Convert::toOgre(rayHit.normal) * penetrationDepth;
				Ogre::Vector3 collisionCorrection = Convert::toOgre(-rayDir) * penetrationDepth;
				return collisionCorrection;
			}
		}
		return Ogre::Vector3(0, 0, 0);
	}

	void ParticleChain::setPosition(const Ogre::Vector3 &position)
	{
		mParticles.front().position = position;
	}

	void ParticleChain::simulate(float timeStep)
	{
		mTimestepAccu += timeStep;
		while (mTimestepAccu >= timeStep)
		{
			mTimestepAccu -= mTimestep;
			computeForces();
			simulateStep();
		}
	}


	/*****************************************************************************************************************************
	************************************************************ FTL *************************************************************
	******************************************************************************************************************************/

	Ogre::Vector3 FTLParticleChain::computeCorrectionVector(const std::vector<Particle>::iterator &particleIter)
	{
		Particle &leader = *(particleIter-1);
		Ogre::Vector3 distVec = particleIter->position - leader.position;
		float dist = distVec.normalise();
		return distVec * (mParticleDist - dist);
	}

	void FTLParticleChain::simulateStep()
	{
		mParticles.front().position += mParticles.front().velocity * mTimestep;

		auto i = mParticles.begin()+1;
		Ogre::Vector3 oldPos(i->position);
		i->velocity += i->force * mTimestep * mParticleMassInv;
		i->position += i->velocity * mTimestep;
		i->position += computeCollisionCorrection(i->position + computeCorrectionVector(i));
		Ogre::Vector3 correctionVec = computeCorrectionVector(i);
		for (; i != mParticles.end()-1; ++i)
		{
			i->position += correctionVec;

			auto succ = i + 1;
			Ogre::Vector3 nextOldPos(succ->position);
			succ->velocity += succ->force * mTimestep * mParticleMassInv;
			succ->position += succ->velocity * mTimestep;
			succ->position += computeCollisionCorrection(succ->position + computeCorrectionVector(succ));
			Ogre::Vector3 nextCorrectionVec = computeCorrectionVector(succ);

			i->velocity = (i->position - oldPos - nextCorrectionVec * mPBDDamping2 - correctionVec * mPBDDamping1) / mTimestep;

			correctionVec = nextCorrectionVec;
			oldPos = nextOldPos;
		}

		// perform update for last particle
		i->position += correctionVec;
		i->velocity = (i->position - oldPos - correctionVec * (mPBDDamping1 + mPBDDamping2)) / mTimestep;
	}

	/*****************************************************************************************************************************
	********************************************************** Springs ***********************************************************
	******************************************************************************************************************************/

	void SpringParticleChain::simulateStep()
	{
		std::vector<Particle> predictedParticles = mParticles;

		// first pass: predict positions and velocities without damping
		for (auto ip = predictedParticles.begin() + 1; ip != predictedParticles.end(); ++ip)
		{
			auto pred = ip - 1;
			Ogre::Vector3 distVec = ip->position - pred->position;
			float dist = distVec.normalise();
			Ogre::Vector3 springForce = distVec * (mParticleDist - dist) * mSpringStiffness;
			ip->force += springForce;
			pred->force -= springForce;
		}
		auto i = mParticles.begin() + 1;
		for (auto ip = predictedParticles.begin() + 1; ip != predictedParticles.end(); ++ip)
		{
			i->force = ip->force;
			ip->velocity += (ip->force / mParticleMass) * mTimestep;
			ip->position += ip->velocity * mTimestep;
			++i;
		}

		// second pass: add damping forces using the predictions
		auto ip = predictedParticles.begin() + 1;
		for (auto i = mParticles.begin() + 1; i != mParticles.end(); ++i)
		{
			auto pred = ip - 1;
			Ogre::Vector3 distVec = ip->position - pred->position;
			float dist = distVec.normalise();

			float projectedDamping = ip->velocity.dotProduct(distVec) - pred->velocity.dotProduct(distVec);
			i->force -= projectedDamping * mSpringDamping * distVec;
			pred->force += projectedDamping * mSpringDamping * distVec;

			++ip;
		}
		for (auto i = mParticles.begin() + 1; i != mParticles.end(); ++i)
		{
			i->velocity += (i->force / mParticleMass) * mTimestep;
			i->position += i->velocity * mTimestep;
		}

	}

	ParticleChainDebugVisual::ParticleChainDebugVisual(ParticleChain *chain, Ogre::SceneManager *ogreSceneMgr) : mParticleChain(chain), mOgreSceneMgr(ogreSceneMgr)
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

	ParticleChainDebugVisual::~ParticleChainDebugVisual()
	{
		for (auto i = mSpheres.begin(); i != mSpheres.end(); ++i)
		{
			mOgreSceneMgr->destroySceneNode(i->first);
			mOgreSceneMgr->destroyEntity(i->second);
		}
	}

	void ParticleChainDebugVisual::sync()
	{
		auto ip = mParticleChain->getParticles().begin();
		for (auto i = mSpheres.begin(); i != mSpheres.end(); ++i)
		{
			i->first->setPosition(ip->position);
			++ip;
		}
	}
}