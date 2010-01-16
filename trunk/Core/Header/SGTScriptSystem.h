#ifndef __SGT_SCRIPT_SYSTEM__
#define __SGT_SCRIPT_SYSTEM__

#include "SGTScript.h"

class __declspec(dllexport) SGTScriptSystem
{
public:
	SGTScriptSystem();
	static SGTScriptSystem& GetInstance();

	void ShareCFunction(std::string strName, SGTScriptFunction fn);
	void ShareScriptFunction(std::string strName, SGTScript& script);

	SGTScript CreateInstance(std::string strFileName);
	SGTScript CreateInstance(std::string strFileName, std::vector<SGTScriptParam> params);

	static std::vector<SGTScriptParam> RunCallbackFunction(SGTScriptParam function, std::vector<SGTScriptParam> params);

	//functions for command line-style use
	std::vector<std::string> GetFunctionNames();
	void RunFunction(std::string strFunction, std::vector<SGTScriptParam> vParams);
	//void KillScript(std::string strFileName);
	void Clear();

	//hard hack for proxy calls
	std::vector<int> GetInstances(std::string strScriptName){if(m_mScriptInstances.find(strScriptName)!=m_mScriptInstances.end())return m_mScriptInstances.find(strScriptName)->second;else return std::vector<int>();}
private:
	friend class SGTScriptParam;//needs these maps for callbacks

	std::map<std::string, SGTScriptFunction> m_mCFunctions;
	std::map<std::string, SGTScript> m_mScriptFunctions;
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