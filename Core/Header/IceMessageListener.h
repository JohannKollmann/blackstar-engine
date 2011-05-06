
#pragma once

#include "IceIncludes.h"
#include "IceMsg.h"

namespace Ice
{
	typedef int NewsgroupID;
	typedef int AccessPermitionID;

	enum AccessPermitions
	{
		ACCESS_VIEW = 0,		//access Ogre
		ACCESS_PHYSICS = 1,		//access physics engine
		ACCESS_NONE = 2,		//access nothing, only perform own computations (ai)
		ACCESS_ALL = 3			//access everything (scripting)
	};

	class DllExport MessageListener
	{
	protected:

		///Joins a newsgroup.
		void JoinNewsgroup(NewsgroupID groupID);

		///Leaves a newsgroup.
		void QuitNewsgroup(NewsgroupID groupID);

		///Leaves all joined newsgroups.
		void QuitAllNewsgroups();

		///Send a message to a certain receiver.
		void SendMessage(Msg &msg, MessageListener *receiver);

		///Multicasts a message.
		void MulticastMessage(Msg &msg, NewsgroupID groupID);

	public:
		MessageListener();

		virtual ~MessageListener();

		/**
		* This specifies what part of the engine you are allowed to access thread-safe inside ReceiveMessage.
		* It determines when you receive your messages, it also determines whether your sended messages are delivered immediately.
		*/
		virtual AccessPermitionID GetAccessPermitionID() = 0;

		/**
		* Handler for receiving a message. It is guaranteed that this is only called from th thread that processes your job context.
		*/
		virtual void ReceiveMessage(Msg &msg) = 0;
	};

	class ViewMessageListener : public MessageListener
	{
	public:
		virtual ~ViewMessageListener() {}

		AccessPermitionID GetAccessPermitionID() { return AccessPermitions::ACCESS_VIEW; }
	};

	class SimulationMessageListener : public MessageListener
	{
	public:
		virtual ~SimulationMessageListener() {}

		AccessPermitionID GetAccessPermitionID() { return AccessPermitions::ACCESS_NONE; }
	};

	class PhysicsMessageListener : public MessageListener
	{
	public:
		virtual ~PhysicsMessageListener() {}

		AccessPermitionID GetAccessPermitionID() { return AccessPermitions::ACCESS_PHYSICS; }
	};

	class SynchronizedMessageListener : public MessageListener
	{
	public:
		virtual ~SynchronizedMessageListener() {}

		AccessPermitionID GetAccessPermitionID() { return AccessPermitions::ACCESS_ALL; }
	};
};