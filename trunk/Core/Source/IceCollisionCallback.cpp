
#include "IceCollisionCallback.h"
#include "IceMain.h"
#include "IceGameObject.h"
#include "IceGOCPhysics.h"

namespace Ice
{

void TriggerCallback::onEnter(OgrePhysX::Actor *trigger, OgrePhysX::Actor *other)
{
	if (trigger->userData && other->userData)
	{
		GameObject *triggerObject = (GameObject*)trigger->userData;
		GOCTrigger *trigger = (GOCTrigger*)triggerObject->GetComponent("Trigger");
		if (trigger)
		{
			GameObject *otherObject = (GameObject*)other;
			trigger->onEnter(otherObject);
		}
	}
}
void TriggerCallback::onLeave(OgrePhysX::Actor *trigger, OgrePhysX::Actor *other)
{
	if (trigger->userData && other->userData)
	{
		GameObject *triggerObject = (GameObject*)trigger->userData;
		GOCTrigger *trigger = (GOCTrigger*)triggerObject->GetComponent("Trigger");
		if (trigger)
		{
			GameObject *otherObject = (GameObject*)other;
			trigger->onLeave(otherObject);
		}
	}
}

};