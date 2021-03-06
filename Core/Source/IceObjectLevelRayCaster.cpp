
#include "IceObjectLevelRayCaster.h"
#include "IceMain.h"
#include "IceSceneManager.h"

namespace Ice
{

ObjectLevelRayCaster::ObjectLevelRayCaster(Ogre::Ray ray)
{
	Ogre::RaySceneQuery *RaySceneQuery = Main::Instance().GetOgreSceneMgr()->createRayQuery(ray);
	RaySceneQuery->setRay(ray);
	RaySceneQuery->setSortByDistance(true);

	mResults = RaySceneQuery->execute();

}

GameObject* ObjectLevelRayCaster::GetFirstHit(bool include_all)
{
	mResultsIterator = mResults.begin();
	if (mResultsIterator != mResults.end())
	{
		//Log::Instance().LogMessage("GetFirstHit: " + (*mResultsIterator).movable->getParentNode()->getName() + " "  + (*mResultsIterator).movable->getMovableType());
		if (((*mResultsIterator).movable->getMovableType() == "Entity" || (*mResultsIterator).movable->getMovableType() == "BillboardSet")
			&& !(*mResultsIterator).movable->getUserAny().isEmpty())
		{
			GameObject *object = Ogre::any_cast<GameObject*>((*mResultsIterator).movable->getUserAny());
			if (include_all || object->IsSelectable()) return object;
			else return GetNextHit(include_all);
		}
		else return GetNextHit(include_all);
	}
	//Log::Instance().LogMessage("Warning: ObjectLevelRayCaster::GetFirstHit() - return NULL");
	return NULL;
}

GameObject* ObjectLevelRayCaster::GetNextHit(bool include_all)
{
	mResultsIterator++;
	if (mResultsIterator != mResults.end())
	{
		//Log::Instance().LogMessage("GetNextHit: " + (*mResultsIterator).movable->getParentNode()->getName() + " " + (*mResultsIterator).movable->getMovableType());
		if (((*mResultsIterator).movable->getMovableType() == "Entity" || (*mResultsIterator).movable->getMovableType() == "BillboardSet")
			&& !(*mResultsIterator).movable->getUserAny().isEmpty())
		{
			GameObject *object =  Ogre::any_cast<GameObject*>((*mResultsIterator).movable->getUserAny());
			if (include_all || object->IsSelectable()) return object;
			else return GetNextHit(include_all);
		}
		else return GetNextHit(include_all);
	}
	//Log::Instance().LogMessage("Warning: ObjectLevelRayCaster::GetNextHit() - return NULL");
	return NULL;
}

bool ObjectLevelRayCaster::HasMoreHits()
{
	if (mResultsIterator == mResults.end()) return false;
	mResultsIterator++;
	if (mResultsIterator != mResults.end())
	{
		if ((*mResultsIterator).movable->getMovableType() == "Entity" || (*mResultsIterator).movable->getMovableType() == "BillboardSet")
		{
			mResultsIterator--;
			return true;
		}
		else
		{
			bool tmp = HasMoreHits();
			mResultsIterator--;
			return tmp;
		}
	}
	mResultsIterator--;
	return false;
}

bool ObjectLevelRayCaster::IsEmpty()
{
	return ((mResults.size() == 0) ? true : false);
}

};