
#include "ScriptableInstances.h"
#include "IceSceneManager.h"
#include "IceUtils.h"

//singleton stuff

ScriptableInstances::ScriptableInstances()
{
	Ice::ScriptSystem::GetInstance().ShareCFunction("InstantiateScript", Lua_InstantiateScript);
	Ice::ScriptSystem::GetInstance().ShareCFunction("RunFunction", Lua_RunFunction);
}

ScriptableInstances&
ScriptableInstances::GetInstance()
{
	static ScriptableInstances singleton;
	return singleton;
}

void
ScriptableInstances::Clear()
{
	m_mScripts.clear();
}

//functionality

std::vector<Ice::ScriptParam>
ScriptableInstances::Lua_InstantiateScript(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> vRes;
	if(vParams.size()>=1)
	{
		if(vParams[0].getType()!=Ice::ScriptParam::PARM_TYPE_STRING)
			return vRes;
		int iID=Ice::SceneManager::Instance().RequestID();
		std::string strScript=vParams[0].getString();
		vParams.erase(vParams.begin());
		Ice::Script script=Ice::ScriptSystem::GetInstance().CreateInstance(strScript, vParams);
		ScriptableInstances::GetInstance().m_mScripts.insert(std::pair<int, Ice::Script>(iID, script));
		vRes.push_back(Ice::ScriptParam(iID));
		return vRes;
	}
	return vRes; 
}
std::vector<Ice::ScriptParam>
ScriptableInstances::Lua_RunFunction(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> vRes;
	if(vParams.size()>=2)
	{
		std::vector<Ice::ScriptParam> errout(1, Ice::ScriptParam());
		std::vector<Ice::ScriptParam> vRef(1, Ice::ScriptParam(0.1));
		vRef.push_back(Ice::ScriptParam(std::string()));
		std::string strErrString=Ice::Utils::TestParameters(vParams, vRef, true);
		if(strErrString.length())
			{errout.push_back(strErrString);return errout;}
		/*if(vParams[1].getType()!=Ice::ScriptParam::PARM_TYPE_STRING || vParams[0].getType()!=Ice::ScriptParam::PARM_TYPE_FLOAT)
		{
			vRes.push_back(Ice::ScriptParam());
			vRes.push_back(Ice::ScriptParam(std::string("first or second parameter has wrong type!")));
			return vRes;
		}*/
		std::map<int, Ice::Script>::iterator it=ScriptableInstances::GetInstance().m_mScripts.find((int)vParams[0].getFloat());
		if(it==ScriptableInstances::GetInstance().m_mScripts.end())
		{
			vRes.push_back(Ice::ScriptParam());
			vRes.push_back(Ice::ScriptParam(std::string("could not find a scriptable instance with the given ID")));
			return vRes;
		}
		std::string strFunction=vParams[1].getString();
		vParams.erase(vParams.begin());
		vParams.erase(vParams.begin());
		vRes=it->second.CallFunction(strFunction, vParams);
		return vRes;
	}
	vRes.push_back(Ice::ScriptParam());
	vRes.push_back(Ice::ScriptParam(std::string("function needs >= 2 parameters!")));
	return vRes;
}