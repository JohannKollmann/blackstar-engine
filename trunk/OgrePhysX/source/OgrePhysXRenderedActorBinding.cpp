
#include "OgrePhysXRenderedActorBinding.h"
#include "OgrePhysXConvert.h"
#include "PxRigidDynamic.h"

namespace OgrePhysX
{

	RenderedActorBinding::RenderedActorBinding(PxRigidDynamic *actor, PointRenderable *renderable)
		: mPxActor(actor), mRenderable(renderable)
	{
	}

	RenderedActorBinding::~RenderedActorBinding()
	{
	}

	void RenderedActorBinding::sync()
	{
		PxTransform pose = mPxActor->getGlobalPose();
		mRenderable->setTransform(Convert::toOgre(pose.p), Convert::toOgre(pose.q));
	}

}