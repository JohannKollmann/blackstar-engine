
#include "IceMessageListener.h"
#include "IceMsg.h"
#include "IceMessageSystem.h"

namespace Ice
{

	MessageListener::MessageListener()
	{
		mAccessPermitionID = 0;
	};
	MessageListener::~MessageListener()
	{
		MessageSystem::Instance().QuitAllNewsgroups(this, mAccessPermitionID);
	};

	void MessageListener::JoinNewsgroup(MsgTypeID groupID)
	{
		MessageSystem::Instance().JoinNewsgroup(this, groupID);
		mAccessPermitionID = GetAccessPermitionID();
	}

	void MessageListener::QuitNewsgroup(MsgTypeID groupID)
	{
		MessageSystem::Instance().QuitNewsgroup(this, groupID);
	}

	void MessageListener::QuitAllNewsgroups()
	{
		MessageSystem::Instance().QuitAllNewsgroups(this, mAccessPermitionID);
	}

	void MessageListener::SendMessage(Msg &msg, std::shared_ptr<MessageListener> &receiver)
	{
		MessageSystem::Instance().SendMessage(msg, GetAccessPermitionID(), receiver);
	}

	void MessageListener::MulticastMessage(Msg &msg)
	{
		MessageSystem::Instance().MulticastMessage(msg, GetAccessPermitionID());
	}

};