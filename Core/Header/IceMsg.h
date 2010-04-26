
#pragma once

#include "IceIncludes.h"
#include "Ogre.h"
#include "IceMessageListener.h"
#include "IceDataMap.h"

namespace Ice
{

	class DllExport Msg
	{
	public:

		Msg()		
		{						//Default: Broadcast
			mReceiver = nullptr;
			mNewsgroup = "COMMON";
		}
		virtual ~Msg() { };

		MessageListener *mReceiver;
		Ogre::String mNewsgroup;
		DataMap mData;
		void *rawData;
	};

};