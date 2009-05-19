
#pragma once

#include "SGTNpc.h"
#include "SGTIncludes.h"

class SGTAIQueueEntry
{
private:
	void Finished();
	SGTNpc *mParentNpc;

public:
	SGTAIQueueEntry();
	virtual ~SGTAIQueueEntry();

	virtual float GetPriority() { 0.0f; };

	virtual void OnUpdate(float time) {};
};