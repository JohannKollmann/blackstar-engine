#include "SGTScriptParam.h"

SGTScriptParam::SGTScriptParam(){m_Type=PARM_TYPE_NONE; m_pScript=0;}
SGTScriptParam::SGTScriptParam(int i){m_Type=PARM_TYPE_INT;m_iData=i;}
SGTScriptParam::SGTScriptParam(bool b){m_Type=PARM_TYPE_BOOL;m_bData=b;}
SGTScriptParam::SGTScriptParam(double f){m_Type=PARM_TYPE_FLOAT;m_fData=f;}
SGTScriptParam::SGTScriptParam(std::string s){m_Type=PARM_TYPE_STRING;m_strData=s;}
SGTScriptParam::SGTScriptParam(std::string strFnName, SGTLuaScript& script){m_Type=PARM_TYPE_FUNCTION;m_strData=strFnName;m_pScript=&script;}

SGTScriptParam::ETypes SGTScriptParam::getType(){return m_Type;}

int SGTScriptParam::getInt(){return m_iData;}
bool SGTScriptParam::getBool(){return m_bData;}
double SGTScriptParam::getFloat(){return m_fData;}
std::string SGTScriptParam::getString(){return m_strData;}
void SGTScriptParam::getFunction(std::string& strFnName, SGTLuaScript*& pScript){strFnName=m_strData;pScript=m_pScript;}

void
SGTScriptParam::set(SGTScriptParam &param)
{
	*this=param;
}