
#include "IceScriptedProcess.h"
#include "mmsystem.h"
#include "IceSceneManager.h"
#include "IceProcessNodeManager.h"

namespace Ice
{

	ScriptedProcess::ScriptedProcess(std::vector<ScriptParam> params)
	{
		mLastUpdateCall = 0.0f;

		for (std::vector<ScriptParam>::iterator i = params.begin(); i != params.end(); i++)
			mScriptParams.push_back((*i));
	}

	ScriptedProcess::~ScriptedProcess()
	{
	}

	void ScriptedProcess::OnEnter()
	{
		if (mEnterCallback.getType() == ScriptParam::PARM_TYPE_FUNCTION)
			ScriptSystem::GetInstance().RunCallbackFunction(mEnterCallback, mScriptParams);

	}
	void ScriptedProcess::OnUpdate(float time)
	{
		if (timeGetTime() - mLastUpdateCall > 0.1f)
		{
			mLastUpdateCall = (float)timeGetTime();
			if (mUpdateCallback.getType() == ScriptParam::PARM_TYPE_FUNCTION)
				ScriptSystem::GetInstance().RunCallbackFunction(mUpdateCallback, mScriptParams);
		}
	}
	void ScriptedProcess::OnLeave()
	{
		if (mLeaveCallback.getType() == ScriptParam::PARM_TYPE_FUNCTION)
			ScriptSystem::GetInstance().RunCallbackFunction(mLeaveCallback, mScriptParams);
	}

	void ScriptedProcess::ReceiveMessage(Msg &msg)
	{
		if (mIsActive) OnReceiveMessage(msg);
	}

	void ScriptedProcess::OnSetActive(bool active)
	{
		if (active) OnEnter();
		else OnLeave();
	}

	void ScriptedProcess::OnReceiveMessage(Msg &msg)
	{
	}

	std::vector<ScriptParam> ScriptedProcess::Lua_SetEnterCallback(Script& caller, std::vector<ScriptParam> vParams)
	{
		auto err = Utils::TestParameters(vParams, "int function");
		if (err == "")
		{
			auto node = ProcessNodeManager::Instance().GetProcessNode(vParams[0].getInt());
			std::shared_ptr<ScriptedProcess> scripted = std::dynamic_pointer_cast<ScriptedProcess, ProcessNode>(node);
			if (scripted.get()) scripted->mEnterCallback = vParams[1];
			else IceWarning("Process is not scriptable!")
		}
		else SCRIPT_RETURNERROR(err) 
		SCRIPT_RETURN()
	}
	std::vector<ScriptParam> ScriptedProcess::Lua_SetUpdateCallback(Script& caller, std::vector<ScriptParam> vParams)
	{
		auto err = Utils::TestParameters(vParams, "int function");
		if (err == "")
		{
			auto node = ProcessNodeManager::Instance().GetProcessNode(vParams[0].getInt());
			std::shared_ptr<ScriptedProcess> scripted = std::dynamic_pointer_cast<ScriptedProcess, ProcessNode>(node);
			if (scripted.get()) scripted->mUpdateCallback = vParams[1];
			else IceWarning("Process is not scriptable!")
		}
		else SCRIPT_RETURNERROR(err) 
		SCRIPT_RETURN()
	}
	std::vector<ScriptParam> ScriptedProcess::Lua_SetLeaveCallback(Script& caller, std::vector<ScriptParam> vParams)
	{
		auto err = Utils::TestParameters(vParams, "int function");
		if (err == "")
		{
			auto node = ProcessNodeManager::Instance().GetProcessNode(vParams[0].getInt());
			std::shared_ptr<ScriptedProcess> scripted = std::dynamic_pointer_cast<ScriptedProcess, ProcessNode>(node);
			if (scripted.get()) scripted->mLeaveCallback = vParams[1];
			else IceWarning("Process is not scriptable!")
		}
		else SCRIPT_RETURNERROR(err) 
		SCRIPT_RETURN()
	}


	DayCycleProcess::DayCycleProcess(std::vector<ScriptParam> params, int startTimeH, int startTimeM, int endtimeH, int endtimeM)
	: ScriptedProcess(params)
	{
		mLastUpdateCall = 0.0f;
		mEndTimeH = endtimeH;
		mEndTimeM = endtimeM;
		mStartTimeH = startTimeH;
		mStartTimeM = startTimeM;
	}

	DayCycleProcess::~DayCycleProcess()
	{
	}

	void DayCycleProcess::OnUpdate(float time)
	{
		int hour = SceneManager::Instance().GetHour();
		int minutes = SceneManager::Instance().GetMinutes();
		if (hour == mEndTimeH && minutes > mEndTimeM)		
		{
			TerminateProcess();
			return;
		}

		ScriptedProcess::OnUpdate(time);
	}

	std::vector<ScriptParam> ScriptedProcess::Lua_ScriptedProcess_Create(Script& caller, std::vector<ScriptParam> vParams)
	{
		std::shared_ptr<ScriptedProcess> process = ProcessNodeManager::Instance().CreateScriptedProcess(vParams);
		SCRIPT_RETURNVALUE(process->GetProcessID())
	}

	std::vector<ScriptParam> DayCycleProcess::Lua_DayCycleProcess_Create(Script& caller, std::vector<ScriptParam> vParams)
	{
		std::string err = Utils::TestParameters(vParams, "int int int int", true);
		if (err == "")
		{
			int startTimeH = vParams[0].getInt();
			int startTimeM = vParams[0].getInt();
			int endTimeH = vParams[0].getInt();
			int endTimeM = vParams[0].getInt();
			std::vector<ScriptParam> userParams;
			for (unsigned int i = 4; i < vParams.size(); i++)
				userParams.push_back(vParams[i]);
			std::shared_ptr<DayCycleProcess> process = ProcessNodeManager::Instance().CreateDayCycleProcess(userParams, startTimeH, startTimeM, endTimeH, endTimeM);
			SCRIPT_RETURNVALUE(process->GetProcessID())
		}
		else SCRIPT_RETURNERROR(err)
		std::vector<ScriptParam> out;
		return out;
	}

}