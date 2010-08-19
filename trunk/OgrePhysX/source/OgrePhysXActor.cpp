
#include "OgrePhysXActor.h"
#include "OgrePhysXConvert.h"

namespace OgrePhysX
{

	Actor::Actor(NxScene *scene, PrimitiveShape& shape, Ogre::Vector3 position, Ogre::Quaternion orientation)
	{
		NxActorDesc ad;
		ad.setToDefault();
		if (shape.getDesc()->mass <= 0) ad.density = shape.getDesc()->density;
		ad.globalPose = NxMat34(NxMat33(Convert::toNx(orientation)), Convert::toNx(position));

		if (shape.getDesc()->density == 0 && shape.getDesc()->mass == 0)
		{
			ad.body = 0;	//actor ist static
		}
		else
		{
			NxBodyDesc bd;
			bd.setToDefault();
			if (shape.getDesc()->mass > 0) bd.mass = shape.getDesc()->mass;
			ad.body = &bd;
		}

		ad.shapes.push_back(shape.getDesc());

		mNxActor = scene->createActor(ad);
		if (!mNxActor) Ogre::LogManager::getSingleton().logMessage("[OgrePhysX] Error: createActor failed!");
	}

	Actor::Actor(NxScene *scene, BaseMeshShape& shape, Ogre::Vector3 position, Ogre::Quaternion orientation)
	{
		NxActorDesc ad;
		ad.setToDefault();
		ad.globalPose = NxMat34(NxMat33(Convert::toNx(orientation)), Convert::toNx(position));

		ad.body = 0;	//actor is static

		ad.shapes.push_back(shape.getDesc());

		mNxActor = scene->createActor(ad);
		if (!mNxActor) Ogre::LogManager::getSingleton().logMessage("[OgrePhysX] Error: createActor failed!");
	}

	Actor::~Actor(void)
	{
		if (mNxActor) mNxActor->getScene().releaseActor(*mNxActor);
		mNxActor = 0;
	}

	NxActor* Actor::getNxActor()
	{
		return mNxActor;
	}

	Ogre::Vector3 Actor::getGlobalPosition()
	{
		return Convert::toOgre(mNxActor->getGlobalPosition());
	}
	Ogre::Quaternion Actor::getGlobalOrientation()
	{
		return Convert::toOgre(mNxActor->getGlobalOrientationQuat());
	}
	void Actor::setGlobalPosition(Ogre::Vector3 position)
	{
		if (mNxActor->isDynamic() && mNxActor->readBodyFlag(NxBodyFlag::NX_BF_KINEMATIC)) mNxActor->moveGlobalPosition(Convert::toNx(position));
		else mNxActor->setGlobalPosition(Convert::toNx(position));
	}
	void Actor::setGlobalOrientation(Ogre::Quaternion rotation)
	{
		if (mNxActor->isDynamic() && mNxActor->readBodyFlag(NxBodyFlag::NX_BF_KINEMATIC)) mNxActor->moveGlobalOrientation(Convert::toNx(rotation));
		else mNxActor->setGlobalOrientation(Convert::toNx(rotation));
	}

}
