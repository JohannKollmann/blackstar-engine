#ifndef __RESIS_INCL__
#define __RESIS_INCL__

#include "LoadSave.h"
#include "IceScriptSystem.h"
#include "LoadSaveMapHandler.h"

class ResidentVariables;


class ResidentManager
{
public:
	ResidentManager();
	static ResidentManager& GetInstance();
	void BindResisToScript(ResidentVariables resis, Ice::Script& script);
	ResidentVariables& GetResis(Ice::Script& script);
	void Clear();
private:
	std::map<int, ResidentVariables> m_mResis;
	std::map<std::string, std::map<std::string, Ice::ScriptParam>> m_mAllocatedVars;

	static std::vector<Ice::ScriptParam> AllocCallback(Ice::Script &caller, std::vector<Ice::ScriptParam> params);
	static std::vector<Ice::ScriptParam> SetCallback(Ice::Script &caller, std::vector<Ice::ScriptParam> params);
	static std::vector<Ice::ScriptParam> GetCallback(Ice::Script &caller, std::vector<Ice::ScriptParam> params);
};

class ResidentVariables : LoadSave::Saveable
{
public:
	void Save(LoadSave::SaveSystem& myManager);
	void Load(LoadSave::LoadSystem& myManager);
	static LoadSave::Saveable* NewInstance();
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn);
	std::string& TellName();

	Ice::ScriptParam GetVariable(std::string strVar);
	void SetVariable(std::string strVar, Ice::ScriptParam& param);
	
	class SaveableScriptParam : public LoadSave::Saveable
	{
	public:
		void Save(LoadSave::SaveSystem& myManager);
		void Load(LoadSave::LoadSystem& myManager);
		static LoadSave::Saveable* NewInstance();
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn);
		std::string& TellName();
		Ice::ScriptParam ToIceScriptParam();
		SaveableScriptParam(Ice::ScriptParam& param);
		SaveableScriptParam();
		void set(Ice::ScriptParam& param);
	private:
		Ice::ScriptParam m_Param;
	};
private:
	
	std::map<std::string, SaveableScriptParam> m_mVars;
};

#endif