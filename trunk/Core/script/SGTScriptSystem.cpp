#include "SGTScriptSystem.h"
#include <iostream>

void
SGTScriptSystem::DummyErrorLogger(std::string strErr)
{
	std::cout<<strErr;
	std::cout<<'\n';
}

SGTScriptSystem::SGTScriptSystem()
{
	m_iCurrID=0;
	SGTLuaScript::SetLogFn(DummyErrorLogger);
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
SGTScriptSystem::ShareScriptFunction(std::string strName, SGTLuaScript &script)
{
	m_mScriptFunctions.insert(std::pair<std::string, SGTLuaScript*>(strName, &script));
}

std::vector<SGTScriptParam>
SGTScriptSystem::BindCFnCallback(SGTScript &caller, std::vector<SGTScriptParam> params)
{
	std::vector<SGTScriptParam> vRes;
	//first test param[0]
	if(params[0].getType()!=SGTScriptParam::PARM_TYPE_STRING)
	{
		vRes.push_back(SGTScriptParam());
		return vRes;
	}
	if(SGTScriptSystem::GetInstance().m_mCFunctions.find(params[0].getString())==SGTScriptSystem::GetInstance().m_mCFunctions.end())
	{
		vRes.push_back(SGTScriptParam());
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
			return vRes;
		}
		caller.m_pLuaScript->ShareCFunction(params[1].getString(), SGTScriptSystem::GetInstance().m_mCFunctions.find(params[0].getString())->second);
		break;
	default:
		vRes.push_back(SGTScriptParam());
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
		caller.m_pLuaScript->ShareExternalFunction(params[0].getString(), params[0].getString(), *SGTScriptSystem::GetInstance().m_mScriptFunctions.find(params[0].getString())->second);
		break;
	case 2:
		//bind with alias
		if(params[1].getType()!=SGTScriptParam::PARM_TYPE_STRING)
		{
			vRes.push_back(SGTScriptParam());
			return vRes;
		}
		caller.m_pLuaScript->ShareExternalFunction(params[1].getString(), params[0].getString(), *SGTScriptSystem::GetInstance().m_mScriptFunctions.find(params[0].getString())->second);
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
	SGTScriptSystem::GetInstance().ShareScriptFunction(params[0].getString(), *caller.m_pLuaScript);
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
SGTScriptSystem::CreateInstance(std::string strFileName)
{
	if(m_mScripts.find(strFileName)==m_mScripts.end())
	{//script was not loaded yet
		m_mScripts.insert(std::pair<std::string, SGTLuaScript>(strFileName, SGTLuaScript(strFileName)));
		SGTLuaScript& luaScript=m_mScripts.find(strFileName)->second;
		luaScript.ShareCFunction(std::string("bindc"), BindCFnCallback);
		luaScript.ShareCFunction(std::string("bindlua"), BindScriptFnCallback);
		luaScript.ShareCFunction(std::string("sharelua"), ShareScriptFnCallback);
		luaScript.ShareCFunction(std::string("load"), LoadScriptCallback);
		SGTScript scriptInst=SGTScript(m_iCurrID++, &luaScript);
		luaScript.CallFunction(scriptInst, "init", std::vector<SGTScriptParam>());
		scriptInst.CallFunction("create", std::vector<SGTScriptParam>());
		return scriptInst;
	}
	else
	{
		//just make an instance
		SGTScript script=SGTScript(m_iCurrID++, &m_mScripts.find(strFileName)->second);
		script.CallFunction("create", std::vector<SGTScriptParam>());
		return script;
	}
}

void
SGTScriptSystem::KillScript(std::string strFileName)
{
	std::map<std::string, SGTLuaScript>::const_iterator it;
	if((it=m_mScripts.find(strFileName))==m_mScripts.end())
		return;
	for(std::map<std::string, SGTLuaScript*>::const_iterator itFunctions=m_mScriptFunctions.begin();
		itFunctions!=m_mScriptFunctions.end(); itFunctions++)
		if(!itFunctions->second->GetScriptName().compare(strFileName))
		{
			std::map<std::string, SGTLuaScript*>::const_iterator itTemp=itFunctions;
			itFunctions--;
			m_mScriptFunctions.erase(itTemp);
		}

	m_mScripts.erase(it);
}

void
SGTScriptSystem::Clear()
{
	m_mScriptFunctions.clear();
	m_mScripts.clear();
}