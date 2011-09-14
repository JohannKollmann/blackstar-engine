#pragma once

#include "OgrePhysXClasses.h"
#include "OgrePhysXRenderableBinding.h"
#include "PxPhysics.h"
#include "OgrePhysXActor.h"

namespace OgrePhysX
{
	/*
	class RenderedActor
	implements the RenderableBinding interface.
	*/
	class RenderedActorBinding : public RenderableBinding
	{
		friend class Scene;

	private:
		PointRenderable *mRenderable;
		PxRigidDynamic *mPxActor;

		RenderedActorBinding(PxRigidDynamic *actor, PointRenderable *renderable);
		~RenderedActorBinding();

	public:
		void sync();
	};

}
