
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
			rawData = nullptr;
		}
		virtual ~Msg() { };

		Ogre::String type;
		DataMap params;
		void *rawData;
	};

};