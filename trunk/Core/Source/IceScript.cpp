#include "IceScript.h"

namespace Ice
{

Script::Script(int iID, LuaScript* pScript)
{
	m_iID=iID;
	m_pLuaScript=pScript;
}

Script::Script()
{
	m_iID=-1;
	m_pLuaScript=0;
}

std::vector<ScriptParam>
Script::CallFunction(std::string strName, std::vector<ScriptParam> params)
{
	return m_pLuaScript->CallFunction(*this, strName, params);
}

int
Script::GetID(){return m_iID;}

std::string
Script::GetScriptName() const
{
	return m_pLuaScript->GetScriptName();
}

};