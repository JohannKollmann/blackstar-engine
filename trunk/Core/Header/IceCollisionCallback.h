
#pragma once

#include "OgrePhysX.h"

namespace Ice
{

	class TriggerCallback : public OgrePhysX::TriggerReportListener
	{
	public:
		TriggerCallback(void) {}
		~TriggerCallback(void) {}

		void onEnter(NxActor &trigger, NxActor &other);
		void onLeave(NxActor &trigger, NxActor &other);
	};

	class PhysXUserCallback : public NxUserNotify
	{
        void onSleep(NxActor** actors, NxU32 count);
		void onWake(NxActor** actors, NxU32 count);

		bool onJointBreak(NxReal breakingImpulse, NxJoint& brokenJoint);

	};

};