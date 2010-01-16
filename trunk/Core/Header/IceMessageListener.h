
#pragma once

#include "IceIncludes.h"
#include "IceMsg.h"

namespace Ice
{

class DllExport MessageListener
{
public:
	MessageListener();

	virtual ~MessageListener();

	virtual void ReceiveMessage(Msg &msg) = 0;

	virtual float GetListenerPriority() { return 1.0f; };
};

};