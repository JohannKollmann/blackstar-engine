
#include "IceGOCScript.h"

namespace Ice
{
	void GOCScript::OnSetParameters()
	{
		if (mOwnerGO) InitScript(mScriptFileName);
	}

	void GOCScript::SetOwner(GameObject *go)
	{
		mOwnerGO = go;
		if (mScriptFileName != "") InitScript(mScriptFileName);
	}


	GOCScriptMessageCallback::GOCScriptMessageCallback()
	{
		MessageSystem::Instance().JoinNewsgroup(this, "REPARSE_SCRIPTS");
	}

	void GOCScriptMessageCallback::ReceiveMessage(Msg &msg)
	{
		if (msg.type == "REPARSE_SCRIPTS") mObjectMsgCallbacks.clear();
	}

	void GOCScriptMessageCallback::ReceiveObjectMessage(Msg &msg)
	{
		auto i = mObjectMsgCallbacks.find(msg.type);
		if (i == mObjectMsgCallbacks.end()) return;

		std::vector<ScriptParam> params;
		while (msg.params.HasNext())
		{
			msg.params.GetNext().data.GetAsScriptParam(params);
		}
		for (auto c = 0; c < i->second.size(); c++)
		{
			ScriptSystem::RunCallbackFunction(i->second[c], params);
		}
	}

	bool GOCScriptMessageCallback::HasListener(const Ogre::String &msgType)
	{
		auto i = mObjectMsgCallbacks.find(msgType);
		return (i != mObjectMsgCallbacks.end());
	}
	void GOCScriptMessageCallback::AddListener(const Ogre::String &msgType, ScriptParam callback)
	{
		auto i = mObjectMsgCallbacks.find(msgType);
		if (i == mObjectMsgCallbacks.end())
		{
			mObjectMsgCallbacks.insert(std::make_pair(msgType, std::vector<ScriptParam>()));
			i = mObjectMsgCallbacks.find(msgType);
		}
		i->second.push_back(callback);
	}

}