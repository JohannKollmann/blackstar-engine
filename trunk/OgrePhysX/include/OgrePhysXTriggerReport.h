#pragma once

#include "OgrePhysXClasses.h"
#include "NxPhysics.h"

namespace OgrePhysX
{
	class OgrePhysXClass TriggerReportListener
	{
	public:
		virtual void onEnter(NxActor &trigger, NxActor &other) {};
		virtual void onLeave(NxActor &trigger, NxActor &other) {};
	};
	class OgrePhysXClass TriggerReport : public NxUserTriggerReport
	{
	private:
		TriggerReportListener *mListener;
	public:
		TriggerReport(TriggerReportListener *listener);
		~TriggerReport();
		void onTrigger(NxShape &triggerShape, NxShape &otherShape, NxTriggerFlag status);
	};

}