#pragma once

#include "IceIncludes.h"
#include "Ogre.h"

namespace Ice
{

class DllExport ObjectLevelRayCaster
{
private:
	Ogre::RaySceneQueryResult mResults;
	Ogre::RaySceneQueryResult::iterator mResultsIterator;

public:
	ObjectLevelRayCaster(Ogre::Ray ray);

	~ObjectLevelRayCaster() { };

	bool IsEmpty();
	bool HasMoreHits();
	GameObject* GetFirstHit(bool include_all = false);
	GameObject* GetNextHit(bool include_all = false);
};

};