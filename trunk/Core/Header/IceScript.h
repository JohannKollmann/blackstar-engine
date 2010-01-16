#pragma once

#include "LuaScript.h"

namespace Ice
{

class ScriptSystem;

class __declspec(dllexport) Script
{
public:
	Script();
	Script(int iID, LuaScript* pScript);
	std::vector<ScriptParam> CallFunction(std::string strName, std::vector<ScriptParam> params);
	int GetID();
	std::string GetScriptName();
private:
	friend class ScriptSystem;//to access the following pointer to the lua-script
	friend class LuaScript;
	LuaScript* m_pLuaScript;
	int m_iID;
};

};