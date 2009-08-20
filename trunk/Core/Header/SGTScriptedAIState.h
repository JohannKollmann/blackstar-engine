
#pragma once

#include "SGTIncludes.h"
#include "SGTGOCAI.h"
#include "SGTScriptSystem.h"

class SGTScriptedAIState
{
protected:
	SGTGOCAI *mAIObject;
	SGTScript mScript;

public:
	SGTScriptedAIState(Ogre::String scriptFileName);
	~SGTScriptedAIState();

	void OnEnter();
	bool OnUpdate();
};