
#pragma once

#include "Ogre.h"
#include "IceAIProcess.h"
#include "IceDirectionBlender.h"
#include "IceNavigationMesh.h"

namespace Ice
{

	class FollowPathwayProcess : public AIProcess
	{
	private:
		std::vector<AStarNode3D*> mPath;

		AStarNode3D mTempObstacleAvoidNode;

		DirectionYawBlender mDirectionBlender;

		PxSweepCache *mSweepCache;

		bool mAvoidingObstacle;
		Ogre::Vector3 mAvoidObstacleVector;

		Ogre::Vector3 mTargetPosition;
		float mRadius;

		PxRigidDynamic* ObstacleCheck(Ogre::Vector3 motion);

		void computePath();
		void verifyPath();
		void optimizePath();

	public:
		FollowPathwayProcess(std::shared_ptr<GOCAI> ai, const Ogre::Vector3 &target, float radius = 0.5f);
		~FollowPathwayProcess();

	protected:
		void OnSetActive(bool active);
		void ReceiveMessage(Msg &msg);
		void Update(float time);
	};

}