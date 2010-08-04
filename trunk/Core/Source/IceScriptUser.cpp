
#include "IceScriptUser.h"
#include "IceScriptSystem.h"
#include "IceMessageSystem.h"

namespace Ice
{
	ScriptUser::ScriptUser()
	{
		MessageSystem::Instance().JoinNewsgroup(this, "REPARSE_SCRIPTS_POST");
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
		if (msg.type == "REPARSE_SCRIPTS_POST")
		{
			OnScriptReload();
			InitScript(mScriptFileName);
		}
		else OnReceiveMessage(msg);
	}

}
