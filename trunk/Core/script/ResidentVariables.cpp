#include "ResidentVariables.h"
#include <string>

ResidentManager&
ResidentManager::GetInstance()
{
	static ResidentManager r;
	return r;
}

ResidentManager::ResidentManager()
{
	SGTScriptSystem::GetInstance().ShareCFunction("ralloc", AllocCallback);
	SGTScriptSystem::GetInstance().ShareCFunction("rset", SetCallback);
	SGTScriptSystem::GetInstance().ShareCFunction("rget", GetCallback);

	SGTLoadSave::Instance().RegisterObject(&ResidentVariables::Register);

}

void
ResidentManager::BindResisToScript(ResidentVariables resis, SGTScript &script)
{
	m_mResis.insert(std::pair<int, ResidentVariables>(script.GetID(), resis));
}

ResidentVariables&
ResidentManager::GetResis(SGTScript &script)
{
	static ResidentVariables r;

	if(m_mResis.find(script.GetID())==m_mResis.end())
		return r;//found nothing
	return m_mResis.find(script.GetID())->second;
}

std::vector<SGTScriptParam>
ResidentManager::AllocCallback(SGTScript &caller, std::vector<SGTScriptParam> params)
{
	if(params.size()!=1 && params.size()!=2)
	{
		SGTLuaScript::LogError(std::string("ralloc: you gave no name"));
		return std::vector<SGTScriptParam>();
	}
	if(params[0].getType()!=SGTScriptParam::PARM_TYPE_STRING)
	{
		SGTLuaScript::LogError(std::string("ralloc: you gave no name"));
		return std::vector<SGTScriptParam>();		
	}
	std::string strScript=caller.GetScriptName();
	if(GetInstance().m_mAllocatedVars.find(strScript)!=GetInstance().m_mAllocatedVars.end())
	{
		std::map<std::string, SGTScriptParam>& mTemp=GetInstance().m_mAllocatedVars.find(strScript)->second;
		if(mTemp.find(params[0].getString())!=mTemp.end())
		{
			SGTLuaScript::LogError(std::string("ralloc: you tried to alloc the same var twice"));
			return std::vector<SGTScriptParam>();
		}
		if(params.size()==2)
			mTemp.insert(std::pair<std::string, SGTScriptParam>(params[0].getString(), params[1]));
		else
			mTemp.insert(std::pair<std::string, SGTScriptParam>(params[0].getString(), SGTScriptParam()));
	}
	else
	{
		std::map<std::string, SGTScriptParam> mTemp;
		if(params.size()==2)
			mTemp.insert(std::pair<std::string, SGTScriptParam>(params[0].getString(), params[1]));
		else
			mTemp.insert(std::pair<std::string, SGTScriptParam>(params[0].getString(), SGTScriptParam()));
		GetInstance().m_mAllocatedVars.insert(std::pair<std::string, std::map<std::string, SGTScriptParam>>(strScript, mTemp));
	}
	return std::vector<SGTScriptParam>();
}

