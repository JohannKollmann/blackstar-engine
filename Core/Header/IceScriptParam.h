#pragma once

#include <string>
#include <map>

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
				PARM_TYPE_TABLE=0x20,
				PARM_TYPE_ANY=0xffff,
				PARM_TYPE_MORE=0x10000 //used to indicate variadic arguments
		};
		ScriptParam(ETypes type);
		/**
		* constructor for PARM_TYPE_NONE
		*/
		ScriptParam();
		ScriptParam(int i);
		ScriptParam(bool b);
		ScriptParam(double f);
		ScriptParam(std::string s);
		/**
		* constructor for PARM_TYPE_FUNCTION
		* @param strFnName name of the function
		* @param script reference to the script that contains the function
		*/
		ScriptParam(std::string strFnName, Script& script);
		/**
		* constructor for PARM_TYPE_TABLE
		* @param t map containing the table data
		*/
		ScriptParam(const std::map<ScriptParam, ScriptParam>& t);

		ETypes getType() const;
		bool hasInt() const;
		int getInt() const;
		bool getBool() const;
		double getFloat() const;
		std::string getString() const;
		void getFunction(std::string& strFnName, Script& script) const;
		std::map<ScriptParam, ScriptParam> getTable() const;
		
		void set(ScriptParam& param);

		bool operator<(const ScriptParam& rvalue) const;
		bool operator==(const ScriptParam& rvalue) const;
		bool operator!=(const ScriptParam& rvalue) const;
private:
		int m_iData;
		bool m_bData;
		double m_fData;
		std::string m_strData;
		LuaScript* m_pScript;
		int m_iScriptID;
		std::map<ScriptParam, ScriptParam> m_mData;
		ETypes m_Type;
};

};
