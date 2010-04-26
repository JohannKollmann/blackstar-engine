#include "IceScriptParam.h"
#include "IceScriptSystem.h"

namespace Ice
{

ScriptParam::ScriptParam(){m_Type=PARM_TYPE_NONE; m_pScript=0;}
ScriptParam::ScriptParam(int i){m_Type=PARM_TYPE_INT;m_iData=i;}
ScriptParam::ScriptParam(bool b){m_Type=PARM_TYPE_BOOL;m_bData=b;}
ScriptParam::ScriptParam(double f){m_Type=PARM_TYPE_FLOAT;m_fData=f;}
ScriptParam::ScriptParam(std::string s){m_Type=PARM_TYPE_STRING;m_strData=s;}
ScriptParam::ScriptParam(std::string strFnName, Script& script){m_Type=PARM_TYPE_FUNCTION;m_strData=strFnName;m_pScript=&ScriptSystem::GetInstance().m_mScripts.find(script.GetScriptName())->second;m_iScriptID=script.GetID();}

ScriptParam::ETypes ScriptParam::getType(){return m_Type;}
bool ScriptParam::hasInt() { return (m_Type==PARM_TYPE_FLOAT || m_Type==PARM_TYPE_INT); }

int ScriptParam::getInt(){return ((m_Type==PARM_TYPE_FLOAT) ? (int)m_fData : m_iData);}
bool ScriptParam::getBool(){return m_bData;}
double ScriptParam::getFloat(){return m_fData;}
std::string ScriptParam::getString(){return m_strData;}
void ScriptParam::getFunction(std::string& strFnName, Script& script){strFnName=m_strData;script=Script(m_iScriptID, m_pScript);}

void
ScriptParam::set(ScriptParam &param)
{
	*this=param;
}

};