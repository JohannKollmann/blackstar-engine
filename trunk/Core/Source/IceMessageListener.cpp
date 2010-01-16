
#include "IceMessageListener.h"
#include "IceMsg.h"
#include "IceMessageSystem.h"

namespace Ice
{

MessageListener::MessageListener()
{
	MessageSystem::Instance().JoinNewsgroup(this, "COMMON");
};
MessageListener::~MessageListener()
{
	MessageSystem::Instance().QuitNewsgroup(this, "COMMON");
};

};