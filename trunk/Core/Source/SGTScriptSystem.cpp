#include "SGTScriptSystem.h"
#include <iostream>

void
SGTScriptSystem::DummyErrorLogger(std::string strScript, int iLine, std::string strErr)
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

SGTScriptSystem::SGTScriptSystem()
{
	m_iCurrID=0;
	SGTLuaScript::SetLogFn(DummyErrorLogger);
	SGTLuaScript::SetLoader(LuaFileLoader);
}

SGTScriptSystem&
SGTScriptSystem::GetInstance()
{
	static SGTScriptSystem m_Singleton;
	return m_Singleton;
}

void
SGTScriptSystem::ShareCFunction(std::string strName, SGTScriptFunction fn)
{
	m_mCFunctions.insert(std::pair<std::string, SGTScriptFunction>(strName, fn));
}

void
SGTScriptSystem::ShareScriptFunction(std::string strName, SGTScript &script)
{
	m_mScriptFunctions.insert(std::pair<std::string, SGTScript>(strName, script));
}

std::vector<SGTScriptParam>
SGTScriptSystem::BindCFnCallback(SGTScript &caller, std::vector<SGTScriptParam> params)
{
	std::vector<SGTScriptParam> vRes;
	//first test param[0]
	if(params[0].getType()!=SGTScriptParam::PARM_TYPE_STRING)
	{
		vRes.push_back(SGTScriptParam());
		vRes.push_back(SGTScriptParam(std::string("the first parameter must be a string!")));
		return vRes;
	}
	if(SGTScriptSystem::GetInstance().m_mCFunctions.find(params[0].getString())==SGTScriptSystem::GetInstance().m_mCFunctions.end())
	{
		vRes.push_back(SGTScriptParam());
		vRes.push_back(SGTScriptParam(std::string("tried to bind non-existing function \"") + params[0].getString() + std::string("\"!")));
		return vRes;
	}
	switch(params.size())
	{
	case 1:
		//bind with original name
		caller.m_pLuaScript->ShareCFunction(params[0].getString(), SGTScriptSystem::GetInstance().m_mCFunctions.find(params[0].getString())->second);
		break;
	case 2:
		//bind with alias
		if(params[1].getType()!=SGTScriptParam::PARM_TYPE_STRING)
		{
			vRes.push_back(SGTScriptParam());
			vRes.push_back(SGTScriptParam(std::string("the second parameter must be a string!")));
			return vRes;
		}
		caller.m_pLuaScript->ShareCFunction(params[1].getString(), SGTScriptSystem::GetInstance().m_mCFunctions.find(params[0].getString())->second);
		break;
	default:
		vRes.push_back(SGTScriptParam());
		vRes.push_back(SGTScriptParam(std::string("this function takes 2 arguments max!")));
	}
	return vRes;
}

std::vector<SGTScriptParam>
SGTScriptSystem::BindScriptFnCallback(SGTScript &caller, std::vector<SGTScriptParam> params)
{
	//only a copy of the share C thing
	std::vector<SGTScriptParam> vRes;

	//first test param[0]
	if(params[0].getType()!=SGTScriptParam::PARM_TYPE_STRING)
	{
		vRes.push_back(SGTScriptParam());
		return vRes;
	}
	if(SGTScriptSystem::GetInstance().m_mScriptFunctions.find(params[0].getString())==SGTScriptSystem::GetInstance().m_mScriptFunctions.end())
	{
		vRes.push_back(SGTScriptParam());
		return vRes;
	}
	switch(params.size())
	{
	case 1:
		//bind with original name
		caller.m_pLuaScript->ShareExternalFunction(params[0].getString(), params[0].getString(), SGTScriptSystem::GetInstance().m_mScriptFunctions.find(params[0].getString())->second);
		break;
	case 2:
		//bind with alias
		if(params[1].getType()!=SGTScriptParam::PARM_TYPE_STRING)
		{
			vRes.push_back(SGTScriptParam());
			return vRes;
		}
		caller.m_pLuaScript->ShareExternalFunction(params[1].getString(), params[0].getString(), SGTScriptSystem::GetInstance().m_mScriptFunctions.find(params[0].getString())->second);
		break;
	default:
		vRes.push_back(SGTScriptParam());
	}
	return vRes;
}

std::vector<SGTScriptParam>
SGTScriptSystem::ShareScriptFnCallback(SGTScript &caller, std::vector<SGTScriptParam> params)
{
	std::vector<SGTScriptParam> vRes;
	if(params[0].getType()!=SGTScriptParam::PARM_TYPE_STRING)
	{
		vRes.push_back(SGTScriptParam());
		return vRes;
	}
	SGTScriptSystem::GetInstance().ShareScriptFunction(params[0].getString(), caller);
	return vRes;
}

