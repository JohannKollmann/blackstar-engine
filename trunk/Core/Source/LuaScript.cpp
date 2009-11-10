#include "LuaScript.h"
#include "SGTScriptSystem.h"
#include <sstream>

void (*SGTLuaScript::m_LogFn)(std::string, int, std::string) =0;
std::string (*SGTLuaScript::m_pfLoader)(lua_State*, std::string) =0;

//handler for transparent function calls across VMs
class CLongCallHandler
{
public:
	CLongCallHandler();
	static CLongCallHandler& GetInstance();
	void RegisterFunction(int iScriptID, std::string strFunction, SGTScript script, std::string strTargetFunction);
	std::pair<SGTScript, std::string> GetTarget(int iScriptID, std::string strFunction);
	std::string TargetSet(int iScriptID, int iTargetScriptID, std::string strTargetFunction);
	std::map<std::string, std::pair<SGTScript, std::string>>::const_iterator GetShares(int iScriptID);
	std::map<std::string, std::pair<SGTScript, std::string>>::const_iterator GetSharesEnd(int iScriptID);
private:
	std::map<int, std::map<std::string, std::pair<SGTScript, std::string>>> m_mRegisteredFunctions;
};

SGTLuaScript::SGTLuaScript(std::string strFile)
{
	LoadScript(strFile);
}
/*
SGTLuaScript::SGTLuaScript(char* pcBuffer, unsigned int nBytes, std::string strName)
{
	LoadScript(pcBuffer, nBytes, strName);
}*/

bool
SGTLuaScript::LoadScript(std::string strFile)
{
	m_strScriptName=strFile;
	m_pState=luaL_newstate();
	luaopen_base(m_pState);
	luaopen_string(m_pState);
/*
	if(luaL_loadfile(m_pState, strFile.c_str()) || lua_pcall(m_pState, 0, 0, 0))
	{
		std::string strError(lua_tostring(m_pState, -1));
		LogError(std::string("error opening ") + strFile + std::string(": ") + strError);
		return false;
	}*/
	std::string strErr=m_pfLoader(m_pState, strFile);
	if(strErr.length())
	{
		LogError(strFile, -1, strErr);
		return false;
	}
	return true;
}
/*
bool
SGTLuaScript::LoadScript(char* pcBuffer, unsigned int nBytes, std::string strName)
{
	m_strScriptName=strName;
	m_pState=luaL_newstate();
	luaopen_base(m_pState);
	luaopen_string(m_pState);

	if(luaL_loadbuffer(m_pState, pcBuffer, nBytes, strName.c_str()) || lua_pcall(m_pState, 0, 0, 0))
	{
		std::string strError(lua_tostring(m_pState, -1));
		LogError(std::string("error opening ") + m_strScriptName + std::string(": ") + strError);
		return false;
	}
	return true;
}*/

void
SGTLuaScript::ShareCFunction(std::string strName, SGTScriptFunction fn)
{
	m_mFunctions.insert(std::pair<std::string, SCShare>(strName, SCShare(false, fn)));
	lua_register(m_pState, strName.c_str(), &SGTLuaScript::ApiCallback);
}

void
SGTLuaScript::ShareStaticCFunction(std::string strName, SGTStaticScriptFunction fn)
{
	m_mFunctions.insert(std::pair<std::string, SCShare>(strName, SCShare(true, fn)));
	lua_register(m_pState, strName.c_str(), &SGTLuaScript::ApiCallback);
}

void
SGTLuaScript::ShareExternalFunction(std::string strShareName, std::string strInternalName, SGTLuaScript &script)
{
	SExternalShare share={strInternalName, script};
	m_mExternalFunctions.insert(std::pair<std::string, SExternalShare>(strShareName, share));
	lua_register(m_pState, strShareName.c_str(), &SGTLuaScript::ApiCallback);
}

