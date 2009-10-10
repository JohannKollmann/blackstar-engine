#ifndef __SGT_SCRIPT_SYSTEM__
#define __SGT_SCRIPT_SYSTEM__

#include "SGTScript.h"
#include "SGTIncludes.h"

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

	void KillScript(std::string strFileName);
	void Clear();
private:
	friend class SGTScriptParam;//needs these maps for callbacks
	std::map<std::string, SGTScriptFunction> m_mCFunctions;
	std::map<std::string, SGTLuaScript*> m_mScriptFunctions;
	std::map<std::string, SGTLuaScript> m_mScripts;

	int m_iCurrID;

	static std::vector<SGTScriptParam> BindCFnCallback(SGTScript &caller, std::vector<SGTScriptParam> params);
	static std::vector<SGTScriptParam> BindScriptFnCallback(SGTScript &caller, std::vector<SGTScriptParam> params);
	static std::vector<SGTScriptParam> ShareScriptFnCallback(SGTScript &caller, std::vector<SGTScriptParam> params);
	static std::vector<SGTScriptParam> LoadScriptCallback(SGTScript &caller, std::vector<SGTScriptParam> params);

	static void DummyErrorLogger(std::string strErr);//for lazy people ;)
};

#endif