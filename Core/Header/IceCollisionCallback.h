
#pragma once

#include "OgrePhysX.h"
#include "IceScript.h"
#include "IceMessageSystem.h"
#include "IceIncludes.h"
#include "PxPhysicsAPI.h"

namespace Ice
{

	class DllExport PhysXContactReport : public PxSimulationEventCallback
	{
	private:

		void onMaterialContact(Ogre::String material1, Ogre::String material2, Ogre::Vector3 position, float force);

	public:
		PhysXContactReport() {}
		~PhysXContactReport() {}

		void onContact(PxContactPair &pair, PxU32 events);

		void onSleep(PxActor** actors, PxU32 count);
		void onWake(PxActor** actors, PxU32 count);

		void onTrigger(PxTriggerPair *pairs, PxU32 count);

		void onConstraintBreak(PxConstraintInfo *constraints, PxU32 count);
	};

};