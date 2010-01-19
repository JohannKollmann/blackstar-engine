
#include "IceCollisionCallback.h"
#include "IceMain.h"
#include "IceGameObject.h"
#include "IceGOCPhysics.h"

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

};