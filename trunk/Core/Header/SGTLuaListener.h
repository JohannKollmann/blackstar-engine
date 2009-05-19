
#pragma once

#include "SGTScriptParam.h"
#include "SGTScript.h"

class SGTDllExport SGTLuaListener
{
public:
	SGTLuaListener() {};
	virtual ~SGTLuaListener() {};

	virtual std::vector<SGTScriptParam> OnCalledByLua(std::string FnName, SGTScript& caller, std::vector<SGTScriptParam> input) = 0;
};