std::vector<SGTScriptParam>
ResidentManager::SetCallback(SGTScript &caller, std::vector<SGTScriptParam> params)
{
	std::vector<SGTScriptParam> vRes;
	vRes.push_back(SGTScriptParam());//an error
	if(params.size()!=2)
	{
		vRes.push_back(SGTScriptParam(std::string("invalid number of arguments")));
		return vRes;
	}
	if(params[0].getType()!=SGTScriptParam::PARM_TYPE_STRING)
	{
		vRes.push_back(SGTScriptParam(std::string("you gave no name")));
		return vRes;
	}
	std::string strScript=caller.GetScriptName();
	if(GetInstance().m_mAllocatedVars.find(strScript)!=GetInstance().m_mAllocatedVars.end())
	{
		std::map<std::string, SGTScriptParam>& mTemp=GetInstance().m_mAllocatedVars.find(strScript)->second;
		if(mTemp.find(params[0].getString())==mTemp.end())
		{
			vRes.push_back(SGTScriptParam(std::string("tried to set a non-existing var")));
			return vRes;
		}
		else
		{
			if(GetInstance().m_mResis.find(caller.GetID())==GetInstance().m_mResis.end())
			{
				//create some temporary script-local-variables
				GetInstance().BindResisToScript(ResidentVariables(), caller);
			}
			GetInstance().m_mResis.find(caller.GetID())->second.SetVariable(params[0].getString(), params[1]);
		}
	}
	else
	{
		vRes.push_back(SGTScriptParam(std::string("tried to set a non-existing var")));
		return vRes;
	}
	return std::vector<SGTScriptParam>();
}
std::vector<SGTScriptParam>
ResidentManager::GetCallback(SGTScript &caller, std::vector<SGTScriptParam> params)
{
	if(params.size()!=1)
	{
		SGTLuaScript::LogError(std::string("rget: invalid number of arguments"));
		return std::vector<SGTScriptParam>();
	}
	if(params[0].getType()!=SGTScriptParam::PARM_TYPE_STRING)
	{
		SGTLuaScript::LogError(std::string("rget: you gave no name"));
		return std::vector<SGTScriptParam>();
	}
	std::vector<SGTScriptParam> vTemp;
	if(GetInstance().m_mResis.find(caller.GetID())==GetInstance().m_mResis.end())
	{//create some temporary script-local-variables
		GetInstance().BindResisToScript(ResidentVariables(), caller);
	}

	SGTScriptParam parm=GetInstance().m_mResis.find(caller.GetID())->second.GetVariable(params[0].getString());
	if(parm.getType()==SGTScriptParam::PARM_TYPE_NONE)
	{
		if(GetInstance().m_mAllocatedVars.find(caller.GetScriptName())!=GetInstance().m_mAllocatedVars.end())
		{
			std::map<std::string, SGTScriptParam>& mTemp=GetInstance().m_mAllocatedVars.find(caller.GetScriptName())->second;
			if(mTemp.find(params[0].getString())!=mTemp.end())
			{
				parm=mTemp.find(params[0].getString())->second;
				if(parm.getType()==SGTScriptParam::PARM_TYPE_NONE)
				{
					//4 ifs. w00t :D
					SGTLuaScript::LogError(std::string("rget: variable not set and no initial value found..."));
					return std::vector<SGTScriptParam>();
				}
			}
			else
			{
				SGTLuaScript::LogError(std::string("rget: variable not set and no initial value found..."));
				return std::vector<SGTScriptParam>();
			}
		}
		else
		{
			SGTLuaScript::LogError(std::string("rget: script has no resis allocated"));
			return std::vector<SGTScriptParam>();
		}
	}
	vTemp.push_back(parm);
	return vTemp;
}


void
ResidentVariables::Save(SGTSaveSystem &myManager)
{
	myManager.SaveAtom("std::map<std::string, ResidentVariables::SaveableScriptParam>", &m_mVars, "m_mVars");
}

void
ResidentVariables::Load(SGTLoadSystem& myManager)
{
	myManager.LoadAtom("std::map<std::string, ResidentVariables::SaveableScriptParam>", &m_mVars);
}

std::string&
ResidentVariables::TellName()
{
	static std::string strName=std::string("ResidentVariables");
	return strName;
}

SGTScriptParam
ResidentVariables::GetVariable(std::string strVar)
{
	if(m_mVars.find(strVar)==m_mVars.end())
		return SGTScriptParam();
	return SGTScriptParam(m_mVars.find(strVar)->second.ToSGTScriptParam());
}

SGTSaveable*
ResidentVariables::NewInstance()
{
	return new ResidentVariables;
}

void
ResidentVariables::Register(std::string* pstrName, SGTSaveableInstanceFn* pFn)
{
	*pstrName=std::string("ResidentVariables");
	*pFn=NewInstance;
	SGTLoadSave::Instance().RegisterObject(&ResidentVariables::SaveableScriptParam::Register);
}

void
ResidentVariables::SetVariable(std::string strVar, SGTScriptParam &param)
{
	if(m_mVars.find(strVar)==m_mVars.end())
		m_mVars.insert(std::pair<std::string, ResidentVariables::SaveableScriptParam>(strVar, ResidentVariables::SaveableScriptParam(param)));
	else
		m_mVars.find(strVar)->second.set(param);
}

ResidentVariables::SaveableScriptParam::SaveableScriptParam()
{
	m_Param=SGTScriptParam();
}

ResidentVariables::SaveableScriptParam::SaveableScriptParam(SGTScriptParam &param)
{
	m_Param=param;
}

void
ResidentVariables::SaveableScriptParam::set(SGTScriptParam &param)
{
	m_Param.set(param);
}

SGTScriptParam
ResidentVariables::SaveableScriptParam::ToSGTScriptParam()
{
	return m_Param;
}

void
ResidentVariables::SaveableScriptParam::Register(std::string *pstrName, SGTSaveableInstanceFn *pFn)
{
	*pstrName=std::string("ResidentVariables::SaveableScriptParam");
	*pFn=NewInstance;
}

