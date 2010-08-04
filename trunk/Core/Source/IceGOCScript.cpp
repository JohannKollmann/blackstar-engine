
#include "IceGOCScript.h"

namespace Ice
{
	GOCScript::GOCScript()
	{
		MessageSystem::Instance().JoinNewsgroup(this, "REPARSE_SCRIPTS");
	}

	std::vector<ScriptParam> GOCScript::Lua_ListenToObjectEvent(Script &caller, std::vector<ScriptParam> params)
	{
		std::vector<ScriptParam> out;
		if (params.size() < 2)
		{
			Utils::LogParameterErrors(caller, "Too few parameters!");
			return out;
		}
		if (params[0].getType() != ScriptParam::PARM_TYPE_STRING || params[1].getType() != ScriptParam::PARM_TYPE_FUNCTION)
		{
			Utils::LogParameterErrors(caller, "Wrong parameters!");
			return out;
		}

		ScriptUser *scriptUser = ScriptSystem::GetInstance().GetScriptableObject(caller.GetID());
		if (!scriptUser) IceWarning("No object is associated with the calling script.")
		else
		{
			GOCScript *gocScript = dynamic_cast<GOCScript*>(scriptUser);
			if (!gocScript) IceWarning("Associated object does not have a script component.")
			else
			{
				Ogre::String msgName = params[0].getString();
				gocScript->mObjectMsgCallbacks.insert(std::make_pair<Ogre::String, ScriptParam>(msgName, params[1]));
			}
		}

		return out;
	}

	void GOCScript::ReceiveObjectMessage(Msg &msg)
	{
		auto i = mObjectMsgCallbacks.find(msg.type);
		if (i == mObjectMsgCallbacks.end()) return;

		std::vector<ScriptParam> params;
		while (msg.params.HasNext())
		{
			msg.params.GetNext().data.GetAsScriptParam(params);
		}
		ScriptSystem::RunCallbackFunction(i->second, params);
	}

	void GOCScript::OnReceiveMessage(Msg &msg)
	{
		if (msg.type == "REPARSE_SCRIPTS") mObjectMsgCallbacks.clear();
	}

}