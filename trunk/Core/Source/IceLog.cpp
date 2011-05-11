
#include "IceLog.h"
#include "IceMessageSystem.h"

namespace Ice
{

	void Log::LogMessage(Ogre::String message)
	{
		mMutex.lock();
		Ogre::LogManager::getSingleton().logMessage(message);
		mMutex.unlock();
	}

	void Log::LogWarning(Ogre::String message)
	{
		mMutex.lock();
		Ogre::LogManager::getSingleton().logMessage("Warning: " + message);
		mMutex.unlock();
	}


	void Log::LogError(Ogre::String message)
	{
		mMutex.lock();
		Ogre::LogManager::getSingleton().logMessage("Error: " + message);
		mMutex.unlock();
	}

	void Log::LogCriticalError(Ogre::String message)
	{
		mMutex.lock();
		Ogre::LogManager::getSingleton().logMessage("Critical error: " + message, Ogre::LML_CRITICAL);
		mMutex.unlock();
	}

	Log& Log::Instance()
	{
		static Log TheOneAndOnly;
		return TheOneAndOnly;
	}

}