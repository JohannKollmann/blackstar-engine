#include "SGTScript.h"

SGTScript::SGTScript(int iID, SGTLuaScript* pScript)
{
	m_iID=iID;
	m_pLuaScript=pScript;
}

std::vector<SGTScriptParam>
SGTScript::CallFunction(std::string strName, std::vector<SGTScriptParam> params)
{
	return m_pLuaScript->CallFunction(*this, strName, params);
}

int
SGTScript::GetID(){return m_iID;}

std::string
SGTScript::GetScriptName()
{
	return m_pLuaScript->GetScriptName();
}