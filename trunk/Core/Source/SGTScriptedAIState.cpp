
#include "SGTScriptedAIState.h"
#include "mmsystem.h"


SGTScriptedAIState::SGTScriptedAIState(SGTGOCAI* ai, Ogre::String scriptFileName)
{
	mLastUpdateCall = 0.0f;
	mAIObject = ai;
	mScript = SGTScriptSystem::GetInstance().CreateInstance(SCRIPT_BASE_DIR + scriptFileName);
}

SGTScriptedAIState::~SGTScriptedAIState()
{
}

void SGTScriptedAIState::OnEnter()
{
	std::vector<SGTScriptParam> params;
	params.push_back(SGTScriptParam((int)mAIObject->GetID()));
	mScript.CallFunction("onEnter", params);
}

bool SGTScriptedAIState::OnUpdate(float time)
{
	if (OnSimulate(time)) return true;
	if (timeGetTime() - mLastUpdateCall > 0.1f)
	{
		mLastUpdateCall = timeGetTime();
		std::vector<SGTScriptParam> params;
		params.push_back(SGTScriptParam((int)mAIObject->GetID()));
		std::vector<SGTScriptParam> returns = mScript.CallFunction("onUpdate", params);
		if (returns.size() == 0) return false;
		if ((*returns.begin()).getType() != SGTScriptParam::PARM_TYPE_BOOL) return false;
		return (*returns.begin()).getBool();
	}
	return false;
}



SGTDayCycle::SGTDayCycle(SGTGOCAI* ai, Ogre::String scriptFileName, std::vector<SGTScriptParam> params, int endtimeH, int endtimeM, bool time_abs)
{
	mLastUpdateCall = 0.0f;
	mAIObject = ai;
	mScript = SGTScriptSystem::GetInstance().CreateInstance(SCRIPT_BASE_DIR + scriptFileName);
	mScriptParams = params;
	mEndTimeH = endtimeH;
	mEndTimeM = endtimeM;
	mStartTimeH = SGTSceneManager::Instance().GetHour();
	mStartTimeM = SGTSceneManager::Instance().GetMinutes();
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

SGTDayCycle::~SGTDayCycle()
{
}

void SGTDayCycle::OnEnter()
{
	std::vector<SGTScriptParam> params;
	params.push_back(SGTScriptParam((int)mAIObject->GetID()));
	for (std::vector<SGTScriptParam>::iterator i = mScriptParams.begin(); i != mScriptParams.end(); i++)
	{
		params.push_back((*i));
	}
	mScriptParams.clear();
	mScriptParams = params;
	mScript.CallFunction("onEnter", mScriptParams);
}

bool SGTDayCycle::OnUpdate(float time)
{
	int hour = SGTSceneManager::Instance().GetHour();
	int minutes = SGTSceneManager::Instance().GetMinutes();
	if (hour == mEndTimeH && minutes > mEndTimeM) return true;

	if (timeGetTime() - mLastUpdateCall > 0.1f)
	{
		mLastUpdateCall = timeGetTime();
		std::vector<SGTScriptParam> returns = mScript.CallFunction("OnUpdate", mScriptParams);

		if (returns.size() == 0) return false;
		if ((*returns.begin()).getType() != SGTScriptParam::PARM_TYPE_BOOL) return false;
		return (*returns.begin()).getBool();
	}
	return false;
}