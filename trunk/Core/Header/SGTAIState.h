
#pragma once

#include "SGTIncludes.h"

class SGTAIState
{
protected:
	SGTGOCAI *mAIObject;
	SGTAIState() { };

public:
	virtual ~SGTAIState() {};

	void SetAI(SGTGOCAI *ai) { mAIObject = ai; };

	virtual float GetPriority() { return 0.0f; };

	virtual void OnEnter() {};
	virtual bool OnUpdate(float time) = 0;
	virtual void OnLeave() {};
};

class SGTScriptedAIState
{
protected:
	SGTGOCAI *mAIObject;

public:
	SGTScriptedAIState () {};
	~SGTScriptedAIState () {};

	void OnEnter () {};
	bool OnUpdate () { return false; };
};