SGTSaveable*
ResidentVariables::SaveableScriptParam::NewInstance()
{
	return new ResidentVariables::SaveableScriptParam;
}

std::string&
ResidentVariables::SaveableScriptParam::TellName()
{
	static std::string strName=std::string("ResidentVariables::SaveableScriptParam");
	return strName;
}

void
ResidentVariables::SaveableScriptParam::Save(SGTSaveSystem &ss)
{
	int iParamType=m_Param.getType();
	ss.SaveAtom("int", &iParamType, "m_Type");
	//some dummy vars...
	bool b;
	double f;
	int i;
	std::string str;
	switch(iParamType)
	{
	case SGTScriptParam::PARM_TYPE_BOOL:
		b=m_Param.getBool();
		ss.SaveAtom("bool", &b, "m_bData");
		break;
	case SGTScriptParam::PARM_TYPE_FLOAT:
		f=m_Param.getFloat();
		ss.SaveAtom("double", &f, "m_fData");
		break;
	case SGTScriptParam::PARM_TYPE_INT:
		i=m_Param.getInt();
		ss.SaveAtom("int", &i, "m_iData");
		break;
	case SGTScriptParam::PARM_TYPE_STRING:
		str=m_Param.getString();
		ss.SaveAtom("std::string", &str, "m_strData");
		break;
	default:
		break;
	}
}

void
ResidentVariables::SaveableScriptParam::Load(SGTLoadSystem &ls)
{
	int iType;
	ls.LoadAtom("int", &iType);
	//some dummy vars...
	bool b;
	double f;
	int i;
	std::string str;
	switch(iType)
	{
	case SGTScriptParam::PARM_TYPE_BOOL:
		ls.LoadAtom("bool", &b);
		m_Param=SGTScriptParam(b);
		break;
	case SGTScriptParam::PARM_TYPE_FLOAT:
		ls.LoadAtom("double", &f);
		m_Param=SGTScriptParam(f);
		break;
	case SGTScriptParam::PARM_TYPE_INT:
		ls.LoadAtom("int", &i);
		m_Param=SGTScriptParam(i);
		break;
	case SGTScriptParam::PARM_TYPE_STRING:
		ls.LoadAtom("std::string", &str);
		m_Param=SGTScriptParam(str);
		break;
	default:
		break;
	}
}


/*void
SGTScriptParamHandler::Save(SGTSaveSystem &ss, void *pData, std::string strVarName)
{
	SGTScriptParam& param=*(SGTScriptParam*)pData;
	int iParamType=param.getType();
	ss.SaveAtom("int", &iParamType, "m_Type");
	//some dummy vars...
	bool b;
	double f;
	int i;
	std::string str;
	switch(iParamType)
	{
	case SGTScriptParam::PARM_TYPE_BOOL:
		b=param.getBool();
		ss.SaveAtom("bool", &b, "m_bData");
		break;
	case SGTScriptParam::PARM_TYPE_FLOAT:
		f=param.getFloat();
		ss.SaveAtom("double", &f, "m_fData");
		break;
	case SGTScriptParam::PARM_TYPE_INT:
		i=param.getInt();
		ss.SaveAtom("int", &i, "m_iData");
		break;
	case SGTScriptParam::PARM_TYPE_STRING:
		str=param.getString();
		ss.SaveAtom("std::string", &str, "m_strData");
		break;
	default:
		break;
	}
}

void
SGTScriptParamHandler::Load(SGTLoadSystem &ls, void *pDest)
{
	SGTScriptParam* pParam=(SGTScriptParam*)pDest;
	int iType;
	ls.LoadAtom("int", &iType);
	//some dummy vars...
	bool b;
	double f;
	int i;
	std::string str;
	switch(iType)
	{
	case SGTScriptParam::PARM_TYPE_BOOL:
		ls.LoadAtom("bool", &b);
		pParam->set(SGTScriptParam(b));
		break;
	case SGTScriptParam::PARM_TYPE_FLOAT:
		ls.LoadAtom("double", &f);
		pParam->set(SGTScriptParam(f));
		break;
	case SGTScriptParam::PARM_TYPE_INT:
		ls.LoadAtom("int", &i);
		pParam->set(SGTScriptParam(i));
		break;
	case SGTScriptParam::PARM_TYPE_STRING:
		ls.LoadAtom("std::string", &str);
		pParam->set(SGTScriptParam(str));
		break;
	default:
		break;
	}
}

std::string&
SGTScriptParamHandler::TellName()
{
	static std::string strName=std::string("SGTScriptParam");
	return strName;
}*/