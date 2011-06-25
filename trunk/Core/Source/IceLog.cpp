
#include "IceLog.h"
#include "IceMessageSystem.h"
#include "IceLogMessageListener.h"

namespace Ice
{

	Log::Log()
	{
		mLogMessageListener = new LogMessageListener();
	}
	Log::~Log()
	{
		Shutdown();
	}

	void Log::LogMessage(Ogre::String message)
	{
		Msg msg;
		msg.typeID = MessageIDs::LOG_MESSAGE;
		msg.params.AddOgreString("Message", message);
		MessageSystem::Instance().SendMessage(msg, mLogMessageListener);
	}

	void Log::LogWarning(Ogre::String message)
	{
		LogMessage("Warning: " + message);
	}


	void Log::LogError(Ogre::String message)
	{
		LogMessage("Error: " + message);
	}

	void Log::LogCriticalError(Ogre::String message)
	{
		Msg msg;
		msg.typeID = MessageIDs::LOG_CRITICALERROR;
		msg.params.AddOgreString("Message", "Critical error: " + message);
		MessageSystem::Instance().SendMessage(msg, mLogMessageListener);
	}

	void Log::Shutdown()
	{
		if (mLogMessageListener)
		{
			delete mLogMessageListener;
			mLogMessageListener = nullptr;
		}
	}

	Log& Log::Instance()
	{
		static Log TheOneAndOnly;
		return TheOneAndOnly;
	}

}