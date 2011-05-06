
#pragma once

#include "IceIncludes.h"
#include "Ogre.h"
#include "IceMessageListener.h"

namespace Ice
{
	typedef int MsgType;

	class DllExport Msg
	{
	public:

		Msg()		
		{
			rawData = nullptr;
		}
		virtual ~Msg() { }

		Ogre::Any params[8];
		MsgType typeID;
		void *rawData;
	};
};