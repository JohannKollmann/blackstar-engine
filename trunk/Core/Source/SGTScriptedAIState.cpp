
#include "SGTScriptedAIState.h"

SGTScriptedAIState::SGTScriptedAIState(Ogre::String scriptFileName)
{
	mScript = SGTScriptSystem::GetInstance().CreateInstance(SCRIPT_BASE_DIR + scriptFileName);
}

SGTScriptedAIState::~SGTScriptedAIState()
{
}

void SGTScriptedAIState::OnEnter()
{
	std::vector<SGTScriptParam> params;
	params.push_back(SGTScriptParam(0));	//Hier muss die eindeutige ID hin
	mScript.CallFunction("OnEnter", params);
}

bool SGTScriptedAIState::OnUpdate()
{
	std::vector<SGTScriptParam> params;
	params.push_back(SGTScriptParam(0));	//Hier muss die eindeutige ID hin
	std::vector<SGTScriptParam> returns = mScript.CallFunction("OnUpdate", params);
	if (returns.size() == 0) return true;
	if ((*returns.begin()).getType() != SGTScriptParam::PARM_TYPE_BOOL) return true;
	return (*returns.begin()).getBool();
}