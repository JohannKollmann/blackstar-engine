
#pragma once

#include "Ogre.h"
#include "SGTAIState.h"

class SGTFollowPathway : public SGTAIState
{
private:
	std::vector<Ogre::Vector3> mPath;
	std::vector<Ogre::Vector3>::iterator mCurrentTarget;
	float mRadius;

	void smoothPath();

public:
	SGTFollowPathway(Ogre::String target, float radius = 1.0f);
	~SGTFollowPathway();

	bool OnUpdate(float time);
};