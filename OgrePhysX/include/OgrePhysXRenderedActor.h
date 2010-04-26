#pragma once

#include "OgrePhysXClasses.h"
#include "OgrePhysXRenderableBinding.h"
#include "OgrePhysXActor.h"

namespace OgrePhysX
{
	/*
	class RenderedActor
	implements the RenderableBinding interface.
	*/
	class RenderedActor : public Actor, public RenderableBinding
	{
		friend class Scene;

	private:
		PointRenderable *mRenderable;

		RenderedActor(NxScene *scene, PointRenderable *renderable, PrimitiveShape& shape, Ogre::Vector3 position = Ogre::Vector3(0,0,0), Ogre::Quaternion orientation = Ogre::Quaternion());
		RenderedActor(NxScene *scene, PointRenderable *renderable, BaseMeshShape& shape, Ogre::Vector3 position = Ogre::Vector3(0,0,0), Ogre::Quaternion orientation = Ogre::Quaternion());
		~RenderedActor();

	public:
		void sync();
	};

}
