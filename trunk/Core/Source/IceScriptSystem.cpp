#include "IceScriptSystem.h"
#include <iostream>

namespace Ice
{

void
ScriptSystem::DummyErrorLogger(std::string strScript, int iLine, std::string strErr)
{
	std::cout<<strErr;
	std::cout<<'\n';
}

std::string
LuaFileLoader(lua_State* pState, std::string strFile)
{
	if(luaL_loadfile(pState, strFile.c_str()) || lua_pcall(pState, 0, 0, 0))
	{
		std::string strError(lua_tostring(pState, -1));
		return std::string(lua_tostring(pState, -1));
	}
	return std::string();
}

ScriptSystem::ScriptSystem()
{
	m_iCurrID=0;
	LuaScript::SetLogFn(DummyErrorLogger);
	LuaScript::SetLoader(LuaFileLoader);
}

ScriptSystem&
ScriptSystem::GetInstance()
{
	static ScriptSystem m_Singleton;
	return m_Singleton;
}

void
ScriptSystem::ShareCFunction(std::string strName, ScriptFunction fn)
{
	m_mCFunctions.insert(std::pair<std::string, ScriptFunction>(strName, fn));
}

void
ScriptSystem::ShareScriptFunction(std::string strName, Script &script)
{
	m_mScriptFunctions.insert(std::pair<std::string, Script>(strName, script));
}

std::vector<ScriptParam>
ScriptSystem::BindCFnCallback(Script &caller, std::vector<ScriptParam> params)
{
	std::vector<ScriptParam> vRes;
	//first test param[0]
	if(params[0].getType()!=ScriptParam::PARM_TYPE_STRING)
	{
		vRes.push_back(ScriptParam());
		vRes.push_back(ScriptParam(std::string("the first parameter must be a string!")));
		return vRes;
	}
	if(ScriptSystem::GetInstance().m_mCFunctions.find(params[0].getString())==ScriptSystem::GetInstance().m_mCFunctions.end())
	{
		vRes.push_back(ScriptParam());
		vRes.push_back(ScriptParam(std::string("tried to bind non-existing function \"") + params[0].getString() + std::string("\"!")));
		return vRes;
	}
	switch(params.size())
	{
	case 1:
		//bind with original name
		caller.m_pLuaScript->ShareCFunction(params[0].getString(), ScriptSystem::GetInstance().m_mCFunctions.find(params[0].getString())->second);
		break;
	case 2:
		//bind with alias
		if(params[1].getType()!=ScriptParam::PARM_TYPE_STRING)
		{
			vRes.push_back(ScriptParam());
			vRes.push_back(ScriptParam(std::string("the second parameter must be a string!")));
			return vRes;
		}
		caller.m_pLuaScript->ShareCFunction(params[1].getString(), ScriptSystem::GetInstance().m_mCFunctions.find(params[0].getString())->second);
		break;
	default:
		vRes.push_back(ScriptParam());
		vRes.push_back(ScriptParam(std::string("this function takes 2 arguments max!")));
	}
	return vRes;
}

std::vector<ScriptParam>
ScriptSystem::BindScriptFnCallback(Script &caller, std::vector<ScriptParam> params)
{
	//only a copy of the share C thing
	std::vector<ScriptParam> vRes;

	//first test param[0]
	if(params[0].getType()!=ScriptParam::PARM_TYPE_STRING)
	{
		vRes.push_back(ScriptParam());
		return vRes;
	}
	if(ScriptSystem::GetInstance().m_mScriptFunctions.find(params[0].getString())==ScriptSystem::GetInstance().m_mScriptFunctions.end())
	{
		vRes.push_back(ScriptParam());
		return vRes;
	}
	switch(params.size())
	{
	case 1:
		//bind with original name
		caller.m_pLuaScript->ShareExternalFunction(params[0].getString(), params[0].getString(), ScriptSystem::GetInstance().m_mScriptFunctions.find(params[0].getString())->second);
		break;
	case 2:
		//bind with alias
		if(params[1].getType()!=ScriptParam::PARM_TYPE_STRING)
		{
			vRes.push_back(ScriptParam());
			return vRes;
		}
		caller.m_pLuaScript->ShareExternalFunction(params[1].getString(), params[0].getString(), ScriptSystem::GetInstance().m_mScriptFunctions.find(params[0].getString())->second);
		break;
	default:
		vRes.push_back(ScriptParam());
	}
	return vRes;
}

std::vector<ScriptParam>
ScriptSystem::ShareScriptFnCallback(Script &caller, std::vector<ScriptParam> params)
{
	std::vector<ScriptParam> vRes;
	if(params[0].getType()!=ScriptParam::PARM_TYPE_STRING)
	{
		vRes.push_back(ScriptParam());
		return vRes;
	}
	ScriptSystem::GetInstance().ShareScriptFunction(params[0].getString(), caller);
	return vRes;
}

std::vector<ScriptParam>
ScriptSystem::LoadScriptCallback(Script &caller, std::vector<ScriptParam> params)
{
	std::vector<ScriptParam> vRes;
	if(params[0].getType()!=ScriptParam::PARM_TYPE_STRING)
	{
		vRes.push_back(ScriptParam());
		return vRes;
	}
	ScriptSystem::GetInstance().CreateInstance(params[0].getString());
	return vRes;
}

Script
ScriptSystem::CreateInstance(std::string strFileName, std::vector<ScriptParam> params)
{
	if(m_mScripts.find(strFileName)==m_mScripts.end())
	{//script was not loaded yet
		m_mScripts.insert(std::pair<std::string, LuaScript>(strFileName, LuaScript(strFileName)));
		LuaScript& luaScript=m_mScripts.find(strFileName)->second;
		m_mScriptInstances.insert(std::pair<std::string, std::vector<int>>(strFileName, std::vector<int>(1, m_iCurrID)));
		luaScript.ShareCFunction(std::string("bindc"), BindCFnCallback);
		luaScript.ShareCFunction(std::string("bindlua"), BindScriptFnCallback);
		luaScript.ShareCFunction(std::string("sharelua"), ShareScriptFnCallback);
		luaScript.ShareCFunction(std::string("load"), LoadScriptCallback);
		Script scriptInst=Script(m_iCurrID++, &luaScript);
		luaScript.CallFunction(scriptInst, "init", std::vector<ScriptParam>());
		scriptInst.CallFunction("create", params);
		return scriptInst;
	}
	else
	{
		//just make an instance
		m_mScriptInstances.find(strFileName)->second.push_back(m_iCurrID);
		Script script=Script(m_iCurrID++, &m_mScripts.find(strFileName)->second);
		script.CallFunction("create", params);
		return script;
	}
}

Script
ScriptSystem::CreateInstance(std::string strFileName)
{
	return CreateInstance(strFileName, std::vector<ScriptParam>());
}

std::vector<ScriptParam>
ScriptSystem::RunCallbackFunction(ScriptParam function, std::vector<ScriptParam> params)
{
	std::vector<ScriptParam> vRes;
	if(function.getType()!=ScriptParam::PARM_TYPE_FUNCTION)
	{
		vRes.push_back(ScriptParam());
		return vRes;
	}
	std::string strFnName;
	Script script;
	function.getFunction(strFnName, script);
	return script.CallFunction(strFnName, params);
}

/*void
ScriptSystem::KillScript(std::string strFileName)
{
	std::map<std::string, LuaScript>::const_iterator it;
	if((it=m_mScripts.find(strFileName))==m_mScripts.end())
		return;
	std::list m_lScriptInstances
	it->second.CallFunction(
	for(std::map<std::string, LuaScript*>::const_iterator itFunctions=m_mScriptFunctions.begin();
		itFunctions!=m_mScriptFunctions.end(); itFunctions++)
		if(!itFunctions->second->GetScriptName().compare(strFileName))
		{
			std::map<std::string, LuaScript*>::const_iterator itTemp=itFunctions;
			itFunctions--;
			m_mScriptFunctions.erase(itTemp);
		}

	m_mScripts.erase(it);
}*/

void
ScriptSystem::Clear()
{
	for(std::map<std::string, std::vector<int>>::const_iterator it=m_mScriptInstances.begin();
		it!=m_mScriptInstances.end(); it++)
	{
		const std::vector<int> vInstances=it->second;
		LuaScript* pScript=&(m_mScripts.find(it->first)->second);
		for(unsigned int i=0; i<vInstances.size(); i++)
			Script(vInstances[i], pScript).CallFunction("destruct", std::vector<ScriptParam>());
		Script(vInstances[0], pScript).CallFunction("die", std::vector<ScriptParam>());
	}
	m_mScriptFunctions.clear();
	m_mScripts.clear();
}

std::vector<std::string>
ScriptSystem::GetFunctionNames()
{
	std::vector<std::string> vstrFunctions(m_mScriptFunctions.size());
	int i=0;
	for(std::map<std::string, Script>::const_iterator it=m_mScriptFunctions.begin();it!=m_mScriptFunctions.end(); i++, it++)
		vstrFunctions[i]=it->first;
	return vstrFunctions;
}

void
ScriptSystem::RunFunction(std::string strFunction, std::vector<ScriptParam> vParams)
{
	if(m_mScriptFunctions.find(strFunction)!=m_mScriptFunctions.end())
		m_mScriptFunctions.find(strFunction)->second.CallFunction(strFunction, vParams);
}

};