
#include "OgrePhysXRenderedActor.h"

namespace OgrePhysX
{

	RenderedActor::RenderedActor(NxScene *scene, PointRenderable *renderable, PrimitiveShape& shape, Ogre::Vector3 position, Ogre::Quaternion orientation) : Actor(scene, shape, position, orientation), mRenderable(renderable)
	{}
	RenderedActor::RenderedActor(NxScene *scene, PointRenderable *renderable, BaseMeshShape& shape, Ogre::Vector3 position, Ogre::Quaternion orientation) : Actor(scene, shape, position, orientation), mRenderable(renderable)
	{}
	RenderedActor::~RenderedActor()
	{
		if (mNxActor) mNxActor->getScene().releaseActor(*mNxActor);
		mNxActor = 0;
		delete mRenderable;
		mRenderable = 0;
	}

	void RenderedActor::sync()
	{
		mRenderable->setTransform(getGlobalPosition(), getGlobalOrientation());
	}

}