#include "IceScriptParam.h"
#include "IceScriptSystem.h"

namespace Ice
{

	ScriptParam::ScriptParam(){m_Type=PARM_TYPE_NONE; m_pScript=0;}
	ScriptParam::ScriptParam(ETypes type) { m_Type = type; }
	ScriptParam::ScriptParam(int i){m_Type=PARM_TYPE_INT;m_iData=i;}
	ScriptParam::ScriptParam(bool b){m_Type=PARM_TYPE_BOOL;m_bData=b;}
	ScriptParam::ScriptParam(double f){m_Type=PARM_TYPE_FLOAT;m_fData=f;}
	ScriptParam::ScriptParam(std::string s){m_Type=PARM_TYPE_STRING;m_strData=s;}
	ScriptParam::ScriptParam(std::string strFnName, Script& script){m_Type=PARM_TYPE_FUNCTION;m_strData=strFnName;m_pScript=&ScriptSystem::GetInstance().m_mScripts.find(script.GetScriptName())->second;m_iScriptID=script.GetID();}
	ScriptParam::ScriptParam(const std::map<ScriptParam, ScriptParam>& t){m_Type=PARM_TYPE_TABLE; m_mData=t;}

	ScriptParam::ETypes ScriptParam::getType() const {return m_Type;}
	bool ScriptParam::hasInt()  const { return (m_Type==PARM_TYPE_FLOAT || m_Type==PARM_TYPE_INT); }

	int ScriptParam::getInt() const {return ((m_Type==PARM_TYPE_FLOAT) ? (int)m_fData : m_iData);}
	bool ScriptParam::getBool() const {return m_bData;}
	double ScriptParam::getFloat() const {return m_fData;}
	std::string ScriptParam::getString() const {return m_strData;}
	void ScriptParam::getFunction(std::string& strFnName, Script& script) const {strFnName=m_strData;script=Script(m_iScriptID, m_pScript);}
	std::map<ScriptParam, ScriptParam> ScriptParam::getTable() const {return m_mData;}
		

	void
	ScriptParam::set(ScriptParam &param){*this=param;}

	bool
	ScriptParam::operator!=(const ScriptParam& rvalue) const
	{
		return !this->operator==(rvalue);
	}


	bool
	ScriptParam::operator==(const ScriptParam& rvalue) const
	{
		if(m_Type!=rvalue.m_Type)
			return false;
		switch(m_Type)
		{
			case ScriptParam::PARM_TYPE_INT:
				return m_iData==rvalue.m_iData;
			case ScriptParam::PARM_TYPE_BOOL:
				return m_bData==rvalue.m_bData;
			case ScriptParam::PARM_TYPE_STRING:
				return m_strData.compare(rvalue.m_strData)==0;
			case ScriptParam::PARM_TYPE_FLOAT:
				return m_fData==rvalue.m_fData;
			case ScriptParam::PARM_TYPE_FUNCTION:
				if(m_iScriptID!=rvalue.m_iScriptID)
					return false;
				return m_strData.compare(rvalue.m_strData)==0;
			case ScriptParam::PARM_TYPE_TABLE:
			{
				//recurse
				if(m_mData.size()!=rvalue.m_mData.size())
					return false;
				std::map<ScriptParam, ScriptParam>::const_iterator it=m_mData.begin();
				std::map<ScriptParam, ScriptParam>::const_iterator itr=rvalue.m_mData.begin();
				for( ; it!=m_mData.end() && itr!=rvalue.m_mData.end(); it++, itr++)
				{
					/*ScriptParam spFirst=it->first,
								spSecond=it->second;
							
					if(spFirst!=itr->first)
						return false;
					if(spSecond!=itr->second)
						return false;*/
					if(it->first!=itr->first)
						return false;
					if(it->second!=itr->second)
						return false;
				}
				break;
			}
		}
		return true;
	}

	bool
	ScriptParam::operator<(const ScriptParam& rvalue) const
	{
		if(m_Type!=rvalue.m_Type)
			return m_Type<rvalue.m_Type;
		switch(m_Type)
		{
			case ScriptParam::PARM_TYPE_INT:
				return m_iData<rvalue.m_iData;
			case ScriptParam::PARM_TYPE_BOOL:
				return (int)m_bData<(int)rvalue.m_bData;
			case ScriptParam::PARM_TYPE_STRING:
				return m_strData.compare(rvalue.m_strData)<0;
			case ScriptParam::PARM_TYPE_FLOAT:
				return m_fData<rvalue.m_fData;
			case ScriptParam::PARM_TYPE_FUNCTION:
				if(m_iScriptID!=rvalue.m_iScriptID)
					return m_iScriptID<rvalue.m_iScriptID;
				return m_strData.compare(rvalue.m_strData)<0;
			case ScriptParam::PARM_TYPE_TABLE:
			{	//recurse
				std::map<ScriptParam, ScriptParam>::const_iterator it=m_mData.begin();
				std::map<ScriptParam, ScriptParam>::const_iterator itr=rvalue.m_mData.begin();
				for( ; it!=m_mData.end() && itr!=rvalue.m_mData.end(); it++, itr++)
				{/*
					ScriptParam spFirst=it->first,
								spSecond=it->second;
							
					if(spFirst!=itr->first)
						return spFirst<itr->first;
					if(spSecond!=itr->second)
						return spSecond<itr->second;*/
			
					if(!(it->first==itr->first))
						return it->first.operator<(itr->first);
					if(it->second!=itr->second)
						return it->second.operator<(itr->second);
				}
				break;
			}
		}
		return false;
	}

};