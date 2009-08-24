
#include "SGTScriptedAIState.h"

SGTScriptedAIState::SGTScriptedAIState(SGTGOCAI* ai, Ogre::String scriptFileName, Ogre::String waypoint, int endtimeH, int endtimeM, bool time_abs)
{
	mAIObject = ai;
	mScript = SGTScriptSystem::GetInstance().CreateInstance(SCRIPT_BASE_DIR + scriptFileName);
	mWaypoint = waypoint;
	mEndTimeH = endtimeH;
	mEndTimeM = endtimeM;
	mStartTimeH = SGTSceneManager::Instance().GetHour();
	mStartTimeM = SGTSceneManager::Instance().GetMinutes();
	if (time_abs)
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

SGTScriptedAIState::~SGTScriptedAIState()
{
}

void SGTScriptedAIState::OnEnter()
{
	std::vector<SGTScriptParam> params;
	params.push_back(SGTScriptParam((int)mAIObject->GetID()));
	params.push_back(SGTScriptParam(mWaypoint));
	mScript.CallFunction("OnEnter", params);
}

bool SGTScriptedAIState::OnUpdate()
{
	int hour = SGTSceneManager::Instance().GetHour();
	int minutes = SGTSceneManager::Instance().GetMinutes();
	if (hour == mEndTimeH && minutes > mEndTimeM) return true;
	std::vector<SGTScriptParam> params;
	params.push_back(SGTScriptParam((int)mAIObject->GetID()));
	params.push_back(SGTScriptParam(mWaypoint));
	std::vector<SGTScriptParam> returns = mScript.CallFunction("OnUpdate", params);
	if (returns.size() == 0) return true;
	if ((*returns.begin()).getType() != SGTScriptParam::PARM_TYPE_BOOL) return true;
	return (*returns.begin()).getBool();
}