
#include "IceScriptUser.h"
#include "IceScriptSystem.h"
#include "IceMessageSystem.h"

namespace Ice
{
	ScriptUser::ScriptUser()
	{
		MessageSystem::JoinNewsgroup(this, GlobalMessageIDs::REPARSE_SCRIPTS_POST);
	}

	ScriptUser::~ScriptUser()
	{
		ScriptSystem::GetInstance().UnregisterScriptUser(mScript.GetID());
	}

	void ScriptUser::InitScript(Ogre::String scriptFilename)
	{
		mScriptFileName = scriptFilename;
		mScript = ScriptSystem::GetInstance().CreateInstance(scriptFilename, false);
		ScriptSystem::GetInstance().RegisterScriptUser(this, mScript.GetID());
		mScript.CallFunction("create", std::vector<ScriptParam>());
	}
	void ScriptUser::ReceiveMessage(Msg& msg)
	{
		if (msg.type == GlobalMessageIDs::REPARSE_SCRIPTS_POST && mScriptFileName != "")
		{
			OnScriptReload();
			InitScript(mScriptFileName);
		}
		else OnReceiveMessage(msg);
	}

}
