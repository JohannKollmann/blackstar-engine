#pragma once

#include "SGTIncludes.h"
#include "SGTMain.h"

class SGTLevelMesh
{
private:
	NxOgre::Body* mBody;
	Ogre::String mMeshFileName;

public:
	SGTLevelMesh(Ogre::String mesh);
	~SGTLevelMesh(void);

	Ogre::String GetMeshFileName() { return mMeshFileName; };
};
