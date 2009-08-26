#ifndef __SGT_SCRIPT__
#define __SGT_SCRIPT__

#include "LuaScript.h"
#include "SGTIncludes.h"

class SGTDllExport SGTScriptSystem;

class SGTScript
{
public:
	SGTScript();
	SGTScript(int iID, SGTLuaScript* pScript);
	std::vector<SGTScriptParam> CallFunction(std::string strName, std::vector<SGTScriptParam> params);
	int GetID();
	std::string GetScriptName();
private:
	friend class SGTScriptSystem;//to access the following pointer to the lua-script
	SGTLuaScript* m_pLuaScript;
	int m_iID;
};

#endif