std::vector<SGTScriptParam>
SGTLuaScript::GetArguments(lua_State* pState, int iStartIndex, SGTScript& script)
{
	int nParams=lua_gettop(pState);
	std::vector<SGTScriptParam> vParams;
	for(int iParam=iStartIndex; iParam<=nParams; iParam++)
		switch(lua_type(pState, iParam))
		{
		case LUA_TNUMBER:
			vParams.push_back(SGTScriptParam(lua_tonumber(pState, iParam)));
			break;
		case LUA_TBOOLEAN:
			vParams.push_back(SGTScriptParam((bool)(lua_toboolean(pState, iParam)!=0)));
			break;
		case LUA_TSTRING:
		{
			std::string str(lua_tostring(pState, iParam));
			vParams.push_back(SGTScriptParam(str));
			break;
		}
		case LUA_TFUNCTION:
		{
			bool bFinished=false;
			lua_pushnil(pState);  /* first key */
			while(lua_next(pState, LUA_GLOBALSINDEX) != 0)
			{
				//function name is at -2, function at -1
				if(lua_equal(pState, -1, iParam))
				{//found the function
					vParams.push_back(SGTScriptParam(std::string(lua_tostring(pState, -2)), script));
					bFinished=true;
					break;
				}
				//kill function. leaves fn name for further iteration.
				lua_pop(pState, 1);
			}
			if(bFinished)
			{
				break;//the function is already in the args
			}
			else
			{
				//this must be a local function
				//find a free name for the function
				int iCounter=0;
				std::stringstream strs;
				lua_getglobal(pState, "_SGTLuaScriptFunction");
				std::string strCounter;
				while(lua_isfunction(pState, -1))
				{
					lua_pop(pState, 1);
					strs.clear();
					strs<<(iCounter++);
					strs>>strCounter;
					lua_getglobal(pState, (std::string("_SGTLuaScriptFunction") + strCounter).c_str());
				}
				lua_pop(pState, 1);
				lua_pushvalue(pState, iParam);
				lua_setglobal(pState, (std::string("_SGTLuaScriptFunction") + strCounter).c_str());
				vParams.push_back(SGTScriptParam(std::string("_SGTLuaScriptFunction") + strCounter, script));
			}
			break;
		}
		default:
			//unsupported type
			vParams.clear();
			vParams.push_back(SGTScriptParam());
			vParams.push_back(SGTScriptParam(std::string("unsupported type while extracting params")));
			return vParams;
		};
	return vParams;
}


void
SGTLuaScript::PutArguments(lua_State *pState, std::vector<SGTScriptParam> params, SGTScript& script)
{
	//push the parameters
	for(unsigned int iParam=0; iParam<params.size(); iParam++)
	{
		switch(params[iParam].getType())
		{
		case SGTScriptParam::PARM_TYPE_INT:
			lua_pushinteger(pState, params[iParam].getInt());
			break;
		case SGTScriptParam::PARM_TYPE_BOOL:
			lua_pushboolean(pState, params[iParam].getBool());
			break;
		case SGTScriptParam::PARM_TYPE_STRING:
			lua_pushstring(pState, params[iParam].getString().c_str());
			break;
		case SGTScriptParam::PARM_TYPE_FLOAT:
			lua_pushnumber(pState, params[iParam].getFloat());
			break;
		case SGTScriptParam::PARM_TYPE_FUNCTION:
		{	//check if we have already added that function
			std::string strTargetFnName;
			SGTScript targetscript;
			params[iParam].getFunction(strTargetFnName, targetscript);
			if(!CLongCallHandler::GetInstance().TargetSet(script.GetID(), targetscript.GetID(), strTargetFnName).size())
			{//create the function
				//same code as in GetArguments
				int iCounter=0;
				std::stringstream strs;
				lua_getglobal(pState, "_SGTLuaScriptFunctionCall");
				std::string strCounter;
				while(lua_isfunction(pState, -1))
				{
					lua_pop(pState, 1);
					strs.clear();
					strs<<(iCounter++);
					strs>>strCounter;
					lua_getglobal(pState, (std::string("_SGTLuaScriptFunctionCall") + strCounter).c_str());
				}
				lua_pop(pState, 1);
				lua_register(pState, (std::string("_SGTLuaScriptFunctionCall") + strCounter).c_str(), &SGTLuaScript::ApiCallback);
				//put it into the map
				CLongCallHandler::GetInstance().RegisterFunction(script.GetID(), std::string("_SGTLuaScriptFunctionCall") + strCounter, targetscript, strTargetFnName);
			}
			lua_getglobal(pState, CLongCallHandler::GetInstance().TargetSet(script.GetID(), targetscript.GetID(), strTargetFnName).c_str());
			break;
		}
		case SGTScriptParam::PARM_TYPE_NONE:
			lua_pushnil(pState);
			break;
		}
	}
}

bool
SGTLuaScript::FunctionExists(std::string strFunction)
{
	lua_getglobal(m_pState, strFunction.c_str());
	bool bRes=lua_isfunction(m_pState, -1);
	lua_pop(m_pState, 1);
	return bRes;
}

int GetLuaLine(lua_State* pState)
{
	lua_Debug ar;
	lua_getstack(pState, 1, &ar);
	lua_getinfo(pState, "l", &ar);
	return ar.currentline;
}

