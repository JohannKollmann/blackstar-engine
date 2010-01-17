
#pragma once

#include "Ogre.h"
#include "IceAIState.h"
#include "NxPhysics.h"

namespace Ice
{

class FollowPathway : public AIState
{
private:
	std::vector<Ogre::Vector3> mPath;
	std::vector<Ogre::Vector3>::iterator mCurrentTarget;

	Ogre::Vector3 mBlendDirection;
	float mTargetBlendYaw;
	float mBlendFactor;

	bool mAvoidingObstacle;
	Ogre::Vector3 mAvoidObstacleVector;

	void StartBlend(Ogre::Vector3 oldDir, Ogre::Vector3 newDir);

	NxActor *mSweepActor;
	NxSweepCache *mSweepCache;
	Ogre::String mTargetWP;
	float mRadius;

	bool ObstacleCheck(Ogre::Vector3 motion);

public:
	FollowPathway(GOCAI *ai, Ogre::String target, float radius = 1.0f);
	~FollowPathway();

	void OnEnter();
	bool OnUpdate(float time);
};

};