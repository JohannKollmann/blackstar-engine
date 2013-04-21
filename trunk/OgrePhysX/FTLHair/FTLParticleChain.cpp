
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
		mChainStiffness = 0.0f;
		mPointDamping = 0.0f;
		mPBDPointDamping = 0.0f;
		mWindForce = Ogre::Vector3(Ogre::Math::RangeRandom(-8.0f, 8.0f), Ogre::Math::RangeRandom(-4.0f, 4.0f), Ogre::Math::RangeRandom(-8.0f, 8.0f));
	}

	void ParticleChain::addExternalForces()
	{
		mStepCounter++;
		if (mStepCounter % 50 == 0)
			mWindForce = Ogre::Vector3(Ogre::Math::RangeRandom(4.0f, 8.0f), Ogre::Math::RangeRandom(-2.0f, 2.0f), Ogre::Math::RangeRandom(-2.0f, 0.0f));

		for (auto i = mParticles.begin() + 1; i != mParticles.end(); ++i)
		{
			i->force = Ogre::Vector3(0, -9.81f * getParticleMass(i), 0);
		}

		for (auto i = mParticles.begin() + (mParticles.size() / 2); i != mParticles.end(); ++i)
		{
			//i->force += mWindForce * mParticleMass;
		}
	}

	void ParticleChain::addChainStiffnessForces(const std::vector<Particle> &predictions, std::vector<Particle> &outParticles)
	{
		auto ip = predictions.begin() + 1;
		for (auto i = outParticles.begin() + 1; i != outParticles.end() - 1; ++i)
		{
			Ogre::Vector3 target = projectPointOnLine(ip->position, (ip-1)->position, (ip+1)->position);
			i->force += (target - ip->position) * mChainStiffness;
			++ip;
		}
	}

	bool ParticleChain::checkPenetration(const Ogre::Vector3 &position, Ogre::Vector3 &closestSurfacePos, Ogre::Vector3 &collisionNormal)
	{
		PxShape *hit = nullptr;
		PxVec3 pxPos = Convert::toPx(position);
		if (mPhysXScene->getPxScene()->overlapAny(PxSphereGeometry(0.05f), PxTransform(pxPos), hit))
		{
			PxVec3 actorCenter = hit->getActor().getWorldBounds().getCenter();
			PxVec3 rayDir = actorCenter - pxPos;
			rayDir.normalize();
			PxVec3 rayOrigin = pxPos - rayDir.multiply(PxVec3(0.2f, 0.2f, 0.2f));
			if (mPhysXScene->getPxScene()->overlapAny(PxSphereGeometry(0.05f), PxTransform(rayOrigin), hit)) return false;
			PxRaycastHit rayHit;
			if (mPhysXScene->getPxScene()->raycastSingle(rayOrigin, rayDir,  0.2f, PxSceneQueryFlag::eIMPACT|PxSceneQueryFlag::eNORMAL|PxSceneQueryFlag::eDISTANCE, rayHit))
			{
				closestSurfacePos = Convert::toOgre(rayHit.impact);
				collisionNormal = Convert::toOgre(rayHit.normal);
				return true;
			}
		}
		return false;
	}

	Ogre::Vector3 ParticleChain::computeCollisionCorrection(const Ogre::Vector3 &position)
	{
		Ogre::Vector3 closestSurfacePos, collisionNormal;
		if (checkPenetration(position, closestSurfacePos, collisionNormal))
		{
			float penetrationDepth = (position - closestSurfacePos).normalise();
			Ogre::Vector3 collisionCorrection = collisionNormal * penetrationDepth;
			return collisionCorrection;
		}
		return Ogre::Vector3(0, 0, 0);
	}

	void ParticleChain::setPosition(const Ogre::Vector3 &position)
	{
		mParticles.front().velocity = (position - mParticles.front().position) / (0.1f);
		mParticles.front().position = position;
	}

	void ParticleChain::simulate(float timeStep)
	{
		mTimestepAccu += timeStep;
		while (mTimestepAccu >= timeStep)
		{
			mTimestepAccu -= mTimestep;
			addExternalForces();
			simulateStep();
		}
	}

	Ogre::Vector3 ParticleChain::computeFTLCorrectionVector(const std::vector<Particle>::iterator &particleIter)
	{
		Particle &leader = *(particleIter-1);
		Ogre::Vector3 distVec = particleIter->position - leader.position;
		float dist = distVec.normalise();
		return distVec * (mParticleDist - dist);
	}

	/*****************************************************************************************************************************
	************************************************************ FTL *************************************************************
	******************************************************************************************************************************/

	void FTLParticleChain::simulateStep()
	{
		auto i = mParticles.begin()+1;
		Ogre::Vector3 oldPos(i->position);
		i->velocity += i->force * mTimestep * mParticleMassInv;
		i->position += i->velocity * mTimestep;
		i->position += computeCollisionCorrection(i->position + computeFTLCorrectionVector(i));
		Ogre::Vector3 correctionVec = computeFTLCorrectionVector(i);
		for (; i != mParticles.end()-1; ++i)
		{
			i->position += correctionVec;

			auto succ = i + 1;
			Ogre::Vector3 nextOldPos(succ->position);
			succ->velocity += succ->force * mTimestep * mParticleMassInv;
			succ->position += succ->velocity * mTimestep;
			succ->position += computeCollisionCorrection(succ->position + computeFTLCorrectionVector(succ));
			Ogre::Vector3 nextCorrectionVec = computeFTLCorrectionVector(succ);

			i->velocity = (i->position - oldPos - nextCorrectionVec *  mFTLDamping - correctionVec * mPBDPointDamping) / mTimestep;

			correctionVec = nextCorrectionVec;
			oldPos = nextOldPos;
		}

		// perform update for last particle
		i->position += correctionVec;
		i->velocity = (i->position - oldPos - correctionVec * (mPBDPointDamping + mFTLDamping)) / mTimestep;
	}

	/*****************************************************************************************************************************
	************************************************************ PBD *************************************************************
	******************************************************************************************************************************/

	// this implementation follows closely the method proposed here: http://www.matthiasmueller.info/publications/posBasedDyn.pdf
	void PBDParticleChain::simulateStep()
	{
		std::vector<Particle> futureParticles = mParticles;
		for (auto i = futureParticles.begin() + 1; i != futureParticles.end(); ++i)
		{
			i->velocity += i->force * mParticleMassInv * mTimestep;
			i->velocity -= i->velocity * mPointDamping * mParticleMassInv * mTimestep;	// point damping
			i->position += i->velocity * mTimestep;
		}

		// generate collision constraints
		std::vector<std::pair<Particle*, CollisionConstraint> > collisionConstraints;
		for (auto i = futureParticles.begin() + 1; i != futureParticles.end(); ++i)
		{
			CollisionConstraint cc;
			if (checkPenetration(i->position, cc.closestSurfacePoint, cc.normal))
				collisionConstraints.push_back(std::make_pair(&(*i), cc));
		}

		// hack: use force component of Particle class to store correction vector for damping later
		for (auto i = futureParticles.begin(); i != futureParticles.end(); ++i)
			i->force = Ogre::Vector3(0, 0, 0);

		int numIterations = mIterationCount;	//std::min<int>(mIterationCount + collisionConstraints.size(), 10);
		for (int iteration = 0; iteration < numIterations; iteration++)
		{
			// collision constraints
			for (auto ic = collisionConstraints.begin(); ic != collisionConstraints.end(); ++ic)
			{
				float penalty = (ic->second.closestSurfacePoint - ic->first->position).dotProduct(ic->second.normal);
				if (penalty > 0)
				{
					ic->first->position += penalty * ic->second.normal;
					ic->first->force += penalty * ic->second.normal;
				}
			}

			auto i = futureParticles.begin() + 1;
			Ogre::Vector3 correction = computeFTLCorrectionVector(i) * mOverRelaxation;
			i->force += correction;
			i->position += correction;

			for (i = futureParticles.begin() + 2; i != futureParticles.end(); ++i)
			{
				correction = computeFTLCorrectionVector(i) * 0.5f * mOverRelaxation;
				(i-1)->position -= correction;
				i->position += correction;
				(i-1)->force -= correction;
				i->force += correction;
			}

			// chain stiffness
			for (i = futureParticles.begin() + 1; i != futureParticles.end() - 1; ++i)
			{
				Ogre::Vector3 target = projectPointOnLine(i->position, (i-1)->position, (i+1)->position);
				Ogre::Vector3 correction = (target - i->position) * mChainStiffness;
				i->position += correction;
				i->force += correction;
			}
		}

		auto ifu = futureParticles.begin() + 1;
		for (auto i = mParticles.begin() + 1; i != mParticles.end(); ++i)
		{
			i->velocity = (ifu->position - i->position - ifu->force * mPBDPointDamping) / mTimestep;
			i->position = ifu->position;
			++ifu;
		}

		// finally solve collision constraints one more but without updating velocity
		for (auto ic = collisionConstraints.begin(); ic != collisionConstraints.end(); ++ic)
		{
			float penalty = (ic->second.closestSurfacePoint - ic->first->position).dotProduct(ic->second.normal);
			if (penalty > 0)
				ic->first->position += penalty * ic->second.normal;
		}
		ifu = futureParticles.begin() + 1;
		for (auto i = mParticles.begin() + 1; i != mParticles.end(); ++i)
		{
			i->position = ifu->position;
			++ifu;
		}
	}

	/*****************************************************************************************************************************
	********************************************************** Springs ***********************************************************
	******************************************************************************************************************************/
	float SpringParticleChain::getParticleMass(const std::vector<Particle>::iterator &particleIter)
	{
		return mParticleMass + std::distance(particleIter, mParticles.end()) * mParticleMass * 0.2f;
	}

	void SpringParticleChain::simulateStep()
	{
		std::vector<Particle> predictedParticles = mParticles;
		std::vector<Ogre::Vector3> postDampings;
		postDampings.resize(mParticles.size());

		// first pass: predict positions and velocities without damping
		for (auto ip = predictedParticles.begin() + 1; ip != predictedParticles.end(); ++ip)
		{
			Ogre::Vector3 distVec = ip->position - (ip-1)->position;
			float dist = distVec.normalise();
			Ogre::Vector3 springForce = distVec * (mParticleDist - dist) * mSpringStiffness;
			ip->force += springForce;
			(ip-1)->force -= springForce;
		}
		auto i = mParticles.begin() + 1;
		for (auto ip = predictedParticles.begin() + 1; ip != predictedParticles.end(); ++ip)
		{
			i->force = ip->force;
			ip->velocity += (ip->force / getParticleMass(i)) * mTimestep;
			ip->position += ip->velocity * mTimestep;
			++i;
		}

		// second pass: add damping forces using the predictions
		auto ip = predictedParticles.begin() + 1;
		for (auto i = mParticles.begin() + 1; i != mParticles.end(); ++i)
		{
			Ogre::Vector3 distVec = ip->position - (ip-1)->position;
			float dist = distVec.normalise();
			float projectedDamping = ip->velocity.dotProduct(distVec) - (ip-1)->velocity.dotProduct(distVec);
			Ogre::Vector3 dampingForce = projectedDamping * mSpringDamping * distVec;
			i->force -= dampingForce;
			(i-1)->force += dampingForce;

			++ip;
		}

		addChainStiffnessForces(predictedParticles, mParticles);

		// point damping
		ip = predictedParticles.begin() + 1;
		for (auto i = mParticles.begin() + 1; i != mParticles.end(); ++i)
		{
			i->force -= ip->velocity * mPointDamping;
			++ip;
		}

		for (auto i = mParticles.begin() + 1; i != mParticles.end(); ++i)
		{
			i->velocity += (i->force / getParticleMass(i)) * mTimestep;
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