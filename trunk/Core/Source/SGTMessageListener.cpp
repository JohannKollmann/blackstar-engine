
#include "SGTMessageListener.h"
#include "SGTMsg.h"
#include "SGTMessageSystem.h"

SGTMessageListener::SGTMessageListener()
{
	SGTMessageSystem::Instance().JoinNewsgroup(this, "COMMON");
};
SGTMessageListener::~SGTMessageListener()
{
	SGTMessageSystem::Instance().QuitNewsgroup(this, "COMMON");
};