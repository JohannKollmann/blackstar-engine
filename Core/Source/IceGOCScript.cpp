
#include "IceGOCScript.h"
#include "IceObjectMessageIDs.h"

namespace Ice
{
	void GOCScriptedProperties::SetProperty(const Ogre::String &propertyName, const ScriptParam &prop)
	{
		mScriptProperties[propertyName] = prop;
	}
	void GOCScriptedProperties::GetProperty(const Ogre::String &propertyName, ScriptParam &prop)
	{
		prop.set(mScriptProperties[propertyName]);
	}
	bool GOCScriptedProperties::HasProperty(const Ogre::String &propertyName)
	{
		return (mScriptProperties.find(propertyName) != mScriptProperties.end());
	}

	void GOCScriptedProperties::SetParameters(DataMap *parameters)
	{
		mScriptProperties.clear();

		while (parameters->HasNext())
		{
			auto item = parameters->GetNext();
			ScriptParam param;
			item.Data->GetAsScriptParam(param);
			mScriptProperties[item.Key] = param;
		}
	}
	void GOCScriptedProperties::GetParameters(DataMap *parameters)
	{
		ITERATE(i, mScriptProperties)
			parameters->AddScriptParam(i->first, i->second);
	}
	void GOCScriptedProperties::GetDefaultParameters(DataMap *parameters) {}

	void GOCScriptedProperties::Save(LoadSave::SaveSystem& mgr)
	{
		//DataMap map;
		GetParameters(&mTempMap);
		mgr.SaveObject(&mTempMap, "Data", false, false);
	}
	void GOCScriptedProperties::Load(LoadSave::LoadSystem& mgr)
	{
		std::shared_ptr<DataMap> map = mgr.LoadTypedObject<DataMap>();
		SetParameters(map.get());
	}

	void GOCScript::Create()
	{
		if (mOwnerGO.expired()) return;
		mScripts.clear();
		auto names = Ogre::StringUtil::split(mScriptFileNames, ";");
		ITERATE(i, names)
			mScripts.push_back(std::make_shared<ScriptItem>(this, *i));
	}

	void GOCScript::SetParameters(DataMap *parameters)
	{
		mScriptFileNames = parameters->GetValue<Ogre::String>("Script Filenames", "");
	}
	void GOCScript::GetParameters(DataMap *parameters)
	{
		parameters->AddOgreString("Script Filenames", mScriptFileNames);
	}
	void GOCScript::GetDefaultParameters(DataMap *parameters)
	{
		parameters->AddOgreString("Script Filenames", "a.lua;b.lua");
	}

	void GOCScript::Save(LoadSave::SaveSystem& mgr)
	{
	}
	void GOCScript::Load(LoadSave::LoadSystem& mgr)
	{
	}

	GOCScriptMessageCallback::GOCScriptMessageCallback()
	{
		JoinNewsgroup(GlobalMessageIDs::REPARSE_SCRIPTS_PRE);
	}

	void GOCScriptMessageCallback::ReceiveMessage(Msg &msg)
	{
		if (msg.typeID == GlobalMessageIDs::REPARSE_SCRIPTS_PRE)
		{
			mObjectMsgCallbacks.clear();
			return;
		}

		if (msg.typeID == ObjectMessageIDs::INTERN_RESET)
		{
			mObjectMsgCallbacks.clear();
			return;
		}

		auto i = mObjectMsgCallbacks.find(msg.typeID);
		if (i == mObjectMsgCallbacks.end()) return;

		std::map<ScriptParam, ScriptParam> table;
		Utils::DataMapToTable(Script(), msg.params, table);
		ScriptParam parm(table);
		for (unsigned int c = 0; c < i->second.size(); c++)
		{
			ScriptSystem::RunCallbackFunction(i->second[c], std::vector<ScriptParam>(1, parm));
		}
	}

	bool GOCScriptMessageCallback::HasListener(MsgTypeID msgType)
	{
		auto i = mObjectMsgCallbacks.find(msgType);
		return (i != mObjectMsgCallbacks.end());
	}
	void GOCScriptMessageCallback::AddListener(MsgTypeID msgType, ScriptParam callback)
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