//will return the inputs
std::vector<SGTScriptParam>
ReportError(std::string strScriptName, lua_State* pState, std::vector<SGTScriptParam> vResults, std::string strInfo=std::string(""))
{
	if(!vResults.size())
		return vResults;
	if(vResults[0].getType()==SGTScriptParam::PARM_TYPE_NONE)
	{
		//find out position in script
		std::string strErr=std::string("");
		if(vResults.size()==2)
		{
			if(vResults[1].getType()==SGTScriptParam::PARM_TYPE_STRING)
				strErr+= std::string(": ") + vResults[1].getString();
		}
		else
			strErr+= std::string(".");
		SGTLuaScript::LogError(strScriptName, GetLuaLine(pState), std::string("(") + strInfo + std::string(")") + strErr);
	}
	return vResults;
}

std::vector<SGTScriptParam>
SGTLuaScript::CallFunction(SGTScript &caller, std::string strName, std::vector<SGTScriptParam> params)
{
	lua_pushinteger(m_pState, ((int)this));
	lua_setglobal(m_pState, "_SGTLuaScript");

	lua_pushinteger(m_pState, ((int)&caller));
	lua_setglobal(m_pState, "_scriptID");

	std::vector<SGTScriptParam> outParams;
	if(!FunctionExists(strName))
	{
		outParams.push_back(SGTScriptParam());
		outParams.push_back(SGTScriptParam(std::string("function \"") + strName + std::string("\" does not exist")));
		return outParams;
	}

	lua_getglobal(m_pState, strName.c_str());//push the function
	
	PutArguments(m_pState, params, caller);

	
	int iStackSize=lua_gettop(m_pState);
	if(lua_pcall(m_pState, params.size(), LUA_MULTRET,0 )!=0)
	{
		const char* pcErr=lua_tostring(m_pState, -1);
		LogError(m_strScriptName, -1, std::string(": error calling function ") + strName + std::string(": ") + std::string(pcErr));
		outParams.push_back(SGTScriptParam());
		return outParams;
	}
	else//get the returned arguments
		outParams=ReportError(m_strScriptName, m_pState, GetArguments(m_pState, iStackSize, caller), std::string("getting results from call to ") + strName);

	return outParams;
}

int
SGTLuaScript::ApiCallback(lua_State* pState)
{
	lua_Debug ar;
	lua_getstack(pState, 0, &ar);
	lua_getinfo(pState, "n", &ar);
	std::string strFunction("");
	if(ar.name)
		strFunction=std::string(ar.name);
	
	int iScriptID;
	lua_getglobal(pState, "_SGTLuaScript");
	if(lua_isnumber(pState, -1))
		iScriptID=lua_tointeger(pState, -1);
	else
	{
		lua_pop(pState, 1);
		return 0;
	}
	lua_pop(pState, 1);//pop the ID
	SGTLuaScript& pScript=*((SGTLuaScript*)iScriptID);

	int iInstanceID;
	lua_getglobal(pState, "_scriptID");
	if(lua_isnumber(pState, -1))
		iInstanceID=lua_tointeger(pState, -1);
	else
	{
		lua_pop(pState, 1);
		return 0;
	}
	lua_pop(pState, 1);//pop the ID
	SGTScript& pInstance=*((SGTScript*)iInstanceID);

	//check if that function is anything that was shared before
	if(pScript.m_mExternalFunctions.find(strFunction)==pScript.m_mExternalFunctions.end() &&
		pScript.m_mFunctions.find(strFunction)==pScript.m_mFunctions.end() &&
		!CLongCallHandler::GetInstance().GetTarget(pInstance.GetID(), strFunction).second.size())
	{
		//it doesn't
		//so check every share for identity with the function
		lua_getinfo(pState, "f", &ar);
		for(std::map<std::string, std::pair<SGTScript, std::string>>::const_iterator it=CLongCallHandler::GetInstance().GetShares(pInstance.GetID()); it!=CLongCallHandler::GetInstance().GetSharesEnd(pInstance.GetID()); it++)
		{
			lua_getglobal(pState, it->first.c_str());
			if(lua_equal(pState, -1, -2))
			{
				lua_pop(pState, 1);
				strFunction=it->first;
				break;
			}
			lua_pop(pState, 1);
		}
		lua_pop(pState, 1);
	}

	//extract the params
	std::vector<SGTScriptParam> vResults;
	if(pScript.m_mExternalFunctions.find(strFunction)==pScript.m_mExternalFunctions.end())
	{
		if(CLongCallHandler::GetInstance().GetTarget(pInstance.GetID(), strFunction).second.size())
			vResults=CLongCallHandler::GetInstance().GetTarget(pInstance.GetID(), strFunction).first.CallFunction(CLongCallHandler::GetInstance().GetTarget(pInstance.GetID(), strFunction).second, ReportError(pScript.GetScriptName(), pState, GetArguments(pState, 1, pInstance)));
		else if(pScript.m_mFunctions.find(strFunction)!=pScript.m_mFunctions.end())
		{
			SCShare share=pScript.m_mFunctions.find(strFunction)->second;
			if(share.bIsStatic)
				vResults=share.fns(pInstance, pScript, ReportError(pScript.GetScriptName(), pState, GetArguments(pState, 1, pInstance)));
			else
				vResults=share.fn(pInstance, ReportError(pScript.GetScriptName(), pState, GetArguments(pState, 1, pInstance)));
		}
		else
		{
			LogError(pScript.GetScriptName(), GetLuaLine(pState), std::string("could not find shared function ") + strFunction);
		}
	}
	else
	{
		SExternalShare data=pScript.m_mExternalFunctions.find(strFunction)->second;
		vResults=data.script.CallFunction(pInstance, data.strInternalName, ReportError(pScript.GetScriptName(), pState, GetArguments(pState, 1, pInstance)));
	}

	PutArguments(pState, ReportError(pScript.GetScriptName(), pState, vResults, std::string("call to ") + strFunction), pInstance);

	lua_pushinteger(pState, iInstanceID);
	lua_setglobal(pState, "_scriptID");
	return vResults.size();
}

