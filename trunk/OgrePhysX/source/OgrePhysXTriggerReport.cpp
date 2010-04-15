
#include "OgrePhysXTriggerReport.h"

namespace OgrePhysX
{
	TriggerReport::TriggerReport(TriggerReportListener *listener)
	{
		mListener = listener;
	}
	TriggerReport::~TriggerReport()
	{
		delete mListener;
	}
	void TriggerReport::onTrigger(NxShape &triggerShape, NxShape &otherShape, NxTriggerFlag status)
	{
		if (status & NX_TRIGGER_ON_ENTER)
		{
			mListener->onEnter(triggerShape.getActor(), otherShape.getActor());
		}
		else if (status & NX_TRIGGER_ON_LEAVE)
		{
			mListener->onLeave(triggerShape.getActor(), otherShape.getActor());
		}
	}

}