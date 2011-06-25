
#pragma once

#include "boost/thread.hpp"
#include "OgreLogManager.h"

/**
* Implements a thread safe log using Ogre::LogManager.
*/

namespace Ice
{
	class MessageListener;

	class __declspec(dllexport) Log
	{
	private:
		MessageListener* mLogMessageListener;

	public:
		Log();
		~Log();

		enum MessageIDs
		{
			LOG_MESSAGE = 210, LOG_CRITICALERROR = 213
		};

		void LogMessage(Ogre::String message);
		void LogWarning(Ogre::String message);
		void LogError(Ogre::String message);
		void LogCriticalError(Ogre::String message);

		void Shutdown();

		static Log& Instance();
	};
}