
#include "IceScriptable.h"

namespace Ice
{
	Scriptable::~Scriptable()
	{
		ScriptSystem::GetInstance().UnregisterScriptable(mScript.GetID());
	}

	void Scriptable::InitScript(Ogre::String scriptFilename)
	{
		mScriptFileName = scriptFilename;
		mScript = ScriptSystem::GetInstance().CreateInstance(scriptFilename);
		ScriptSystem::GetInstance().RegisterScriptable(this, mScript.GetID());
	}

	std::vector<ScriptParam> Scriptable::ScriptCall(MethodID method, std::vector<ScriptParam> &params)
	{
		std::vector<ScriptParam> out;
		if (method == "SetProperty")
		{
			if (params.size() < 2) return out;
			if (params[0].getType() != ScriptParam::PARM_TYPE_STRING) return out;
			std::string key = params[0].getString();
			mProperties[key] = params[1];
			return out;
		}
		else if (method == "GetProperty")
		{
			if (params.size() != 1) return out;
			if (params[0].getType() != ScriptParam::PARM_TYPE_STRING) return out;
			std::string key = params[0].getString();
			auto i = mProperties.find(key);
			if (i == mProperties.end()) return out;
			out.push_back(i->second);
			return out;
		}
		else return OnScriptCall(method, params);

		assert(false);	//something went wrong
		return out;
	}

	void Scriptable::ReceiveMessage(const Msg& msg)
	{
		if (msg.type == "REPARSE_SCRIPTS_POST")
		{
			OnScriptReload();
			InitScript(mScriptFileName);
		}
		else OnReceiveMessage(msg);
	}
}
