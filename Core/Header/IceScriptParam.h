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
		PARM_TYPE_INT,
		PARM_TYPE_STRING,
		PARM_TYPE_BOOL,
		PARM_TYPE_FLOAT,
		PARM_TYPE_FUNCTION,
		PARM_TYPE_NONE //unknown or uninitialized
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