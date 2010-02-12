
#pragma once

#include "Ogre.h"
#include "IceAIState.h"
#include "NxPhysics.h"
#include "IceDirectionBlender.h"

namespace Ice
{

	class FollowPathway : public AIState
	{
	private:
		std::vector<Ogre::Vector3> mPath;
		std::vector<Ogre::Vector3>::iterator mCurrentTarget;

		DirectionYawBlender mDirectionBlender;

		bool mAvoidingObstacle;
		Ogre::Vector3 mAvoidObstacleVector;

		NxActor *mSweepActor;
		NxSweepCache *mSweepCache;
		Ogre::String mTargetWP;
		float mRadius;

		bool ObstacleCheck(Ogre::Vector3 motion);

	public:
		FollowPathway(GOCAI *ai, Ogre::String target, float radius = 1.0f);
		~FollowPathway();

		void OnEnter();
		void Leave();
		void Pause();
		bool Update(float time);
	};

}