
#pragma once

#include "IceIncludes.h"
#include "IceMessageSystem.h"
#include "IceLog.h"
#include "OgreLogManager.h"

namespace Ice
{
	class DllExport LogMessageListener : public ViewMessageListener
	{
	public:
		void ReceiveMessage(Msg &msg)
		{
			if (msg.typeID == Log::MessageIDs::LOG_MESSAGE)
			{
				Ogre::LogManager::getSingleton().logMessage(msg.params.GetValue<Ogre::String>(0));
			}
			if (msg.typeID == Log::MessageIDs::LOG_CRITICALERROR)
			{
				Ogre::LogManager::getSingleton().logMessage(msg.params.GetValue<Ogre::String>(0), Ogre::LogMessageLevel::LML_CRITICAL);
			}
		}
	};
};