
#pragma once

#include "Ogre.h"
#include "SGTAIState.h"
#include "NxPhysics.h"

class SGTFollowPathway : public SGTAIState
{
private:
	std::vector<Ogre::Vector3> mPath;
	std::vector<Ogre::Vector3>::iterator mCurrentTarget;

	Ogre::Vector3 mBlendDirection;
	float mTargetBlendYaw;
	float mBlendFactor;

	void StartBlend(Ogre::Vector3 oldDir, Ogre::Vector3 newDir);

	NxActor *mSweepActor;
	NxSweepCache *mSweepCache;
	Ogre::String mTargetWP;
	float mRadius;

	bool ObstacleCheck(Ogre::Vector3 motion);

public:
	SGTFollowPathway(SGTGOCAI *ai, Ogre::String target, float radius = 1.0f);
	~SGTFollowPathway();

	void OnEnter();
	bool OnUpdate(float time);
};