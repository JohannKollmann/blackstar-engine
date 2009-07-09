
#pragma once

#include "SGTIncludes.h"

class SGTAIQueueEntry
{
private:
	void Finished();

public:
	SGTAIQueueEntry();
	virtual ~SGTAIQueueEntry();

	virtual float GetPriority() { 0.0f; };

	virtual void OnUpdate(float time) {};
};