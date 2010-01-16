
#pragma once

#include "OgrePhysX.h"

namespace Ice
{

class TriggerCallback : public OgrePhysX::TriggerReportListener
{
public:
	TriggerCallback(void) {}
	~TriggerCallback(void) {}

	void onEnter(OgrePhysX::Actor *trigger, OgrePhysX::Actor *other);
	void onLeave(OgrePhysX::Actor *trigger, OgrePhysX::Actor *other);
};

};