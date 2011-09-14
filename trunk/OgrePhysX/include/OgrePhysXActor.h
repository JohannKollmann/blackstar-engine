#pragma once

#include "OgrePhysXConvert.h"

namespace OgrePhysX
{

	/**
	Helper class that wraps an PxRigidActor to keep code clean.
	Provides set/get position/orientation methods that accept Ogre math types.
	*/

	template<class T>		//T extends PxRigidActor
	class Actor
	{
	protected:
		T *mActor;

	public:
		Actor() : mActor(nullptr) {}
		Actor(T *actor) : mActor(actor) {}
		virtual ~Actor() {}

		void setPxActor(T *actor)
		{
			mActor = actor;
		}

		T* getPxActor() const
		{
			return mActor;
		}

		PxShape* getFirstShape()
		{
			PxShape *shapes[1];
			mActor->getShapes(shapes, 1);
			return shapes[0];
		}

		void setGlobalPosition(const Ogre::Vector3 &position)
		{
			mActor->setGlobalPose(PxTransform(Convert::toPx(position), mActor->getGlobalPose().q));
		}

		Ogre::Vector3 getGlobalPosition()
		{
			return Convert::toOgre(mActor->getGlobalPose().p);
		}

		void setGlobalOrientation(const Ogre::Quaternion &q)
		{
			mActor->setGlobalPose(PxTransform(mActor->getGlobalPose().p, Convert::toPx(q)));
		}

		Ogre::Quaternion getGlobalOrientation()
		{
			return Convert::toOgre(mActor->getGlobalPose().q);
		}

		void setGlobalTransform(const Ogre::Vector3 &position, const Ogre::Quaternion &q)
		{
			mActor->setGlobalPose(PxTransform(Convert::toPx(position), Convert::toPx(q)));
		}
	};
}