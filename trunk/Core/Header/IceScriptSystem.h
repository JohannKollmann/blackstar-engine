#pragma once

#include "IceScript.h"
#include "IceScriptUser.h"
#include "IceIncludes.h"

namespace Ice
{
	class ScriptMessageListener;

class __declspec(dllexport) ScriptSystem
{
public:
	ScriptSystem();
	static ScriptSystem& GetInstance();

	void ShareCFunction(std::string strName, ScriptFunction fn);
	void ShareScriptFunction(std::string strName, Script& script);

	Script CreateInstance(std::string strFileName, bool callCreate = true);
	Script CreateInstance(std::string strFileName, std::vector<ScriptParam> params, bool callCreate = true);

	static std::vector<ScriptParam> RunCallbackFunction(ScriptParam function, std::vector<ScriptParam> params);

	//functions for command line-style use
	std::vector<std::string> GetFunctionNames();
	void RunFunction(std::string strFunction, std::vector<ScriptParam> vParams);
	//void KillScript(std::string strFileName);
	void Clear();

	//hard hack for proxy calls
	std::vector<int> GetInstances(std::string strScriptName){if(m_mScriptInstances.find(strScriptName)!=m_mScriptInstances.end())return m_mScriptInstances.find(strScriptName)->second;else return std::vector<int>();}

	static std::vector<ScriptParam> Lua_JoinNewsgroup(Script &caller, std::vector<ScriptParam> params);

	ScriptUser* GetScriptableObject(int scriptID);
	void RegisterScriptUser(ScriptUser *script, int scriptID);
	void UnregisterScriptUser(int scriptID);

private:
	friend class ScriptParam;//needs these maps for callbacks

	std::vector<ScriptMessageListener*> mScriptMessageListeners;

	std::map<std::string, ScriptFunction> m_mCFunctions;
	std::map<std::string, Script> m_mScriptFunctions;
	std::map<std::string, LuaScript> m_mScripts;
	std::map<std::string, std::vector<int>> m_mScriptInstances;//hack for deletion

	//Script ID - Object bindings
	std::map<int, ScriptUser*> mScriptObjectBinds;

	int m_iCurrID;

	static std::vector<ScriptParam> BindCFnCallback(Script &caller, std::vector<ScriptParam> params);
	static std::vector<ScriptParam> BindScriptFnCallback(Script &caller, std::vector<ScriptParam> params);
	static std::vector<ScriptParam> ShareScriptFnCallback(Script &caller, std::vector<ScriptParam> params);
	static std::vector<ScriptParam> LoadScriptCallback(Script &caller, std::vector<ScriptParam> params);

	static void DummyErrorLogger(std::string strScript, int iLine, std::string strErr);//for lazy people ;)
};

};