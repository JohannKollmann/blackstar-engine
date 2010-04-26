#pragma once

#include "OgrePhysXClasses.h"
#include "Ogre.h"
#include "OgrePhysXPointRenderable.h"

namespace OgrePhysX
{
	class OgrePhysXClass RenderableBinding
	{
	public:
		virtual ~RenderableBinding() {}

		virtual void sync() = 0;
	};
}