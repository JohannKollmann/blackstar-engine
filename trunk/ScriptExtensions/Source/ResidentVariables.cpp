#include "ResidentVariables.h"
#include <string>

#undef AddAtom
CREATEMAPHANDLER(std::string, "std::string", ResidentVariables::SaveableScriptParam, "ResidentVariables::SaveableScriptParam", ScriptParamMapHandler);

ResidentManager&
ResidentManager::GetInstance()
{
	static ResidentManager r;
	return r;
}

ResidentManager::ResidentManager()
{
	Ice::ScriptSystem::GetInstance().ShareCFunction("ralloc", AllocCallback);
	Ice::ScriptSystem::GetInstance().ShareCFunction("rset", SetCallback);
	Ice::ScriptSystem::GetInstance().ShareCFunction("rget", GetCallback);

	LoadSave::LoadSave::Instance().RegisterObject(&ResidentVariables::Register);
	LoadSave::LoadSave::Instance().RegisterAtom((LoadSave::AtomHandler*)new ScriptParamMapHandler);
}

void
ResidentManager::BindResisToScript(ResidentVariables resis, Ice::Script &script)
{
	m_mResis.insert(std::pair<int, ResidentVariables>(script.GetID(), resis));
}

ResidentVariables&
ResidentManager::GetResis(Ice::Script &script)
{
	static ResidentVariables r;

	if(m_mResis.find(script.GetID())==m_mResis.end())
		return r;//found nothing
	return m_mResis.find(script.GetID())->second;
}

void
ResidentManager::Clear()
{
	m_mResis.clear();
	m_mAllocatedVars.clear();
}

std::vector<Ice::ScriptParam>
ResidentManager::AllocCallback(Ice::Script &caller, std::vector<Ice::ScriptParam> params)
{
	std::vector<Ice::ScriptParam> vRes;
	vRes.push_back(Ice::ScriptParam());//an error

	if(params.size()!=1 && params.size()!=2)
	{
		vRes.push_back(Ice::ScriptParam(std::string("you gave no name")));
		return vRes;
	}
	if(params[0].getType()!=Ice::ScriptParam::PARM_TYPE_STRING)
	{
		vRes.push_back(Ice::ScriptParam(std::string("you gave no name")));
		return vRes;
	}
	std::string strScript=caller.GetScriptName();
	if(GetInstance().m_mAllocatedVars.find(strScript)!=GetInstance().m_mAllocatedVars.end())
	{
		std::map<std::string, Ice::ScriptParam>& mTemp=GetInstance().m_mAllocatedVars.find(strScript)->second;
		if(mTemp.find(params[0].getString())!=mTemp.end())
		{
			vRes.push_back(Ice::ScriptParam(std::string("you tried to alloc \"") + params[0].getString() +  std::string("\" twice")));
			return vRes;
		}
		if(params.size()==2)
			mTemp.insert(std::pair<std::string, Ice::ScriptParam>(params[0].getString(), params[1]));
		else
			mTemp.insert(std::pair<std::string, Ice::ScriptParam>(params[0].getString(), Ice::ScriptParam()));
	}
	else
	{
		std::map<std::string, Ice::ScriptParam> mTemp;
		if(params.size()==2)
			mTemp.insert(std::pair<std::string, Ice::ScriptParam>(params[0].getString(), params[1]));
		else
			mTemp.insert(std::pair<std::string, Ice::ScriptParam>(params[0].getString(), Ice::ScriptParam()));
		GetInstance().m_mAllocatedVars.insert(std::pair<std::string, std::map<std::string, Ice::ScriptParam>>(strScript, mTemp));
	}
	return std::vector<Ice::ScriptParam>();
}

