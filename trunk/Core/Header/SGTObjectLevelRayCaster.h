#ifndef __SGTRayCaster_H__
#define __SGTRayCaster_H__

#include "SGTIncludes.h"
#include "Ogre.h"

class SGTDllExport SGTObjectLevelRayCaster
{
private:
	Ogre::RaySceneQueryResult mResults;
	Ogre::RaySceneQueryResult::iterator mResultsIterator;

public:
	SGTObjectLevelRayCaster(Ogre::Ray ray);

	~SGTObjectLevelRayCaster() { };

	bool IsEmpty();
	bool HasMoreHits();
	SGTGameObject* GetFirstHit(bool include_all = false);
	SGTGameObject* GetNextHit(bool include_all = false);
};

#endif
	