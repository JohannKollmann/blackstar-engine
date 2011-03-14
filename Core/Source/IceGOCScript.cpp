
#include "IceGOCScript.h"

namespace Ice
{
	void GOCScript::Create()
	{
		if (!mOwnerGO) return;
		mScripts.clear();
		auto names = Ogre::StringUtil::split(mScriptFileNames, ";");
		ITERATE(i, names)
			mScripts.push_back(std::make_shared<ScriptItem>(this, *i));
	}

	void GOCScript::SetOwner(GameObject *go)
	{
		mOwnerGO = go;
	}

	std::vector<ScriptParam> GOCScript::Script_SetProperty(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		Ogre::String key = vParams[0].getString().c_str();
		mScriptProperties[key] = vParams[1];
		return out;
	}
	std::vector<ScriptParam> GOCScript::Script_GetProperty(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		Ogre::String key = vParams[0].getString().c_str();
		auto i = mScriptProperties.find(key);
		if (i == mScriptProperties.end()) return out;
		out.push_back(i->second);
		return out;
	}
	std::vector<ScriptParam> GOCScript::Script_HasProperty(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		Ogre::String key = vParams[0].getString().c_str();
		auto i = mScriptProperties.find(key);
		out.push_back(i != mScriptProperties.end());
		return out;
	}

	void GOCScript::SetParameters(DataMap *parameters)
	{
		mScriptProperties.clear();
		mScriptFileNames = parameters->GetValue<Ogre::String>("Script Filenames", "");

		while (parameters->HasNext())
		{
			auto item = parameters->GetNext();
			if (item.key == "Script Filenames") continue;
			std::vector<ScriptParam> vParams;
			item.data.GetAsScriptParam(vParams);
			if (vParams.size() == 1) mScriptProperties[item.key] = vParams[0];
		}
	}
	void GOCScript::GetParameters(DataMap *parameters)
	{
		parameters->AddOgreString("Script Filenames", mScriptFileNames);
		ITERATE(i, mScriptProperties)
			parameters->AddScriptParam(i->first, i->second);
	}
	void GOCScript::GetDefaultParameters(DataMap *parameters)
	{
		parameters->AddOgreString("Script Filenames", "a.lua;b.lua");
	}

	void GOCScript::Save(LoadSave::SaveSystem& mgr)
	{
		//DataMap map;
		GetParameters(&mTempMap);
		mgr.SaveObject(&mTempMap, "Data", false, false);
	}
	void GOCScript::Load(LoadSave::LoadSystem& mgr)
	{
		DataMap *map = (DataMap*)mgr.LoadObject();
		SetParameters(map);
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
		if (msg.type == "INTERN_RESET")
		{
			mObjectMsgCallbacks.clear();
			return;
		}

		auto i = mObjectMsgCallbacks.find(msg.type);
		if (i == mObjectMsgCallbacks.end()) return;

		std::vector<ScriptParam> params;
		while (msg.params.HasNext())
		{
			msg.params.GetNext().data.GetAsScriptParam(params);
		}
		for (unsigned int c = 0; c < i->second.size(); c++)
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