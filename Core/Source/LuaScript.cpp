#include "LuaScript.h"
#include "IceScriptSystem.h"
#include <sstream>

namespace Ice
{

void (*LuaScript::m_LogFn)(std::string, int, std::string) =0;
std::string (*LuaScript::m_pfLoader)(lua_State*, std::string) =0;

//handler for transparent function calls across VMs
class CLongCallHandler
{
public:
	CLongCallHandler();
	static CLongCallHandler& GetInstance();
	void RegisterFunction(int iScriptID, std::string strFunction, Script script, std::string strTargetFunction);
	std::pair<Script, std::string> GetTarget(int iScriptID, std::string strFunction);
	std::string TargetSet(int iScriptID, int iTargetScriptID, std::string strTargetFunction);
	bool HasShares(int iScriptID);
	std::map<std::string, std::pair<Script, std::string>>::const_iterator GetShares(int iScriptID);
	std::map<std::string, std::pair<Script, std::string>>::const_iterator GetSharesEnd(int iScriptID);
private:
	std::map<int, std::map<std::string, std::pair<Script, std::string>>> m_mRegisteredFunctions;
};

class CLuaShareInstanceHandler
{
public:
	CLuaShareInstanceHandler();
	static CLuaShareInstanceHandler& GetInstance();
	void SetScriptInstance(LuaScript* pScript, int iCallerID, Script& instance);
	void RemoveScriptInstance(LuaScript* pScript, int iCallerID);
	Script GetScriptInstance(LuaScript* pScript, int iCallerID);
private:
	std::map<LuaScript*, std::map<int, Script>> m_mInstances;
};

LuaScript::LuaScript(std::string strFile)
{
	LoadScript(strFile);
}

bool
LuaScript::LoadScript(std::string strFile)
{
	m_strScriptName=strFile;
	m_pState=luaL_newstate();
	luaopen_base(m_pState);
	luaopen_string(m_pState);
	luaopen_table(m_pState);

	std::string strErr=m_pfLoader(m_pState, strFile);
	if(strErr.length())
	{
		LogError(strFile, -1, strErr);
		return false;
	}
	return true;
}

void
LuaScript::ShareCFunction(std::string strName, ScriptFunction fn)
{
	m_mFunctions.insert(std::pair<std::string, SCShare>(strName, SCShare(false, fn)));
	lua_register(m_pState, strName.c_str(), &LuaScript::ApiCallback);
}

void
LuaScript::ShareStaticCFunction(std::string strName, StaticScriptFunction fn)
{
	m_mFunctions.insert(std::pair<std::string, SCShare>(strName, SCShare(true, fn)));
	lua_register(m_pState, strName.c_str(), &LuaScript::ApiCallback);
}

void
LuaScript::ShareExternalFunction(std::string strShareName, std::string strInternalName, Script &script)
{
	SExternalShare share={strInternalName, script};
	m_mExternalFunctions.insert(std::pair<std::string, SExternalShare>(strShareName, share));
	lua_register(m_pState, strShareName.c_str(), &LuaScript::ApiCallback);
}

std::vector<ScriptParam>
LuaScript::GetArguments(lua_State* pState, int iStartIndex, Script& script)
{
	int nParams=lua_gettop(pState);
	std::vector<ScriptParam> vParams;
	int iType;
	for(int iParam=iStartIndex; iParam<=nParams; iParam++)
		switch(iType=lua_type(pState, iParam))
		{
		case LUA_TNUMBER:
			vParams.push_back(ScriptParam(lua_tonumber(pState, iParam)));
			break;
		case LUA_TBOOLEAN:
			vParams.push_back(ScriptParam((bool)(lua_toboolean(pState, iParam)!=0)));
			break;
		case LUA_TSTRING:
		{
			std::string str(lua_tostring(pState, iParam));
			vParams.push_back(ScriptParam(str));
			break;
		}
		case LUA_TFUNCTION:
		{
			std::string strFunctionName;
			bool bFinished=false;
			//iterate all global variables
			
			//required by lua
			lua_pushnil(pState);
			while(lua_next(pState, LUA_GLOBALSINDEX) != 0)
			{
				//function name is at -2, function at -1
				//check if the iterated function is equal to the parameter.
				//there is no way for lua to directly tell the name of the function
				if(lua_equal(pState, -1, iParam))
				{//found the function
					vParams.push_back(ScriptParam(strFunctionName=std::string(lua_tostring(pState, -2)), script));
					bFinished=true;
					break;
				}
				//required by lua
				lua_pop(pState, 1);
			}
			if(bFinished)
				//the function is already in the args
				break;
			else
			{
				//this must be a local (inline) function
				//find a free name for the function
				int iCounter=0;
				std::stringstream strs;
				lua_getglobal(pState, "_LuaScriptFunction");
				std::string strCounter;
				while(lua_isfunction(pState, -1))
				{
					lua_pop(pState, 1);
					strs.clear();
					strs<<(iCounter++);
					strs>>strCounter;
					lua_getglobal(pState, (std::string("_LuaScriptFunction") + strCounter).c_str());
				}
				lua_pop(pState, 1);
				//create a global variable for this function
				lua_pushvalue(pState, iParam);
				lua_setglobal(pState, (std::string("_LuaScriptFunction") + strCounter).c_str());
				vParams.push_back(ScriptParam(std::string("_LuaScriptFunction") + strCounter, script));
			}
			break;
		}
		case LUA_TTABLE:
		{
			//iterate the table
			std::map<ScriptParam, ScriptParam> mTable;
			//required by lua
			lua_pushnil(pState);
			while (lua_next(pState, iParam) != 0)
			{
				//key and value have been pushed onto the stack.
				//simply recurse
				//keep in mind, this can also lead to nested tables
				std::vector<ScriptParam> keyvalue=GetArguments(pState, lua_gettop(pState)-1, script);
				//check for error
				if(keyvalue[0].getType()==ScriptParam::PARM_TYPE_NONE)
				{//we produced an error
					//error code is contained in the output of GetArguments
					return keyvalue;
				}
				//no error occured
				//key is at index 0, value at index 1
				mTable.insert(std::make_pair(keyvalue[0], keyvalue[1]));
				//required by lua
				lua_pop(pState, 1);
			}
			vParams.push_back(ScriptParam(mTable));
			break;
		}
		default:
			//unsupported type
			vParams.clear();
			vParams.push_back(ScriptParam());
			std::stringstream st;
			st << iType;
			std::stringstream st2;
			st2 << (iParam-iStartIndex);
			vParams.push_back(ScriptParam(std::string("unsupported type (") + (iType==LUA_TNIL ? std::string("nil") : st.str()) + std::string(") while extracting param ") + st2.str()));
			return vParams;
		};
	return vParams;
}


void
LuaScript::PutArguments(lua_State *pState, std::vector<ScriptParam> params, Script& script,  Script* pShareCallInstanceHack)
{
	lua_checkstack(pState, params.size()*2);//make sure that the stack is sufficently big for the parameters
	//push the parameters
	for(unsigned int iParam=0; iParam<params.size(); iParam++)
	{
		switch(params[iParam].getType())
		{
		case ScriptParam::PARM_TYPE_INT:
			lua_pushinteger(pState, params[iParam].getInt());
			break;
		case ScriptParam::PARM_TYPE_BOOL:
			lua_pushboolean(pState, params[iParam].getBool());
			break;
		case ScriptParam::PARM_TYPE_STRING:
			lua_pushstring(pState, params[iParam].getString().c_str());
			break;
		case ScriptParam::PARM_TYPE_FLOAT:
			lua_pushnumber(pState, params[iParam].getFloat());
			break;
		case ScriptParam::PARM_TYPE_FUNCTION:
		{	//check if we have already added that function
			std::string strTargetFnName;
			Script targetscript;
			params[iParam].getFunction(strTargetFnName, targetscript);
			if(!CLongCallHandler::GetInstance().TargetSet(script.GetID(), targetscript.GetID(), strTargetFnName).size())
			{//create the function
				//same code as in GetArguments
				int iCounter=0;
				std::stringstream strs;
				lua_getglobal(pState, "_LuaScriptFunctionCall");
				std::string strCounter;
				while(lua_isfunction(pState, -1))
				{
					lua_pop(pState, 1);
					strs.clear();
					strs<<(iCounter++);
					strs>>strCounter;
					lua_getglobal(pState, (std::string("_LuaScriptFunctionCall") + strCounter).c_str());
				}
				lua_pop(pState, 1);
				lua_register(pState, (std::string("_LuaScriptFunctionCall") + strCounter).c_str(), &LuaScript::ApiCallback);
				//put it into the map
				if(pShareCallInstanceHack)
					CLongCallHandler::GetInstance().RegisterFunction(pShareCallInstanceHack->GetID(), std::string("_LuaScriptFunctionCall") + strCounter, targetscript, strTargetFnName);
				else
					CLongCallHandler::GetInstance().RegisterFunction(script.GetID(), std::string("_LuaScriptFunctionCall") + strCounter, targetscript, strTargetFnName);
			}
			int iScriptID=pShareCallInstanceHack ? pShareCallInstanceHack->GetID() : script.GetID();
			lua_getglobal(pState, CLongCallHandler::GetInstance().TargetSet(iScriptID, targetscript.GetID(), strTargetFnName).c_str());
			break;
		}
		case ScriptParam::PARM_TYPE_TABLE:
		{
			std::map<ScriptParam, ScriptParam> mTable=params[iParam].getTable();
			//create anonymous table
			lua_createtable(pState, mTable.size(),0);
			for(std::map<ScriptParam, ScriptParam>::const_iterator it=mTable.begin(); it!=mTable.end(); it++)
			{
				//first push the key, then the value
				std::vector<ScriptParam> vKeyValue(2);
				vKeyValue[0]=it->first;
				vKeyValue[1]=it->second;
				//recurse to do so
				PutArguments(pState, vKeyValue, script,  pShareCallInstanceHack);
				//now that our arguments are pushed, we can use settable
				//value is at -1, key at -2 and table at -3
				lua_settable(pState, -3);
				//lua erases both key and value from the stack, so the table is the top element now
			}
			break;
		}
		//kind of the 'default' case
		case ScriptParam::PARM_TYPE_NONE:
			lua_pushnil(pState);
			break;
		}
	}
}

bool
LuaScript::FunctionExists(std::string strFunction)
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
std::vector<ScriptParam>
ReportError(std::string strScriptName, lua_State* pState, std::vector<ScriptParam> vResults, std::string strInfo=std::string(""))
{
	if(!vResults.size())
		return vResults;
	if(vResults[0].getType()==ScriptParam::PARM_TYPE_NONE)
	{
		//find out position in script
		std::string strErr=std::string("");
		if(vResults.size()==2)
		{
			if(vResults[1].getType()==ScriptParam::PARM_TYPE_STRING)
				strErr+= std::string(": ") + vResults[1].getString();
		}
		else
			strErr+= std::string(".");
		LuaScript::LogError(strScriptName, GetLuaLine(pState), std::string("(") + strInfo + std::string(")") + strErr);
	}
	return vResults;
}

std::vector<ScriptParam>
ReportErrorWithoutLine(std::string strScriptName, lua_State* pState, std::vector<ScriptParam> vResults, std::string strInfo=std::string(""))
{
	if(!vResults.size())
		return vResults;
	if(vResults[0].getType()==ScriptParam::PARM_TYPE_NONE)
	{
		//find out position in script
		std::string strErr=std::string("");
		if(vResults.size()==2)
		{
			if(vResults[1].getType()==ScriptParam::PARM_TYPE_STRING)
				strErr+= std::string(": ") + vResults[1].getString();
		}
		else
			strErr+= std::string(".");
		LuaScript::LogError(strScriptName, -1, std::string("(") + strInfo + std::string(")") + strErr);
	}
	return vResults;
}

std::vector<ScriptParam>
LuaScript::CallFunction(Script &caller, std::string strName, std::vector<ScriptParam> params, Script* pShareCallInstanceHack)//=Script())
{
	lua_pushinteger(m_pState, ((int)this));
	lua_setglobal(m_pState, "_LuaScript");

	lua_pushinteger(m_pState, ((int)&caller));
	lua_setglobal(m_pState, "_scriptID");

	std::vector<ScriptParam> outParams;
	if(!FunctionExists(strName))
	{
		outParams.push_back(ScriptParam());
		outParams.push_back(ScriptParam(std::string("function \"") + strName + std::string("\" does not exist")));
		return outParams;
	}

	lua_getglobal(m_pState, strName.c_str());//push the function
	
	int iStackSize=lua_gettop(m_pState);

	PutArguments(m_pState, params, caller, pShareCallInstanceHack);
	
	
	if(lua_pcall(m_pState, params.size(), LUA_MULTRET,0 )!=0)
	{
		const char* pcErr=lua_tostring(m_pState, -1);
		LogError(m_strScriptName, -1, std::string(": error calling function ") + strName + std::string(": ") + std::string(pcErr));
		outParams.push_back(ScriptParam());
		return outParams;
	}
	else//get the returned arguments
	{
		Script referenceInstance=caller;
		if(caller.GetScriptName()!=m_strScriptName)//this is a proxy call
		{
			referenceInstance=CLuaShareInstanceHandler::GetInstance().GetScriptInstance(this, caller.GetID());
			if(referenceInstance.GetID()==-1)
				referenceInstance=caller;
		}
		outParams=ReportErrorWithoutLine(m_strScriptName, m_pState, GetArguments(m_pState, iStackSize, referenceInstance), std::string("getting results from call to ") + strName);
	}

	return outParams;
}

int
LuaScript::ApiCallback(lua_State* pState)
{
	lua_Debug ar;
	lua_getstack(pState, 0, &ar);
	lua_getinfo(pState, "n", &ar);
	std::string strFunction("");
	if(ar.name)
		strFunction=std::string(ar.name);
	
	int iScriptID;
	lua_getglobal(pState, "_LuaScript");
	if(lua_isnumber(pState, -1))
		iScriptID=lua_tointeger(pState, -1);
	else
	{
		lua_pop(pState, 1);
		return 0;
	}
	lua_pop(pState, 1);//pop the ID
	LuaScript& pScript=*((LuaScript*)iScriptID);

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
	Script& pInstance=*((Script*)iInstanceID);

	

	//check if that function is anything that was shared before
	if(pScript.m_mExternalFunctions.find(strFunction)==pScript.m_mExternalFunctions.end() &&
		pScript.m_mFunctions.find(strFunction)==pScript.m_mFunctions.end() &&
		!CLongCallHandler::GetInstance().GetTarget(pInstance.GetID(), strFunction).second.size())
	{
		//it doesn't
		//so check every share for identity with the function
		lua_getinfo(pState, "f", &ar);
		if(CLongCallHandler::GetInstance().HasShares(pInstance.GetID()))
			for(std::map<std::string, std::pair<Script, std::string>>::const_iterator it=CLongCallHandler::GetInstance().GetShares(pInstance.GetID()); it!=CLongCallHandler::GetInstance().GetSharesEnd(pInstance.GetID()); it++)
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

	Script referenceInstance=pInstance;
	if(pInstance.GetScriptName()!=pScript.GetScriptName())//this is a proxy call
	{
		referenceInstance=CLuaShareInstanceHandler::GetInstance().GetScriptInstance(&pScript, pInstance.GetID());
		if(referenceInstance.GetID()==-1)
			referenceInstance=pInstance;
	}
	//extract the params
	std::vector<ScriptParam> vResults;
	if(pScript.m_mExternalFunctions.find(strFunction)==pScript.m_mExternalFunctions.end())
	{
		if(CLongCallHandler::GetInstance().GetTarget(pInstance.GetID(), strFunction).second.size())
			vResults=CLongCallHandler::GetInstance().GetTarget(pInstance.GetID(), strFunction).first.CallFunction(CLongCallHandler::GetInstance().GetTarget(pInstance.GetID(), strFunction).second, ReportError(pScript.GetScriptName(), pState, GetArguments(pState, 1, referenceInstance)));
		else if(pScript.m_mFunctions.find(strFunction)!=pScript.m_mFunctions.end())
		{
			SCShare share=pScript.m_mFunctions.find(strFunction)->second;
			if(share.bIsStatic)
				vResults=share.fns(pInstance, pScript, ReportError(pScript.GetScriptName(), pState, GetArguments(pState, 1, referenceInstance)));
			else
				vResults=share.fn(pInstance, ReportError(pScript.GetScriptName(), pState, GetArguments(pState, 1, referenceInstance)));
		}
		else
		{
			LogError(pScript.GetScriptName(), GetLuaLine(pState), std::string("could not find shared function ") + strFunction);
		}
	}
	else
	{
		SExternalShare data=pScript.m_mExternalFunctions.find(strFunction)->second;
		//hack: use CLuaShareInstanceHandler
		CLuaShareInstanceHandler::GetInstance().SetScriptInstance(data.script.m_pLuaScript, pInstance.GetID(), data.script);
		vResults=data.script.m_pLuaScript->CallFunction(pInstance, data.strInternalName, ReportError(pScript.GetScriptName(), pState, GetArguments(pState, 1, referenceInstance)), &data.script);
		//now remove the instance
		CLuaShareInstanceHandler::GetInstance().RemoveScriptInstance(data.script.m_pLuaScript, pInstance.GetID());
	}

	PutArguments(pState, ReportError(pScript.GetScriptName(), pState, vResults, std::string("call to ") + strFunction), pInstance);

	lua_pushinteger(pState, iInstanceID);
	lua_setglobal(pState, "_scriptID");
	return vResults.size();
}

std::string
LuaScript::GetScriptName() const {return m_strScriptName;}

//error stuff

void
LuaScript::SetLogFn(void (*logFn)(std::string, int, std::string))
{
	m_LogFn=logFn;
}

void
LuaScript::LogError(std::string strScript, int iLine, std::string strError)
{
	if(m_LogFn!=0)
		m_LogFn(strScript, iLine, strError);
}

void
LuaScript::SetLoader(std::string (*pfLoader)(lua_State* pState, std::string strFileName))
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

std::pair<Script, std::string>
CLongCallHandler::GetTarget(int iScriptID, std::string strFunction)
{
	std::map<int, std::map<std::string, std::pair<Script, std::string>>>::const_iterator it;
	if((it=m_mRegisteredFunctions.find(iScriptID))==m_mRegisteredFunctions.end())
		return std::pair<Script, std::string>(Script(),std::string(""));
	std::map<std::string, std::pair<Script, std::string>>::const_iterator itInner;
	if((itInner=it->second.find(strFunction))==it->second.end())
		return std::pair<Script, std::string>(Script(),std::string(""));
	return itInner->second;
}

void
CLongCallHandler::RegisterFunction(int iScriptID, std::string strFunction, Script script, std::string strTargetFunction)
{
	std::map<int, std::map<std::string, std::pair<Script, std::string>>>::iterator it;
	if((it=m_mRegisteredFunctions.find(iScriptID))==m_mRegisteredFunctions.end())
	{
		m_mRegisteredFunctions.insert(std::pair<int, std::map<std::string, std::pair<Script, std::string>>>(iScriptID, std::map<std::string, std::pair<Script, std::string>>()));
		it=m_mRegisteredFunctions.find(iScriptID);
	}
	it->second.insert(std::pair<std::string, std::pair<Script, std::string>>(strFunction, std::pair<Script, std::string>(script, strTargetFunction)));
}

std::string
CLongCallHandler::TargetSet(int iScriptID, int iTargetScriptID, std::string strTargetFunction)
{
	std::map<int, std::map<std::string, std::pair<Script, std::string>>>::iterator it;
	if((it=m_mRegisteredFunctions.find(iScriptID))==m_mRegisteredFunctions.end())
		return std::string();
	for(std::map<std::string, std::pair<Script, std::string>>::iterator itFunctions=it->second.begin();
		itFunctions!=it->second.end(); itFunctions++)
	{
		if(itFunctions->second.first.GetID()==iTargetScriptID && !itFunctions->second.second.compare(strTargetFunction))
			return itFunctions->first;
	}
	return std::string();
}

bool
CLongCallHandler::HasShares(int iScriptID)
{
	if(m_mRegisteredFunctions.find(iScriptID)==m_mRegisteredFunctions.end())
		return false;
	return true;
}

std::map<std::string, std::pair<Script, std::string>>::const_iterator
CLongCallHandler::GetShares(int iScriptID)
{
	return m_mRegisteredFunctions.find(iScriptID)->second.begin();
}

std::map<std::string, std::pair<Script, std::string>>::const_iterator
CLongCallHandler::GetSharesEnd(int iScriptID)
{
	return m_mRegisteredFunctions.find(iScriptID)->second.end();
}

CLuaShareInstanceHandler::CLuaShareInstanceHandler()
{
}

CLuaShareInstanceHandler&
CLuaShareInstanceHandler::GetInstance()
{
	static CLuaShareInstanceHandler singleton;
	return singleton;
}
void
CLuaShareInstanceHandler::SetScriptInstance(LuaScript *pScript, int iCallerID, Script &instance)
{
	std::map<LuaScript*, std::map<int, Script>>::iterator it=m_mInstances.find(pScript);
	if(it!=m_mInstances.end())
	{
		it->second.insert(std::pair<int, Script>(iCallerID,instance));
	}
	else
	{
		std::map<int, Script> m;
		m[iCallerID]=instance;
		m_mInstances[pScript]=m;
	}
}

void
CLuaShareInstanceHandler::RemoveScriptInstance(LuaScript *pScript, int iCallerID)
{
	std::map<LuaScript*, std::map<int, Script>>::iterator it=m_mInstances.find(pScript);
	if(it!=m_mInstances.end())
	{
		it->second.erase(iCallerID);
	}
}

Script
CLuaShareInstanceHandler::GetScriptInstance(LuaScript *pScript, int iCallerID)
{
	std::map<LuaScript*, std::map<int, Script>>::iterator it=m_mInstances.find(pScript);
	if(it!=m_mInstances.end())
	{
		if(it->second.find(iCallerID)!=it->second.end())
		{
			return it->second.find(iCallerID)->second;
		}
	}
	return Script();
}

};
