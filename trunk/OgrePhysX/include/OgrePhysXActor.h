#pragma once

#include "OgrePhysXClasses.h"
#include "Ogre.h"
#include "OgrePhysXRenderableBinding.h"
#include "NxActor.h"
#include "OgrePhysXShapes.h"
#include "NxScene.h"

namespace OgrePhysX
{

	/*
	class Actor
	Wraps an NxActor.
	*/
	class OgrePhysXClass Actor
	{
		friend class Scene;
		friend class Ragdoll;

	protected:
		NxActor *mNxActor;

		Actor(NxScene *scene, PrimitiveShape& shape, Ogre::Vector3 position = Ogre::Vector3(0,0,0), Ogre::Quaternion orientation = Ogre::Quaternion());
		Actor(NxScene *scene, BaseMeshShape& shape, Ogre::Vector3 position = Ogre::Vector3(0,0,0), Ogre::Quaternion orientation = Ogre::Quaternion());
		virtual ~Actor();

	public:

		NxActor* getNxActor();

		Ogre::Vector3 getGlobalPosition();
		Ogre::Quaternion getGlobalOrientation();

		void setGlobalPosition(Ogre::Vector3 position, bool moveKinematic = true);
		void setGlobalOrientation(Ogre::Quaternion rotation);
		void setGlobalPose(Ogre::Vector3 position, Ogre::Quaternion rotation, bool moveKinematic = true);
	};

}
