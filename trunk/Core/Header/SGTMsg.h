
#ifndef __SGTMsg_H__
#define __SGTMsg_H__

#include "SGTIncludes.h"
#include "Ogre.h"
#include "SGTMessageListener.h"
#include "SGTDataMap.h"


class SGTDllExport SGTMsg
{
public:

	SGTMsg()		
	{						//Default: Broadcast
		mReceiver = NULL;
		mNewsgroup = "COMMON";
	}
	virtual ~SGTMsg() { };

	SGTMessageListener *mReceiver;
	Ogre::String mNewsgroup;
	SGTDataMap mData;
};

#endif