
#pragma once

#include "SGTIncludes.h"
#include "SGTGOCAI.h"

class SGTScriptedAIState
{
protected:
	SGTGOCAI *mAIObject;

public:
	SGTScriptedAIState();
	~SGTScriptedAIState();

	void OnEnter();
	bool OnUpdate();
};