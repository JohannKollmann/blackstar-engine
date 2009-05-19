
#include "SGTObjectLevelRayCaster.h"
#include "SGTMain.h"
#include "SGTSceneManager.h"


SGTObjectLevelRayCaster::SGTObjectLevelRayCaster(Ogre::Ray ray)
{
	Ogre::RaySceneQuery *RaySceneQuery = SGTMain::Instance().GetOgreSceneMgr()->createRayQuery(ray);
	RaySceneQuery->setRay(ray);
	RaySceneQuery->setSortByDistance(true);

	mResults = RaySceneQuery->execute();

}

SGTGameObject* SGTObjectLevelRayCaster::GetFirstHit()
{
	mResultsIterator = mResults.begin();
	if (mResultsIterator != mResults.end())
	{
		//Ogre::LogManager::getSingleton().logMessage("GetFirstHit: " + (*mResultsIterator).movable->getParentNode()->getName() + " "  + (*mResultsIterator).movable->getMovableType());
		if ((*mResultsIterator).movable->getMovableType() == "Entity")
		{
			SGTGameObject *object = (SGTGameObject*)((*mResultsIterator).movable->getUserObject());//SGTSceneManager::Instance().FindObjectByInternName((*mResultsIterator).movable->getParentNode()->getName());//(SGTObject3D*)((*mResultsIterator).movable->getUserObject());
			if (object != 0) return object;
			else return GetNextHit();
		}
		else return GetNextHit();
	}
	//Ogre::LogManager::getSingleton().logMessage("Warning: SGTObjectLevelRayCaster::GetFirstHit() - return NULL");
	return NULL;
}

SGTGameObject* SGTObjectLevelRayCaster::GetNextHit()
{
	mResultsIterator++;
	if (mResultsIterator != mResults.end())
	{
		//Ogre::LogManager::getSingleton().logMessage("GetNextHit: " + (*mResultsIterator).movable->getParentNode()->getName() + " " + (*mResultsIterator).movable->getMovableType());
		if ((*mResultsIterator).movable->getMovableType() == "Entity")
		{
			SGTGameObject *object = (SGTGameObject*)((*mResultsIterator).movable->getUserObject());//SGTSceneManager::Instance().FindEntityByInternName((*mResultsIterator).movable->getParentNode()->getName());//(SGTObject3D*)((*mResultsIterator).movable->getUserObject());
			if (object != 0) return object;
			else return GetNextHit();
		}
		else return GetNextHit();
	}
	//Ogre::LogManager::getSingleton().logMessage("Warning: SGTObjectLevelRayCaster::GetNextHit() - return NULL");
	return NULL;
}

bool SGTObjectLevelRayCaster::HasMoreHits()
{
	mResultsIterator++;
	if (mResultsIterator != mResults.end())
	{
		if ((*mResultsIterator).movable->getMovableType() == "Entity")
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

bool SGTObjectLevelRayCaster::IsEmpty()
{
	return ((mResults.size() == 0) ? true : false);
}