std::vector<Ice::ScriptParam>
ResidentManager::SetCallback(Ice::Script &caller, std::vector<Ice::ScriptParam> params)
{
	std::vector<Ice::ScriptParam> vRes;
	vRes.push_back(Ice::ScriptParam());//an error
	if(params.size()!=2)
	{
		vRes.push_back(Ice::ScriptParam(std::string("invalid number of arguments")));
		return vRes;
	}
	if(params[0].getType()!=Ice::ScriptParam::PARM_TYPE_STRING)
	{
		vRes.push_back(Ice::ScriptParam(std::string("you gave no name")));
		return vRes;
	}
	std::string strScript=caller.GetScriptName();
	if(GetInstance().m_mAllocatedVars.find(strScript)!=GetInstance().m_mAllocatedVars.end())
	{
		std::map<std::string, Ice::ScriptParam>& mTemp=GetInstance().m_mAllocatedVars.find(strScript)->second;
		if(mTemp.find(params[0].getString())==mTemp.end())
		{
			vRes.push_back(Ice::ScriptParam(std::string("tried to set non-existing var \"") + params[0].getString() +  std::string("\"")));
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
		vRes.push_back(Ice::ScriptParam(std::string("tried to set non-existing var \"") + params[0].getString() +  std::string("\"")));
		GetInstance();
		return vRes;
	}
	return std::vector<Ice::ScriptParam>();
}
std::vector<Ice::ScriptParam>
ResidentManager::GetCallback(Ice::Script &caller, std::vector<Ice::ScriptParam> params)
{
	std::vector<Ice::ScriptParam> vRes(1, Ice::ScriptParam());
	if(params.size()!=1)
	{
		vRes.push_back(Ice::ScriptParam(std::string("invalid number of arguments")));
		return vRes;
	}
	if(params[0].getType()!=Ice::ScriptParam::PARM_TYPE_STRING)
	{
		vRes.push_back(Ice::ScriptParam(std::string("you gave no name")));
		return vRes;
	}
	std::vector<Ice::ScriptParam> vTemp;
	if(GetInstance().m_mResis.find(caller.GetID())==GetInstance().m_mResis.end())
	{//create some temporary script-local-variables
		GetInstance().BindResisToScript(ResidentVariables(), caller);
	}

	Ice::ScriptParam parm=GetInstance().m_mResis.find(caller.GetID())->second.GetVariable(params[0].getString());
	if(parm.getType()==Ice::ScriptParam::PARM_TYPE_NONE)
	{
		if(GetInstance().m_mAllocatedVars.find(caller.GetScriptName())!=GetInstance().m_mAllocatedVars.end())
		{
			std::map<std::string, Ice::ScriptParam>& mTemp=GetInstance().m_mAllocatedVars.find(caller.GetScriptName())->second;
			if(mTemp.find(params[0].getString())!=mTemp.end())
			{
				parm=mTemp.find(params[0].getString())->second;
				if(parm.getType()==Ice::ScriptParam::PARM_TYPE_NONE)
				{
					//4 ifs. w00t :D
					vRes.push_back(Ice::ScriptParam(std::string("variable \"") + params[0].getString() +  std::string("\" not set and no initial value found...")));
					return vRes;
				}
			}
			else
			{
				vRes.push_back(Ice::ScriptParam(std::string("variable \"") + params[0].getString() +  std::string("\" not set and no initial value found...")));
				return vRes;
			}
		}
		else
		{
			vRes.push_back(Ice::ScriptParam(std::string("script has no resis allocated")));
			return vRes;
		}
	}
	vTemp.push_back(parm);
	return vTemp;
}


void
ResidentVariables::Save(LoadSave::SaveSystem &myManager)
{
	myManager.SaveAtom("std::map<std::string, ResidentVariables::SaveableScriptParam>", &m_mVars, "m_mVars");
}

void
ResidentVariables::Load(LoadSave::LoadSystem& myManager)
{
	myManager.LoadAtom("std::map<std::string, ResidentVariables::SaveableScriptParam>", &m_mVars);
}

std::string&
ResidentVariables::TellName()
{
	static std::string strName=std::string("ResidentVariables");
	return strName;
}

Ice::ScriptParam
ResidentVariables::GetVariable(std::string strVar)
{
	if(m_mVars.find(strVar)==m_mVars.end())
		return Ice::ScriptParam();
	return Ice::ScriptParam(m_mVars.find(strVar)->second.ToIceScriptParam());
}

LoadSave::Saveable*
ResidentVariables::NewInstance()
{
	return new ResidentVariables;
}

void
ResidentVariables::Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn)
{
	*pstrName=std::string("ResidentVariables");
	*pFn=NewInstance;
	LoadSave::LoadSave::Instance().RegisterObject(&ResidentVariables::SaveableScriptParam::Register);
}

void
ResidentVariables::SetVariable(std::string strVar, Ice::ScriptParam &param)
{
	if(m_mVars.find(strVar)==m_mVars.end())
		m_mVars.insert(std::pair<std::string, ResidentVariables::SaveableScriptParam>(strVar, ResidentVariables::SaveableScriptParam(param)));
	else
		m_mVars.find(strVar)->second.set(param);
}

ResidentVariables::SaveableScriptParam::SaveableScriptParam()
{
	m_Param=Ice::ScriptParam();
}

ResidentVariables::SaveableScriptParam::SaveableScriptParam(Ice::ScriptParam &param)
{
	m_Param=param;
}

