#pragma once

#include "OgrePhysXClasses.h"
#include "OgrePhysXConvert.h"
#include "OgrePhysXWorld.h"
#include "OgrePhysXScene.h"
#include "OgrePhysXRenderedActorBinding.h"
#include "OgrePhysXPointRenderable.h"
#include "OgrePhysXNodeRenderable.h"
#include "OgrePhysXCooker.h"
#include "OgrePhysXRagdoll.h"
#include "OgrePhysXGeometry.h"
#include "OgrePhysXActor.h"

namespace OgrePhysX
{
	///Shortcut function, retrieves the global PxPhysics object.
	static PxPhysics* getPxPhysics()
	{
		return World::getSingleton().getPxPhysics();
	}

	using namespace Convert;	//for easy use
}