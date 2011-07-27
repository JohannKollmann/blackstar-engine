
#pragma once

#include "IceIncludes.h"
#include "IceMsg.h"

namespace Ice
{
	typedef int AccessPermissionID;

	class DllExport MessageListener
	{
	private:
		AccessPermissionID mAccessPermissionID;

	public:

		///Joins a newsgroup.
		void JoinNewsgroup(MsgTypeID groupID);

		///Leaves a newsgroup.
		void QuitNewsgroup(MsgTypeID groupID);

		///Leaves all joined newsgroups.
		void QuitAllNewsgroups();

		///Send a message to a certain receiver.
		void SendMessage(Msg &msg, std::shared_ptr<MessageListener> &receiver);

		///Multicasts a message.
		void MulticastMessage(Msg &msg);

		MessageListener();

		virtual ~MessageListener();

		/**
		* This specifies what part of the engine you are allowed to access thread-safe inside ReceiveMessage.
		* It determines when you receive your messages, it also determines whether your sended messages are delivered immediately.
		*/
		virtual AccessPermissionID GetAccessPermissionID() = 0;

		/**
		* Handler for receiving a message. It is guaranteed that this is only called from th thread that processes your job context.
		*/
		virtual void ReceiveMessage(Msg &msg) = 0;
	};

	class ViewMessageListener : public MessageListener
	{
	public:
		virtual ~ViewMessageListener() {}

		AccessPermissionID GetAccessPermissionID() { return AccessPermissions::ACCESS_VIEW; }
	};

	class IndependantMessageListener : public MessageListener
	{
	public:
		virtual ~IndependantMessageListener() {}

		AccessPermissionID GetAccessPermissionID() { return AccessPermissions::ACCESS_NONE; }
	};

	class PhysicsMessageListener : public MessageListener
	{
	public:
		virtual ~PhysicsMessageListener() {}

		AccessPermissionID GetAccessPermissionID() { return AccessPermissions::ACCESS_PHYSICS; }
	};

	class SynchronizedMessageListener : public MessageListener
	{
	public:
		virtual ~SynchronizedMessageListener() {}

		AccessPermissionID GetAccessPermissionID() { return AccessPermissions::ACCESS_ALL; }
	};
};