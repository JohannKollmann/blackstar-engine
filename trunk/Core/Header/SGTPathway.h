#ifndef __SGTPathway_H__
#define __SGTPathway_H__

#include "Ogre.h"

class SGTPathway
{
private:
	std::vector<Ogre::Vector3> mWPList;
	std::vector<Ogre::Vector3>::iterator mCurrentTarget;
	float mRadius;

public:
	SGTPathway(std::vector<Ogre::Vector3> mWPList, float radius);
	~SGTPathway();

	Ogre::Vector3 getTarget(Ogre::Vector3 Position);
	void smoothPath();
};

#endif