std::vector<SGTScriptParam>
SGTScriptSystem::LoadScriptCallback(SGTScript &caller, std::vector<SGTScriptParam> params)
{
	std::vector<SGTScriptParam> vRes;
	if(params[0].getType()!=SGTScriptParam::PARM_TYPE_STRING)
	{
		vRes.push_back(SGTScriptParam());
		return vRes;
	}
	SGTScriptSystem::GetInstance().CreateInstance(params[0].getString());
	return vRes;
}

SGTScript
SGTScriptSystem::CreateInstance(std::string strFileName, std::vector<SGTScriptParam> params)
{
	if(m_mScripts.find(strFileName)==m_mScripts.end())
	{//script was not loaded yet
		m_mScripts.insert(std::pair<std::string, SGTLuaScript>(strFileName, SGTLuaScript(strFileName)));
		SGTLuaScript& luaScript=m_mScripts.find(strFileName)->second;
		m_mScriptInstances.insert(std::pair<std::string, std::vector<int>>(strFileName, std::vector<int>(1, m_iCurrID)));
		luaScript.ShareCFunction(std::string("bindc"), BindCFnCallback);
		luaScript.ShareCFunction(std::string("bindlua"), BindScriptFnCallback);
		luaScript.ShareCFunction(std::string("sharelua"), ShareScriptFnCallback);
		luaScript.ShareCFunction(std::string("load"), LoadScriptCallback);
		SGTScript scriptInst=SGTScript(m_iCurrID++, &luaScript);
		luaScript.CallFunction(scriptInst, "init", std::vector<SGTScriptParam>());
		scriptInst.CallFunction("create", params);
		return scriptInst;
	}
	else
	{
		//just make an instance
		m_mScriptInstances.find(strFileName)->second.push_back(m_iCurrID);
		SGTScript script=SGTScript(m_iCurrID++, &m_mScripts.find(strFileName)->second);
		script.CallFunction("create", params);
		return script;
	}
}

SGTScript
SGTScriptSystem::CreateInstance(std::string strFileName)
{
	return CreateInstance(strFileName, std::vector<SGTScriptParam>());
}

std::vector<SGTScriptParam>
SGTScriptSystem::RunCallbackFunction(SGTScriptParam function, std::vector<SGTScriptParam> params)
{
	std::vector<SGTScriptParam> vRes;
	if(function.getType()!=SGTScriptParam::PARM_TYPE_FUNCTION)
	{
		vRes.push_back(SGTScriptParam());
		return vRes;
	}
	std::string strFnName;
	SGTScript script;
	function.getFunction(strFnName, script);
	return script.CallFunction(strFnName, params);
}

/*void
SGTScriptSystem::KillScript(std::string strFileName)
{
	std::map<std::string, SGTLuaScript>::const_iterator it;
	if((it=m_mScripts.find(strFileName))==m_mScripts.end())
		return;
	std::list m_lScriptInstances
	it->second.CallFunction(
	for(std::map<std::string, SGTLuaScript*>::const_iterator itFunctions=m_mScriptFunctions.begin();
		itFunctions!=m_mScriptFunctions.end(); itFunctions++)
		if(!itFunctions->second->GetScriptName().compare(strFileName))
		{
			std::map<std::string, SGTLuaScript*>::const_iterator itTemp=itFunctions;
			itFunctions--;
			m_mScriptFunctions.erase(itTemp);
		}

	m_mScripts.erase(it);
}*/

void
SGTScriptSystem::Clear()
{
	for(std::map<std::string, std::vector<int>>::const_iterator it=m_mScriptInstances.begin();
		it!=m_mScriptInstances.end(); it++)
	{
		const std::vector<int> vInstances=it->second;
		SGTLuaScript* pScript=&(m_mScripts.find(it->first)->second);
		for(unsigned int i=0; i<vInstances.size(); i++)
			SGTScript(vInstances[i], pScript).CallFunction("destruct", std::vector<SGTScriptParam>());
		SGTScript(vInstances[0], pScript).CallFunction("die", std::vector<SGTScriptParam>());
	}
	m_mScriptFunctions.clear();
	m_mScripts.clear();
}

std::vector<std::string>
SGTScriptSystem::GetFunctionNames()
{
	std::vector<std::string> vstrFunctions(m_mScriptFunctions.size());
	int i=0;
	for(std::map<std::string, SGTScript>::const_iterator it=m_mScriptFunctions.begin();it!=m_mScriptFunctions.end(); i++, it++)
		vstrFunctions[i]=it->first;
	return vstrFunctions;
}

void
SGTScriptSystem::RunFunction(std::string strFunction, std::vector<SGTScriptParam> vParams)
{
	if(m_mScriptFunctions.find(strFunction)!=m_mScriptFunctions.end())
		m_mScriptFunctions.find(strFunction)->second.CallFunction(strFunction, vParams);
}