std::string
SGTLuaScript::GetScriptName(){return m_strScriptName;}

//error stuff

void
SGTLuaScript::SetLogFn(void (*logFn)(std::string, int, std::string))
{
	m_LogFn=logFn;
}

void
SGTLuaScript::LogError(std::string strScript, int iLine, std::string strError)
{
	if(m_LogFn!=0)
		m_LogFn(strScript, iLine, strError);
}

void
SGTLuaScript::SetLoader(std::string (*pfLoader)(lua_State* pState, std::string strFileName))
{
	m_pfLoader=pfLoader;
}

CLongCallHandler::CLongCallHandler()
{
}

CLongCallHandler&
CLongCallHandler::GetInstance()
{
	static CLongCallHandler singleton;
	return singleton;
}

std::pair<SGTScript, std::string>
CLongCallHandler::GetTarget(int iScriptID, std::string strFunction)
{
	std::map<int, std::map<std::string, std::pair<SGTScript, std::string>>>::const_iterator it;
	if((it=m_mRegisteredFunctions.find(iScriptID))==m_mRegisteredFunctions.end())
		return std::pair<SGTScript, std::string>(SGTScript(),std::string(""));
	std::map<std::string, std::pair<SGTScript, std::string>>::const_iterator itInner;
	if((itInner=it->second.find(strFunction))==it->second.end())
		return std::pair<SGTScript, std::string>(SGTScript(),std::string(""));
	return itInner->second;
}

void
CLongCallHandler::RegisterFunction(int iScriptID, std::string strFunction, SGTScript script, std::string strTargetFunction)
{
	std::map<int, std::map<std::string, std::pair<SGTScript, std::string>>>::iterator it;
	if((it=m_mRegisteredFunctions.find(iScriptID))==m_mRegisteredFunctions.end())
	{
		m_mRegisteredFunctions.insert(std::pair<int, std::map<std::string, std::pair<SGTScript, std::string>>>(iScriptID, std::map<std::string, std::pair<SGTScript, std::string>>()));
		it=m_mRegisteredFunctions.find(iScriptID);
	}
	it->second.insert(std::pair<std::string, std::pair<SGTScript, std::string>>(strFunction, std::pair<SGTScript, std::string>(script, strTargetFunction)));
}

std::string
CLongCallHandler::TargetSet(int iScriptID, int iTargetScriptID, std::string strTargetFunction)
{
	std::map<int, std::map<std::string, std::pair<SGTScript, std::string>>>::iterator it;
	if((it=m_mRegisteredFunctions.find(iScriptID))==m_mRegisteredFunctions.end())
		return std::string();
	for(std::map<std::string, std::pair<SGTScript, std::string>>::iterator itFunctions=it->second.begin();
		itFunctions!=it->second.end(); itFunctions++)
	{
		if(itFunctions->second.first.GetID()==iTargetScriptID && !itFunctions->second.second.compare(strTargetFunction))
			return itFunctions->first;
	}
	return std::string();
}

std::map<std::string, std::pair<SGTScript, std::string>>::const_iterator
CLongCallHandler::GetShares(int iScriptID)
{
	return m_mRegisteredFunctions.find(iScriptID)->second.begin();
}

std::map<std::string, std::pair<SGTScript, std::string>>::const_iterator
CLongCallHandler::GetSharesEnd(int iScriptID)
{
	return m_mRegisteredFunctions.find(iScriptID)->second.end();
}