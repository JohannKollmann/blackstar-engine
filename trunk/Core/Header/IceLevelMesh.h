#pragma once

#include "IceIncludes.h"
#include "OgrePhysX.h"
#include "Ogre.h"


namespace Ice
{

class LevelMesh
{
private:
	OgrePhysX::Actor *mActor;
	Ogre::Entity *mEntity;
	Ogre::SceneNode *mNode;

public:
	LevelMesh(Ogre::String mesh);
	~LevelMesh(void);

	Ogre::String GetMeshFileName() { return mEntity->getMesh()->getName(); };
};

};
