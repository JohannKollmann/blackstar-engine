
#include "IceScriptedProcess.h"
#include "mmsystem.h"
#include "IceSceneManager.h"
#include "IceProcessNodeManager.h"

namespace Ice
{

	ScriptedProcess::ScriptedProcess(std::weak_ptr<GameObject> &gameObject, std::vector<ScriptParam> params)
	{
		mLastUpdateCall = 0.0f;
		mGameObject = gameObject;

		mScriptParams.push_back(ScriptParam((int)mGameObject.lock()->GetID()));
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


	DayCycleProcess::DayCycleProcess(std::weak_ptr<GameObject> &object, std::vector<ScriptParam> params, int endtimeH, int endtimeM, bool time_abs)
	: ScriptedProcess(object, params)
	{
		mLastUpdateCall = 0.0f;
		mEndTimeH = endtimeH;
		mEndTimeM = endtimeM;
		mStartTimeH = SceneManager::Instance().GetHour();
		mStartTimeM = SceneManager::Instance().GetMinutes();
		if (!time_abs)
		{
			mEndTimeM += mStartTimeH;
			if (mEndTimeM > 60)
			{
				mEndTimeM -= 60;
				mStartTimeH++;
				if (mEndTimeH > 24) mEndTimeH -= 24;
			}
			mEndTimeH += mStartTimeH; if (mEndTimeH > 24) mEndTimeH -= 24;
		}
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

}