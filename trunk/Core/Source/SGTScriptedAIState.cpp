
#include "SGTScriptedAIState.h"

SGTScriptedAIState::SGTScriptedAIState(SGTGOCAI* ai, Ogre::String scriptFileName, int endtimeH, int endtimeM, bool time_abs)
{
	mAIObject = ai;
	mScript = SGTScriptSystem::GetInstance().CreateInstance(SCRIPT_BASE_DIR + scriptFileName);
	mEndTimeH = endtimeH;
	mEndTimeM = endtimeM;
	mStartTimeH = 8;
	mStartTimeM = 0;
	mTimeAbs = time_abs;
}

SGTScriptedAIState::~SGTScriptedAIState()
{
}

void SGTScriptedAIState::OnEnter()
{
	std::vector<SGTScriptParam> params;
	params.push_back(SGTScriptParam((int)mAIObject->GetID()));
	mScript.CallFunction("OnEnter", params);
}

bool SGTScriptedAIState::OnUpdate()
{
	std::vector<SGTScriptParam> params;
	params.push_back(SGTScriptParam((int)mAIObject->GetID()));
	std::vector<SGTScriptParam> returns = mScript.CallFunction("OnUpdate", params);
	if (returns.size() == 0) return true;
	if ((*returns.begin()).getType() != SGTScriptParam::PARM_TYPE_BOOL) return true;
	return (*returns.begin()).getBool();
}