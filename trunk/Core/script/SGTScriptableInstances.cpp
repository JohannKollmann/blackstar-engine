#include "SGTScriptableInstances.h"

//singleton stuff

SGTScriptableInstances::SGTScriptableInstances()
{
}

SGTScriptableInstances&
SGTScriptableInstances::GetInstance()
{
	static SGTScriptableInstances singleton;
	return singleton;
}

void
SGTScriptableInstances::Clear()
{
	m_mScripts.clear();
}

//functionality

std::vector<SGTScriptParam>
SGTScriptableInstances::Lua_InstantiateScript(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> vRes;
	if(vParams.size()==1)
	{
		if(vParams[0].getType()!=SGTScriptParam::PARM_TYPE_STRING)
			return vRes;
		int iID=SGTSceneManager::Instance().RequestID();
		SGTScript script=SGTScriptSystem::GetInstance().CreateInstance(vParams[0].getString());
		SGTScriptableInstances::GetInstance().m_mScripts.insert(std::pair<int, SGTScript>(iID, script));
		vRes.push_back(SGTScriptParam(iID));
		return vRes;
	}
	return vRes;
}
std::vector<SGTScriptParam>
SGTScriptableInstances::Lua_RunFunction(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> vRes;
	if(vParams.size()>=2)
	{
		if(vParams[1].getType()!=SGTScriptParam::PARM_TYPE_STRING || vParams[0].getType()!=SGTScriptParam::PARM_TYPE_INT)
			return vRes;
		std::map<int, SGTScript>::iterator it=SGTScriptableInstances::GetInstance().m_mScripts.find(vParams[0].getInt());
		if(it==SGTScriptableInstances::GetInstance().m_mScripts.end())
			return vRes;
		std::string strFunction=vParams[1].getString();
		vParams.erase(vParams.begin());
		vParams.erase(vParams.begin());
		vRes=it->second.CallFunction(strFunction, vParams);
		return vRes;
	}
	return vRes;
}