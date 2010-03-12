
#include "IceCollisionCallback.h"
#include "IceMain.h"
#include "IceGameObject.h"
#include "IceGOCPhysics.h"
#include "IceMessageSystem.h"

namespace Ice
{

	void TriggerCallback::onEnter(NxActor &trigger, NxActor &other)
	{
		if (trigger.userData && other.userData)
		{
			GameObject *triggerObject = (GameObject*)trigger.userData;
			GOCTrigger *trigger = (GOCTrigger*)triggerObject->GetComponent("Trigger");
			if (trigger)
			{
				GameObject *otherObject = (GameObject*)other.userData;
				trigger->onEnter(otherObject);
			}
		}
	}
	void TriggerCallback::onLeave(NxActor &trigger, NxActor &other)
	{
		if (trigger.userData && other.userData)
		{
			GameObject *triggerObject = (GameObject*)trigger.userData;
			GOCTrigger *trigger = (GOCTrigger*)triggerObject->GetComponent("Trigger");
			if (trigger)
			{
				GameObject *otherObject = (GameObject*)other.userData;
				trigger->onLeave(otherObject);
			}
		}
	}


	void PhysXUserCallback::onSleep(NxActor** actors, NxU32 count)
	{
		for (unsigned int i = 0; i < count; i++)
		{
			Msg msg;
			msg.mNewsgroup = "ACOTR_ONSLEEP";
			msg.rawData = actors[i];
			MessageSystem::Instance().SendMessage(msg);
		}
	}

	void PhysXUserCallback::onWake(NxActor** actors, NxU32 count)
	{
		for (unsigned int i = 0; i < count; i++)
		{
			Msg msg;
			msg.mNewsgroup = "ACOTR_ONWAKE";
			msg.rawData = actors[i];
			MessageSystem::Instance().SendMessage(msg);
		}
	}

	bool PhysXUserCallback::onJointBreak(NxReal breakingImpulse, NxJoint& brokenJoint)
	{
		return true;
	}

};