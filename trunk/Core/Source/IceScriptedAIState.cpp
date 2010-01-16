
#include "IceScriptedAIState.h"
#include "mmsystem.h"
#include "IceSceneManager.h"

namespace Ice
{

ScriptedAIState::ScriptedAIState(GOCAI* ai, Ogre::String scriptFileName)
{
	mLastUpdateCall = 0.0f;
	mAIObject = ai;
	mScript = ScriptSystem::GetInstance().CreateInstance(scriptFileName);
}

ScriptedAIState::~ScriptedAIState()
{
}

void ScriptedAIState::OnEnter()
{
	std::vector<ScriptParam> params;
	params.push_back(ScriptParam((int)mAIObject->GetID()));
	mScript.CallFunction("onEnter", params);
}

bool ScriptedAIState::OnUpdate(float time)
{
	if (OnSimulate(time)) return true;
	if (timeGetTime() - mLastUpdateCall > 0.1f)
	{
		mLastUpdateCall = timeGetTime();
		std::vector<ScriptParam> params;
		params.push_back(ScriptParam((int)mAIObject->GetID()));
		std::vector<ScriptParam> returns = mScript.CallFunction("onUpdate", params);
		if (returns.size() == 0) return false;
		if ((*returns.begin()).getType() != ScriptParam::PARM_TYPE_BOOL) return false;
		return (*returns.begin()).getBool();
	}
	return false;
}



DayCycle::DayCycle(GOCAI* ai, Ogre::String scriptFileName, std::vector<ScriptParam> params, int endtimeH, int endtimeM, bool time_abs)
{
	mLastUpdateCall = 0.0f;
	mAIObject = ai;
	mScript = ScriptSystem::GetInstance().CreateInstance(scriptFileName);
	mScriptParams = params;
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

DayCycle::~DayCycle()
{
}

void DayCycle::OnEnter()
{
	std::vector<ScriptParam> params;
	params.push_back(ScriptParam((int)mAIObject->GetID()));
	for (std::vector<ScriptParam>::iterator i = mScriptParams.begin(); i != mScriptParams.end(); i++)
	{
		params.push_back((*i));
	}
	mScriptParams.clear();
	mScriptParams = params;
	mScript.CallFunction("onEnter", mScriptParams);
}

bool DayCycle::OnUpdate(float time)
{
	int hour = SceneManager::Instance().GetHour();
	int minutes = SceneManager::Instance().GetMinutes();
	if (hour == mEndTimeH && minutes > mEndTimeM) return true;

	if (timeGetTime() - mLastUpdateCall > 0.1f)
	{
		mLastUpdateCall = timeGetTime();
		std::vector<ScriptParam> returns = mScript.CallFunction("onUpdate", mScriptParams);

		if (returns.size() == 0) return false;
		if ((*returns.begin()).getType() != ScriptParam::PARM_TYPE_BOOL) return false;
		return (*returns.begin()).getBool();
	}
	return false;
}

};