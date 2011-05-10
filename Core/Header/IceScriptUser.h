
#pragma once

#include "IceIncludes.h"
#include "OgreString.h"
#include "IceMessageListener.h"
#include "IceScript.h"

namespace Ice
{
	class DllExport ScriptUser : public SynchronizedMessageListener
	{
	protected:
		Ogre::String mScriptFileName;

		void InitScript(Ogre::String scriptFilename);

		virtual void OnScriptReload() {}
	public:

		Script mScript;

		ScriptUser();
		virtual ~ScriptUser();

		/*
		Handles Script reparse functionality and calls OnReceiveMessage
		*/
		void ReceiveMessage(Msg& msg);

		virtual void OnReceiveMessage(Msg& msg) {}

		virtual int GetThisID() = 0;
	};
}