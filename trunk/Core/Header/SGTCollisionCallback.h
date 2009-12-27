
#pragma once

#include "OgrePhysX.h"

class SGTTriggerCallback : public OgrePhysX::TriggerReportListener
{
public:
	SGTTriggerCallback(void) {}
	~SGTTriggerCallback(void) {}

	void onEnter(OgrePhysX::Actor *trigger, OgrePhysX::Actor *other);
	void onLeave(OgrePhysX::Actor *trigger, OgrePhysX::Actor *other);
};