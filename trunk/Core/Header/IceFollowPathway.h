
#pragma once

#include "Ogre.h"
#include "IceAIProcess.h"
#include "NxPhysics.h"
#include "IceDirectionBlender.h"
#include "IceNavigationMesh.h"

namespace Ice
{

	class FollowPathway : public AIProcess
	{
	private:
		std::vector<AStarNode3D*> mPath;

		AStarNode3D mTempObstacleAvoidNode;

		DirectionYawBlender mDirectionBlender;

		bool mAvoidingObstacle;
		Ogre::Vector3 mAvoidObstacleVector;

		NxActor *mSweepActor;
		NxSweepCache *mSweepCache;
		Ogre::String mTargetWP;
		float mRadius;

		NxActor* ObstacleCheck(Ogre::Vector3 motion);

		void computePath();
		void verifyPath();
		void optimizePath();

	public:
		FollowPathway(GOCAI *ai, Ogre::String target, float radius = 0.5f);
		~FollowPathway();

	protected:
		void OnSetActive(bool active);
		void OnReceiveMessage(Msg &msg);
		void Update(float time);
	};

}