#ifndef __RESIS_INCL__
#define __RESIS_INCL__

#include "SGTLoadSave.h"
#include "SGTScriptSystem.h"
#include "LoadSaveMapHandler.h"

class ResidentVariables;


class ResidentManager
{
public:
	ResidentManager();
	static ResidentManager& GetInstance();
	void BindResisToScript(ResidentVariables resis, SGTScript& script);
	ResidentVariables& GetResis(SGTScript& script);
	void Clear();
private:
	std::map<int, ResidentVariables> m_mResis;
	std::map<std::string, std::map<std::string, SGTScriptParam>> m_mAllocatedVars;

	static std::vector<SGTScriptParam> AllocCallback(SGTScript &caller, std::vector<SGTScriptParam> params);
	static std::vector<SGTScriptParam> SetCallback(SGTScript &caller, std::vector<SGTScriptParam> params);
	static std::vector<SGTScriptParam> GetCallback(SGTScript &caller, std::vector<SGTScriptParam> params);
};

class ResidentVariables : SGTSaveable
{
public:
	void Save(SGTSaveSystem& myManager);
	void Load(SGTLoadSystem& myManager);
	static SGTSaveable* NewInstance();
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn);
	std::string& TellName();

	SGTScriptParam GetVariable(std::string strVar);
	void SetVariable(std::string strVar, SGTScriptParam& param);
	
	class SaveableScriptParam : public SGTSaveable
	{
	public:
		void Save(SGTSaveSystem& myManager);
		void Load(SGTLoadSystem& myManager);
		static SGTSaveable* NewInstance();
		static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn);
		std::string& TellName();
		SGTScriptParam ToSGTScriptParam();
		SaveableScriptParam(SGTScriptParam& param);
		SaveableScriptParam();
		void set(SGTScriptParam& param);
	private:
		SGTScriptParam m_Param;
	};
private:
	
	std::map<std::string, SaveableScriptParam> m_mVars;
};

#endif