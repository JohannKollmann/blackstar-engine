#pragma once

#include <string>

namespace Ice
{

class LuaScript;
class Script;


class __declspec(dllexport) ScriptParam
{
public:
	enum ETypes
	{
		PARM_TYPE_NONE=0x0, //unknown or uninitialized
		PARM_TYPE_INT=0x1,
		PARM_TYPE_STRING=0x2,
		PARM_TYPE_BOOL=0x4,
		PARM_TYPE_FLOAT=0x8,
		PARM_TYPE_FUNCTION=0x10,
		PARM_TYPE_ANY=0xffff,
		PARM_TYPE_MORE=0x10000 //used to indicate variadic arguments
	};
	ETypes getType() const;
	bool hasInt() const;
	int getInt() const;
	bool getBool() const;
	double getFloat() const;
	std::string getString() const;
	void getFunction(std::string& strFnName, Script& script) const;
	
	ScriptParam(ETypes type);
	ScriptParam();
	ScriptParam(int i);
	ScriptParam(bool b);
	ScriptParam(double f);
	ScriptParam(std::string s);
	ScriptParam(std::string strFnName, Script& script);

	void set(ScriptParam& param);
private:
	int m_iData;
	bool m_bData;
	double m_fData;
	std::string m_strData;
	LuaScript* m_pScript;
	int m_iScriptID;
	ETypes m_Type;
};

};