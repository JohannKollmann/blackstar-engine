#ifndef __SGT_LUA_SCRIPT__
#define __SGT_LUA_SCRIPT__

extern "C"
{
	#include "lua\src\lua.h"
	#include "lua\src\lualib.h"
	#include "lua\src\lauxlib.h"
}

#include "SGTScriptParam.h"
#include <vector>
#include <map>

class SGTScript;
class SGTLuaScript;

typedef __declspec(dllexport) std::vector<SGTScriptParam> (*SGTScriptFunction)(SGTScript &caller, std::vector<SGTScriptParam>);
typedef __declspec(dllexport) std::vector<SGTScriptParam> (*SGTStaticScriptFunction)(SGTScript &caller, SGTLuaScript& script, std::vector<SGTScriptParam>);

class __declspec(dllexport) SGTLuaScript
{
public:
	SGTLuaScript(std::string strFile);
	SGTLuaScript(char* pcBuffer, unsigned int nBytes, std::string strName);
	bool LoadScript(std::string strFile);
	//bool LoadScript(char* pcBuffer, unsigned int nBytes, std::string strName);
	void ShareCFunction(std::string strName, SGTScriptFunction fn);
	void ShareStaticCFunction(std::string strName, SGTStaticScriptFunction fn);
	void ShareExternalFunction(std::string strShareName, std::string strInternalName, SGTLuaScript& script);
	std::vector<SGTScriptParam> CallFunction(SGTScript &caller, std::string strName, std::vector<SGTScriptParam> params);
	std::string GetScriptName();
	bool FunctionExists(std::string strFunction);
	static void SetLogFn(void (*logFn)(std::string, int, std::string));
	static void LogError(std::string strScript, int iLine, std::string strError);

	static void SetLoader(std::string (*pfLoader)(lua_State* pState, std::string strFileName));
protected:
	static int ApiCallback(lua_State* pState);
	bool IncludeScript(SGTLuaScript& script);

	static std::vector<SGTScriptParam> GetArguments(lua_State* pState, int iStartIndex, SGTScript& script);
	static void PutArguments(lua_State *pState, std::vector<SGTScriptParam> params, SGTScript& script);

	lua_State* m_pState;
	std::string m_strScriptName;

	struct SExternalShare
	{
		std::string strInternalName;
		SGTLuaScript& script;
	};

	struct SCShare
	{
		SCShare(bool bStatic, void* pFn){bIsStatic=bStatic; fn=(SGTScriptFunction)pFn; fns=(SGTStaticScriptFunction)pFn;}
		bool bIsStatic;
		SGTScriptFunction fn;
		SGTStaticScriptFunction fns;
	};

	std::map<std::string, SCShare> m_mFunctions;
	std::map<std::string, SExternalShare> m_mExternalFunctions;

	static void (*m_LogFn)(std::string, int, std::string);
	static std::string (*m_pfLoader)(lua_State*, std::string);
};

#endif