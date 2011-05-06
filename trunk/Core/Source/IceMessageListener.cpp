
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
		MessageSystem::QuitAllNewsgroups(this);
	};

	void MessageListener::JoinNewsgroup(NewsgroupID groupID)
	{
		MessageSystem::JoinNewsgroup(this, groupID);
	}

	void MessageListener::QuitNewsgroup(NewsgroupID groupID)
	{
		MessageSystem::QuitNewsgroup(this, groupID);
	}

	void MessageListener::QuitAllNewsgroups()
	{
		MessageSystem::QuitAllNewsgroups(this);
	}

	void MessageListener::SendMessage(Msg &msg, MessageListener *receiver)
	{
		MessageSystem::SendMessage(msg, GetJobContextID(), receiver);
	}

	void MessageListener::MulticastMessage(Msg &msg, NewsgroupID groupID)
	{
		MessageSystem::MulticastMessage(msg, GetJobContextID(), groupID);
	}

};