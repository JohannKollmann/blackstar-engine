
#pragma once

#include "Ogre.h"
#include "IceAIState.h"
#include "NxPhysics.h"
#include "IceDirectionBlender.h"
#include "IceNavigationMesh.h"

namespace Ice
{

	class FollowPathway : public AIState
	{
	private:
		std::vector<AStarNode3D*> mPath;

		DirectionYawBlender mDirectionBlender;

		bool mAvoidingObstacle;
		Ogre::Vector3 mAvoidObstacleVector;

		NxActor *mSweepActor;
		NxSweepCache *mSweepCache;
		Ogre::String mTargetWP;
		float mRadius;

		NxActor* ObstacleCheck(Ogre::Vector3 motion);

		void computePath();
		void checkPath();

	public:
		FollowPathway(GOCAI *ai, Ogre::String target, float radius = 0.5f);
		~FollowPathway();

		void OnEnter();
		void Leave();
		void Pause();
		bool Update(float time);
	};

}