void
ResidentVariables::SaveableScriptParam::set(Ice::ScriptParam &param)
{
	m_Param.set(param);
}

Ice::ScriptParam
ResidentVariables::SaveableScriptParam::ToIceScriptParam()
{
	return m_Param;
}

void
ResidentVariables::SaveableScriptParam::Register(std::string *pstrName, LoadSave::SaveableInstanceFn *pFn)
{
	*pstrName=std::string("ResidentVariables::SaveableScriptParam");
	*pFn=NewInstance;
}

LoadSave::Saveable*
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
ResidentVariables::SaveableScriptParam::Save(LoadSave::SaveSystem &ss)
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
	case Ice::ScriptParam::PARM_TYPE_BOOL:
		b=m_Param.getBool();
		ss.SaveAtom("bool", &b, "m_bData");
		break;
	case Ice::ScriptParam::PARM_TYPE_FLOAT:
		f=m_Param.getFloat();
		ss.SaveAtom("double", &f, "m_fData");
		break;
	case Ice::ScriptParam::PARM_TYPE_INT:
		i=m_Param.getInt();
		ss.SaveAtom("int", &i, "m_iData");
		break;
	case Ice::ScriptParam::PARM_TYPE_STRING:
		str=m_Param.getString();
		ss.SaveAtom("std::string", &str, "m_strData");
		break;
	default:
		break;
	}
}

void
ResidentVariables::SaveableScriptParam::Load(LoadSave::LoadSystem &ls)
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
	case Ice::ScriptParam::PARM_TYPE_BOOL:
		ls.LoadAtom("bool", &b);
		m_Param=Ice::ScriptParam(b);
		break;
	case Ice::ScriptParam::PARM_TYPE_FLOAT:
		ls.LoadAtom("double", &f);
		m_Param=Ice::ScriptParam(f);
		break;
	case Ice::ScriptParam::PARM_TYPE_INT:
		ls.LoadAtom("int", &i);
		m_Param=Ice::ScriptParam(i);
		break;
	case Ice::ScriptParam::PARM_TYPE_STRING:
		ls.LoadAtom("std::string", &str);
		m_Param=Ice::ScriptParam(str);
		break;
	default:
		break;
	}
}


/*void
Ice::ScriptParamHandler::Save(LoadSave::SaveSystem &ss, void *pData, std::string strVarName)
{
	Ice::ScriptParam& param=*(Ice::ScriptParam*)pData;
	int iParamType=param.getType();
	ss.SaveAtom("int", &iParamType, "m_Type");
	//some dummy vars...
	bool b;
	double f;
	int i;
	std::string str;
	switch(iParamType)
	{
	case Ice::ScriptParam::PARM_TYPE_BOOL:
		b=param.getBool();
		ss.SaveAtom("bool", &b, "m_bData");
		break;
	case Ice::ScriptParam::PARM_TYPE_FLOAT:
		f=param.getFloat();
		ss.SaveAtom("double", &f, "m_fData");
		break;
	case Ice::ScriptParam::PARM_TYPE_INT:
		i=param.getInt();
		ss.SaveAtom("int", &i, "m_iData");
		break;
	case Ice::ScriptParam::PARM_TYPE_STRING:
		str=param.getString();
		ss.SaveAtom("std::string", &str, "m_strData");
		break;
	default:
		break;
	}
}

void
Ice::ScriptParamHandler::Load(LoadSave::LoadSystem &ls, void *pDest)
{
	Ice::ScriptParam* pParam=(Ice::ScriptParam*)pDest;
	int iType;
	ls.LoadAtom("int", &iType);
	//some dummy vars...
	bool b;
	double f;
	int i;
	std::string str;
	switch(iType)
	{
	case Ice::ScriptParam::PARM_TYPE_BOOL:
		ls.LoadAtom("bool", &b);
		pParam->set(Ice::ScriptParam(b));
		break;
	case Ice::ScriptParam::PARM_TYPE_FLOAT:
		ls.LoadAtom("double", &f);
		pParam->set(Ice::ScriptParam(f));
		break;
	case Ice::ScriptParam::PARM_TYPE_INT:
		ls.LoadAtom("int", &i);
		pParam->set(Ice::ScriptParam(i));
		break;
	case Ice::ScriptParam::PARM_TYPE_STRING:
		ls.LoadAtom("std::string", &str);
		pParam->set(Ice::ScriptParam(str));
		break;
	default:
		break;
	}
}

std::string&
Ice::ScriptParamHandler::TellName()
{
	static std::string strName=std::string("Ice::ScriptParam");
	return strName;
}*/
