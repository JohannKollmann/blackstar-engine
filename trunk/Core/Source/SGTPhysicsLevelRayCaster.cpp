
#include "SGTPhysicsLevelRayCaster.h"

/*SGTPhysicsLevelRayCaster::SGTPhysicsLevelRayCaster(Ogre::Vector3 Origin, Ogre::Vector3 Direction, float Distance, NxOgre::RayCaster::RayCastType Filter, NxOgre::RayCaster::ActorFilter AFilter)
{
	NxOgre::RayCaster RayCaster(
		Origin,
		Direction,
		Distance,
		Filter,
		SGTMain::Instance().GetNxScene());
	RayCaster.cast();
};

SGTPhysicsLevelRayCaster::SGTPhysicsLevelRayCaster(Ogre::Vector3 Origin, Ogre::Vector3 Target, NxOgre::RayCaster::RayCastType Filter, float Distance, NxOgre::RayCaster::ActorFilter AFilter)
{
	Ogre::Vector3 Diff = Target - Origin;
	Ogre::Vector3 Direction = Diff.normalisedCopy();
	if (Distance == 99999) Distance = Diff.length() + 5;
	mRayCaster = new NxOgre::RayCaster(
		Origin,
		Direction,
		Distance,
		Filter,
		SGTMain::Instance().GetNxScene());
	mRayCaster->castShape(AFilter);
}

SGTPhysicsLevelRayCaster::SGTPhysicsLevelRayCaster(Ogre::Vector3 Origin, Ogre::Quaternion Orientation, float Distance, NxOgre::RayCaster::RayCastType Filter, NxOgre::RayCaster::ActorFilter AFilter)
{
	Ogre::Vector3 Direction = Orientation * -Ogre::Vector3::UNIT_Z;
	mRayCaster = new NxOgre::RayCaster(
		Origin,
		Direction,
		Distance,
		Filter,
		SGTMain::Instance().GetNxScene());
	mRayCaster->castShape(AFilter);
};

bool SGTPhysicsLevelRayCaster::isEmpty()
{
	if (mRayCaster->mReport.count() > 0) return false;
	return true;
};

NxOgre::RayCastReport SGTPhysicsLevelRayCaster::getAllHits()
{
	return mRayCaster->mReport;
};

NxOgre::RayCastHit SGTPhysicsLevelRayCaster::getClosestRaycastHit()
{
	return mRayCaster->getClosestRaycastHit();
};

NxOgre::Actor* SGTPhysicsLevelRayCaster::getClosestActorHit()
{
	return mRayCaster->getClosestActor();
};

NxOgre::CharacterSystem::Character* SGTPhysicsLevelRayCaster::getClosestCharacterHit()
{
	return mRayCaster->getClosestCharacter();
};*/