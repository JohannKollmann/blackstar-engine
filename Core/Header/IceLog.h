
#pragma once

#include "boost/thread.hpp"
#include "OgreLogManager.h"

/**
* Implements a thread safe log using Ogre::LogManager.
*/

namespace Ice
{
	class __declspec(dllexport) Log
	{
	private:
		boost::mutex mMutex;

	public:
		void LogMessage(Ogre::String message);
		void LogWarning(Ogre::String message);
		void LogError(Ogre::String message);
		void LogCriticalError(Ogre::String message);

		static Log& Instance();
	};
}