#pragma once

extern "C"
{
	#include "lua\src\lua.h"
	#include "lua\src\lualib.h"
	#include "lua\src\lauxlib.h"
}

#include "IceScriptParam.h"
#include <vector>
#include <map>


namespace Ice
{

class Script;
class LuaScript;

typedef __declspec(dllexport) std::vector<ScriptParam> (*ScriptFunction)(Script &caller, std::vector<ScriptParam>);
typedef __declspec(dllexport) std::vector<ScriptParam> (*StaticScriptFunction)(Script &caller, LuaScript& script, std::vector<ScriptParam>);

class __declspec(dllexport) LuaScript
{
public:
	LuaScript(std::string strFile);
	LuaScript(char* pcBuffer, unsigned int nBytes, std::string strName);
	bool LoadScript(std::string strFile);
	//bool LoadScript(char* pcBuffer, unsigned int nBytes, std::string strName);
	void ShareCFunction(std::string strName, ScriptFunction fn);
	void ShareStaticCFunction(std::string strName, StaticScriptFunction fn);
	void ShareExternalFunction(std::string strShareName, std::string strInternalName, Script& script);
	std::vector<ScriptParam> CallFunction(Script &caller, std::string strName, std::vector<ScriptParam> params, Script* pShareCallInstanceHack=0);
	std::string GetScriptName() const;
	bool FunctionExists(std::string strFunction);
	static void SetLogFn(void (*logFn)(std::string, int, std::string));
	static void LogError(std::string strScript, int iLine, std::string strError);

	static void SetLoader(std::string (*pfLoader)(lua_State* pState, std::string strFileName));
protected:
	static int ApiCallback(lua_State* pState);
	bool IncludeScript(LuaScript& script);

	static std::vector<ScriptParam> GetArguments(lua_State* pState, int iStartIndex, Script& script);
	static void PutArguments(lua_State *pState, std::vector<ScriptParam> params, Script& script,  Script* pShareCallInstanceHack=0);

	lua_State* m_pState;
	std::string m_strScriptName;

	struct SExternalShare
	{
		std::string strInternalName;
		Script& script;
	};

	struct SCShare
	{
		SCShare(bool bStatic, void* pFn){bIsStatic=bStatic; fn=(ScriptFunction)pFn; fns=(StaticScriptFunction)pFn;}
		bool bIsStatic;
		ScriptFunction fn;
		StaticScriptFunction fns;
	};

	std::map<std::string, SCShare> m_mFunctions;
	std::map<std::string, SExternalShare> m_mExternalFunctions;

	static void (*m_LogFn)(std::string, int, std::string);
	static std::string (*m_pfLoader)(lua_State*, std::string);
};

};