#pragma once

#include "SGTIncludes.h"
#include "OgrePhysX.h"
#include "Ogre.h"

class SGTLevelMesh
{
private:
	OgrePhysX::Actor *mActor;
	Ogre::Entity *mEntity;
	Ogre::SceneNode *mNode;

public:
	SGTLevelMesh(Ogre::String mesh);
	~SGTLevelMesh(void);

	Ogre::String GetMeshFileName() { return mEntity->getMesh()->getName(); };
};
