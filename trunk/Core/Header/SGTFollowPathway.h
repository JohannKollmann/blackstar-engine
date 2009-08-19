#ifndef __SGTFollowPathway_H__
#define __SGTFollowPathway_H__

#include "Ogre.h"
#include "SGTAIState.h"

class SGTFollowPathway : public SGTAIState
{
private:
	std::vector<Ogre::Vector3> mWPList;
	std::vector<Ogre::Vector3>::iterator mCurrentTarget;
	float mRadius;

public:
	SGTFollowPathway(Ogre::String targetWP);
	~SGTFollowPathway();

	bool OnUpdate(float time);
};

#endif