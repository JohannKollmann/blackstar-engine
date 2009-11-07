#ifndef __SGT_SCRIPT_SYSTEM__
#define __SGT_SCRIPT_SYSTEM__

#include "SGTScript.h"

class SGTDllExport SGTScriptSystem
{
public:
	SGTScriptSystem();
	static SGTScriptSystem& GetInstance();

	void ShareCFunction(std::string strName, SGTScriptFunction fn);
	void ShareScriptFunction(std::string strName, SGTLuaScript& script);

	SGTScript CreateInstance(std::string strFileName);
	SGTScript CreateInstance(std::string strFileName, std::vector<SGTScriptParam> params);

	static std::vector<SGTScriptParam> RunCallbackFunction(SGTScriptParam function, std::vector<SGTScriptParam> params);

	//functions for command line-style use
	std::vector<std::string> GetFunctionNames();
	void RunFunction(std::string strFunction, std::vector<SGTScriptParam> vParams);
	//void KillScript(std::string strFileName);
	void Clear();
private:
	friend class SGTScriptParam;//needs these maps for callbacks
	std::map<std::string, SGTScriptFunction> m_mCFunctions;
	std::map<std::string, SGTLuaScript*> m_mScriptFunctions;
	std::map<std::string, SGTLuaScript> m_mScripts;
	std::map<std::string, std::vector<int>> m_mScriptInstances;//hack for deletion

	int m_iCurrID;

	static std::vector<SGTScriptParam> BindCFnCallback(SGTScript &caller, std::vector<SGTScriptParam> params);
	static std::vector<SGTScriptParam> BindScriptFnCallback(SGTScript &caller, std::vector<SGTScriptParam> params);
	static std::vector<SGTScriptParam> ShareScriptFnCallback(SGTScript &caller, std::vector<SGTScriptParam> params);
	static std::vector<SGTScriptParam> LoadScriptCallback(SGTScript &caller, std::vector<SGTScriptParam> params);

	static void DummyErrorLogger(std::string strScript, int iLine, std::string strErr);//for lazy people ;)
};

#endif