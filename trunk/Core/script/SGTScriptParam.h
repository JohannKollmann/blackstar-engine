
#pragma once

#include <string>
#include "SGTIncludes.h"

class SGTLuaScript;

class SGTDllExport SGTScriptParam
{
public:
	enum ETypes
	{
		PARM_TYPE_INT,
		PARM_TYPE_STRING,
		PARM_TYPE_BOOL,
		PARM_TYPE_FLOAT,
		PARM_TYPE_FUNCTION,
		PARM_TYPE_NONE //unknown or uninitialized
	};
	ETypes getType();
	bool hasInt();
	int getInt();
	bool getBool();
	double getFloat();
	std::string getString();
	void getFunction(std::string& strFnName, SGTLuaScript*& script);
	
	SGTScriptParam();
	SGTScriptParam(int i);
	SGTScriptParam(bool b);
	SGTScriptParam(double f);
	SGTScriptParam(std::string s);
	SGTScriptParam(std::string strFnName, SGTLuaScript& script);

	void set(SGTScriptParam& param);
private:
	int m_iData;
	bool m_bData;
	double m_fData;
	std::string m_strData;
	SGTLuaScript* m_pScript;
	ETypes m_Type;
};