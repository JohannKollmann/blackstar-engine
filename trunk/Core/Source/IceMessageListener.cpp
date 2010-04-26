
#include "IceMessageListener.h"
#include "IceMsg.h"
#include "IceMessageSystem.h"

namespace Ice
{

	MessageListener::MessageListener()
	{
	};
	MessageListener::~MessageListener()
	{
		MessageSystem::Instance().QuitAllNewsgroups(this);
		//MessageSystem::Instance().QuitNewsgroup(this, "COMMON");
	};

};