
#pragma once

#include "OgrePhysX.h"
#include "IceScript.h"
#include "IceMessageSystem.h"
#include "IceIncludes.h"

namespace Ice
{

	class DllExport ActorContactReport : public NxUserContactReport
	{
	private:
		void onMaterialContact(Ogre::String material1, Ogre::String material2, Ogre::Vector3 position, float force);

	public:
		ActorContactReport();
		~ActorContactReport() {}
		void onContactNotify(NxContactPair &pair, NxU32 events);
	};

	class DllExport TriggerCallback : public OgrePhysX::TriggerReportListener
	{
	public:
		TriggerCallback(void) {}
		~TriggerCallback(void) {}

		void onEnter(NxActor &trigger, NxActor &other);
		void onLeave(NxActor &trigger, NxActor &other);
	};

	class DllExport PhysXUserCallback : public NxUserNotify
	{
        void onSleep(NxActor** actors, NxU32 count);
		void onWake(NxActor** actors, NxU32 count);

		bool onJointBreak(NxReal breakingImpulse, NxJoint& brokenJoint);

	};

};