
#pragma once

#include "SGTMain.h"
#include "NxOgre.h"
#include "NxOgreRaycaster.h"
#include "Ogre.h"
#include "SGTIncludes.h"


class SGTDllExport SGTPhysicsLevelRayCaster
{
/*private:
	NxOgre::Raycaster *mRayCaster;

public:

	/*
	Ertellt ein Ray, welches von einem Startpunkt Origin in Richtung Direction fliegt.
	
	SGTPhysicsLevelRayCaster(Ogre::Vector3 Origin, Ogre::Vector3 Direction, float Distance, NxOgre::Raycaster::RaycastType Filter = NxOgre::Raycaster::RaycastType::RCT_Closest, NxOgre::Raycaster::ActorFilter AFilter = NxOgre::Raycaster::ActorFilter::AF_None);

	/*
	Erstellt ein Ray, was von einem Startpunkt Origin zu einem Ziel Target fliegt.
	
	SGTPhysicsLevelRayCaster(Ogre::Vector3 Origin, Ogre::Vector3 Target, NxOgre::Raycaster::RaycastType Filter = NxOgre::Raycaster::RaycastType::RCT_Closest, float Distance = 99999, NxOgre::Raycaster::ActorFilter AFilter = NxOgre::Raycaster::ActorFilter::AF_None);

	/*
	Angenommen man hat ein Npc mit der Position Origin und der Blickrichtung Orientation,
	so würde dieser Konstruktor ein Ray in Blickrichtung schießen.
	
	SGTPhysicsLevelRayCaster(Ogre::Vector3 Origin, Ogre::Quaternion Orientation, float Distance, NxOgre::Raycaster::RaycastType Filter = NxOgre::Raycaster::RaycastType::RCT_Closest, NxOgre::Raycaster::ActorFilter AFilter = NxOgre::Raycaster::ActorFilter::AF_None);

	~SGTPhysicsLevelRayCaster() {};

	bool isEmpty();
	NxOgre::Actor* getClosestActorHit();
	NxOgre::CharacterSystem::Character* getClosestCharacterHit();
	NxOgre::RayCastHit getClosestRaycastHit();
	NxOgre::RayCastReport getAllHits(); */

};
