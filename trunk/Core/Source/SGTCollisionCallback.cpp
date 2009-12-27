
#include "SGTCollisionCallback.h"
#include "SGTMain.h"
#include "SGTGameObject.h"
#include "SGTGOCPhysics.h"


void SGTTriggerCallback::onEnter(OgrePhysX::Actor *trigger, OgrePhysX::Actor *other)
{
	if (trigger->userData && other->userData)
	{
		SGTGameObject *triggerObject = (SGTGameObject*)trigger->userData;
		SGTGOCTrigger *trigger = (SGTGOCTrigger*)triggerObject->GetComponent("Trigger");
		if (trigger)
		{
			SGTGameObject *otherObject = (SGTGameObject*)other;
			trigger->onEnter(otherObject);
		}
	}
}
void SGTTriggerCallback::onLeave(OgrePhysX::Actor *trigger, OgrePhysX::Actor *other)
{
	if (trigger->userData && other->userData)
	{
		SGTGameObject *triggerObject = (SGTGameObject*)trigger->userData;
		SGTGOCTrigger *trigger = (SGTGOCTrigger*)triggerObject->GetComponent("Trigger");
		if (trigger)
		{
			SGTGameObject *otherObject = (SGTGameObject*)other;
			trigger->onLeave(otherObject);
		}
	}
}