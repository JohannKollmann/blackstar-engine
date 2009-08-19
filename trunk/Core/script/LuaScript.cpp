#include "LuaScript.h"

void (*SGTLuaScript::m_LogFn)(std::string) =0;

SGTLuaScript::SGTLuaScript(std::string strFile)
{
	LoadScript(strFile);
}

bool
SGTLuaScript::LoadScript(std::string strFile)
{
	m_strScriptName=strFile;
	m_pState=luaL_newstate();
	luaopen_base(m_pState);
	luaopen_string(m_pState);

	if(luaL_loadfile(m_pState, strFile.c_str()) || lua_pcall(m_pState, 0, 0, 0))
	{
		std::string strError(lua_tostring(m_pState, -1));
		LogError(std::string("error opening ") + strFile + std::string(": ") + strError);
		return false;
	}
	return true;
}

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
GetArguments(lua_State* pState, int iStartIndex, SGTLuaScript& script)
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
			vParams.push_back(SGTScriptParam(std::string(lua_tostring(pState, iParam))));
			break;
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
				lua_pop(pState, 2);
				break;//the function is already in the args
			}
			else
				//user tried to use an invalid value or a local function
				vParams.push_back(SGTScriptParam());
			break;
		}
		default:
			//unsupported type
			vParams.push_back(SGTScriptParam());
			SGTLuaScript::LogError(std::string("unsupported type while extracting params"));
			break;
		};
	return vParams;
}


void
PutArguments(lua_State *pState, std::vector<SGTScriptParam> params)
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
		return outParams;
	}

	lua_getglobal(m_pState, strName.c_str());//push the function
	PutArguments(m_pState, params);

	
	int iStackSize=lua_gettop(m_pState);
	if(lua_pcall(m_pState, params.size(), LUA_MULTRET,0 )!=0)
	{
		const char* pcErr=lua_tostring(m_pState, -1);
		outParams.push_back(SGTScriptParam());
		return outParams;
	}
	else//get the returned arguments
		outParams=GetArguments(m_pState, iStackSize, *this);

	return outParams;
}

int
SGTLuaScript::ApiCallback(lua_State* pState)
{
	lua_Debug ar;
	lua_getstack(pState, 0, &ar);
	lua_getinfo(pState, "n", &ar);
	std::string strFunction(ar.name);
	
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

	//extract the params
	std::vector<SGTScriptParam> vResults;
	if(pScript.m_mExternalFunctions.find(strFunction)==pScript.m_mExternalFunctions.end())
	{
		if(pScript.m_mFunctions.find(strFunction)!=pScript.m_mFunctions.end())
		{
			SCShare share=pScript.m_mFunctions.find(strFunction)->second;
			if(share.bIsStatic)
				vResults=share.fns(pInstance, pScript, GetArguments(pState, 1, pScript));
			else
				vResults=share.fn(pInstance, GetArguments(pState, 1, pScript));
		}
		else
			LogError(std::string("could not find shared function ") + strFunction);
	}
	else
	{
		SExternalShare data=pScript.m_mExternalFunctions.find(strFunction)->second;
		vResults=data.script.CallFunction(pInstance, data.strInternalName, GetArguments(pState, 1, pScript));
	}
	if(vResults.size())
		if(vResults[vResults.size()-1].getType()==SGTScriptParam::PARM_TYPE_NONE)
			LogError(strFunction + std::string(" returned an error."));
	PutArguments(pState, vResults);

	lua_pushinteger(pState, iInstanceID);
	lua_setglobal(pState, "_scriptID");
	return vResults.size();
}

std::string
SGTLuaScript::GetScriptName(){return m_strScriptName;}

//error stuff

void
SGTLuaScript::SetLogFn(void (*logFn)(std::string))
{
	m_LogFn=logFn;
}

void
SGTLuaScript::LogError(std::string strError)
{
	if(m_LogFn!=0)
		m_LogFn(strError);
}