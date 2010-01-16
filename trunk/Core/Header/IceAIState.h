
#pragma once

#include "IceIncludes.h"

namespace Ice
{

class AIState
{
protected:
	GOCAI *mAIObject;
	AIState() { };

public:
	virtual ~AIState() {};

	void SetAI(GOCAI *ai) { mAIObject = ai; };

	virtual float GetPriority() { return 0.0f; };

	virtual void OnEnter() {};
	virtual bool OnUpdate(float time) = 0;
	virtual void OnLeave() {};
};

};