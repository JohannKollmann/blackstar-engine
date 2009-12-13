
#pragma once

#include "SGTScriptSystem.h"
#include "SGTMessageSystem.h"

class SGTScriptedCollisionCallback : public SGTMessageListener
{
private:
	SGTScript mScript;

public:
	SGTScriptedCollisionCallback(void);
	~SGTScriptedCollisionCallback(void);

	void ReceiveMessage(SGTMsg &msg);
};