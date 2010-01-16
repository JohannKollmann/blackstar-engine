
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
		//Ogre::LogManager::getSingleton().logMessage("GetFirstHit: " + (*mResultsIterator).movable->getParentNode()->getName() + " "  + (*mResultsIterator).movable->getMovableType());
		if ((*mResultsIterator).movable->getMovableType() == "Entity")
		{
			GameObject *object = (GameObject*)((*mResultsIterator).movable->getUserObject());//SceneManager::Instance().FindObjectByInternName((*mResultsIterator).movable->getParentNode()->getName());//(Object3D*)((*mResultsIterator).movable->getUserObject());
			if (object)
			{
				if (include_all || object->IsSelectable()) return object;
			}
			else return GetNextHit(include_all);
		}
		else return GetNextHit(include_all);
	}
	//Ogre::LogManager::getSingleton().logMessage("Warning: ObjectLevelRayCaster::GetFirstHit() - return NULL");
	return NULL;
}

GameObject* ObjectLevelRayCaster::GetNextHit(bool include_all)
{
	mResultsIterator++;
	if (mResultsIterator != mResults.end())
	{
		//Ogre::LogManager::getSingleton().logMessage("GetNextHit: " + (*mResultsIterator).movable->getParentNode()->getName() + " " + (*mResultsIterator).movable->getMovableType());
		if ((*mResultsIterator).movable->getMovableType() == "Entity")
		{
			GameObject *object = (GameObject*)((*mResultsIterator).movable->getUserObject());//SceneManager::Instance().FindEntityByInternName((*mResultsIterator).movable->getParentNode()->getName());//(Object3D*)((*mResultsIterator).movable->getUserObject());
			if (object)
			{
				if (include_all || object->IsSelectable()) return object;
			}
			else return GetNextHit(include_all);
		}
		else return GetNextHit(include_all);
	}
	//Ogre::LogManager::getSingleton().logMessage("Warning: ObjectLevelRayCaster::GetNextHit() - return NULL");
	return NULL;
}

bool ObjectLevelRayCaster::HasMoreHits()
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

bool ObjectLevelRayCaster::IsEmpty()
{
	return ((mResults.size() == 0) ? true : false);
}

};