
#pragma once

#include "Ogre.h"
#include "SGTAIState.h"

class SGTFollowPathway : public SGTAIState
{
private:
	std::vector<Ogre::Vector3> mPath;
	std::vector<Ogre::Vector3>::iterator mCurrentTarget;
	Ogre::String mTargetWP;
	float mRadius;

	void smoothPath();

public:
	SGTFollowPathway(SGTGOCAI *ai, Ogre::String target, float radius = 3.0f);
	~SGTFollowPathway();

	void OnEnter();
	bool OnUpdate(float time);
};