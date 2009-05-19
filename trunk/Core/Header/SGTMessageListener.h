
#ifndef __SGTMessageListener_H__
#define __SGTMessageListener_H__

#include "SGTIncludes.h"
#include "SGTMsg.h"

class SGTDllExport SGTMessageListener
{
public:
	SGTMessageListener();

	virtual ~SGTMessageListener();

	virtual void ReceiveMessage(SGTMsg &msg) = 0;

	virtual float GetListenerPriority() { return 1.0f; };
};

#endif