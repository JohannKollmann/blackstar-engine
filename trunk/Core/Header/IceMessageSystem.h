
#pragma once

#include <map>
#include "Ogre.h"
#include "IceIncludes.h"
#include "IceMsg.h"
#include "IceMessageListener.h"
#include "IceWrappedVector.h"
#include "boost/thread.hpp"

namespace Ice
{
	/*
	render thread (variable timestep)
	while (1)
	{
		MessageSystem::Instance().MulticastMessage(simulateMsg, Newsgroups::BEGIN_RENDERING);
		MessageSystem::Instance().ProcessMessages(AccessPermitionIDs::ACCESS_VIEW);
		MessageSystem::Instance().MulticastMessage(simulateMsg, Newsgroups::END_RENDERING);
		MessageSystem::Instance().ProcessMessages(AccessPermitionIDs::ACCESS_VIEW);
	}

	simulation thread (fixed timestep)
	while (1)
	{
		MessageSystem::Instance().ProcessMessages(AccessPermitionIDs::ACCESS_PHYSICS);
		MessageSystem::Instance().MulticastMessage(simulateMsg, AccessPermitionIDs::ACCESS_PHYSICS, Newsgroups::BEGIN_PHYSICS);
		MessageSystem::Instance().MulticastMessage(simulateMsg, AccessPermitionIDs::ACCESS_PHYSICS, Newsgroups::END_PHYSICS);
		MessageSystem::Instance().ProcessMessages(AccessPermitionIDs::ACCESS_ALL, true);	//exclusive
	}

	*/

	class DllExport MessageSystem
	{
		friend class MessageListener;

	private:
		struct Newsgroup
		{
			MsgTypeID groupID;
			std::map<AccessPermitionID, WrappedVector<MessageListener*> > receivers;
		};

		class DllExport MsgPacket
		{
		private:
			Msg mMsg;
			std::weak_ptr<MessageListener> mReceiver;
			WrappedVector<MessageListener*> *mReceivers;
			MessageListener *mReceiverRawPtr;
			bool mMulticast;

		public:
			MsgPacket(Msg &msg, WrappedVector<MessageListener*> *receivers)
			{
				mMsg = msg;
				mReceivers = receivers;
				mMulticast = true;
				mReceiverRawPtr = nullptr;
			}
			MsgPacket(Msg &msg, std::weak_ptr<MessageListener> &receiver)
			{
				mMsg = msg;
				mReceiver = receiver;
				mMulticast = false;
				mReceiverRawPtr = nullptr;
			}
			MsgPacket(Msg &msg, MessageListener* receiver)
			{
				mMsg = msg;
				mMulticast = false;
				mReceiverRawPtr = receiver;
			}
			~MsgPacket() {}
			void Send()
			{
				if (mMulticast)
				{
					for (mReceivers->Init(); mReceivers->HasNext();)
						mReceivers->GetNext()->ReceiveMessage(mMsg);
				}
				else if (mReceiverRawPtr)
				{
					mReceiverRawPtr->ReceiveMessage(mMsg);
				}
				else
				{
					std::shared_ptr<MessageListener> ptr = mReceiver.lock();
					if (ptr.get()) ptr->ReceiveMessage(mMsg);
				}
			}
		};

		static const int MAX_NUM_JOBQUEUES = 10;
	
		struct JobQueue
		{
			boost::mutex flushingMutex;
			std::vector<MsgPacket> cachedPackets[MAX_NUM_JOBQUEUES];
			std::vector<MsgPacket> packets;
		};

		JobQueue mCurrentJobs[MAX_NUM_JOBQUEUES];

		std::map<MsgTypeID, Newsgroup > mNewsgroupReceivers;

		boost::mutex mProcessingMsgCond;
		boost::mutex mProcessingMsg;
		int mNumProcessingMessages;

		MessageSystem() : mNumProcessingMessages(0) {}

		/**
		* Registers a message listener in a newsgroup.
		*/
		void JoinNewsgroup(MessageListener *listener, MsgTypeID groupname);

		/**
		* Removes a message listener from a Newsgroup.
		*/
		void QuitNewsgroup(MessageListener *listener, MsgTypeID groupname);

		/**
		* removes a message listener from all newsgroups.
		*/
		void QuitAllNewsgroups(MessageListener *listener) { QuitAllNewsgroups(listener, listener->GetAccessPermitionID()); }
		void QuitAllNewsgroups(MessageListener *listener, AccessPermitionID accessPermitionID);

	public:

		/**
		* Send a message to a message listener.
		* The message is delivered instantly when senderAccessPermitionID == receiver.AccessPermitionID or when ProcessMessages(receiver.AccessPermitionID) ist called.
		* @param lock specifies whether the receiver queue shall be locked, when senderAccessPermitionID != receiver.AccessPermitionID)
		*/
		void SendMessage(Msg &msg, AccessPermitionID senderAccessPermitionID, std::shared_ptr<MessageListener> &receiver);
		void SendMessage(Msg &msg, AccessPermitionID senderAccessPermitionID, MessageListener *receiver);

		/**
		* Sends a message to a message listener.
		* @param synchronized If set to true, the message es processed immediately and it is ensured that now other messages are processed in the meantime (avoid this if you can). 
		*/
		void SendMessage(Msg &msg, std::shared_ptr<MessageListener> &receiver);
		void SendMessage(Msg &msg, MessageListener *receiver);

		/**
		* Multicasts a message to all Message listeners that are registered member of groupID.
		* The message is delivered instantly when senderAccessPermitionID == group.AccessPermitionID or when ProcessMessages(group.AccessPermitionID) ist called.
		*/
		void MulticastMessage(Msg &msg, AccessPermitionID senderAccessPermitionID);

		/**
		* Multicasts a message to all Message listeners that are registered member of groupID (always asynchron).
		*/
		void MulticastMessage(Msg &msg);


		class DllExport ProcessingListener
		{
		public:
			virtual ~ProcessingListener() {}
			virtual void OnStartSending(AccessPermitionID accessPermitionID) {}
			virtual void OnFinishSending(AccessPermitionID accessPermitionID) {}
		};

		/**
		* Processes messages for all receivers with the AccessPermitionID accessPermitionID.
		* @param synchronized If set to true it is ensured that no other messages are processed in the meantime.
		* @remarks This method must be called from "outside" (mainloop or similar). NEVER call this as a message listener inside ReceiveMessage.
		*/
		void ProcessMessages(AccessPermitionID accessPermitionID, bool synchronized = false, ProcessingListener *listener = nullptr);

		void LockMessageProcessing();
		void UnlockMessageProcessing();

		/**
		* Creates a newsgroup with id groupID
		*/
		void CreateNewsgroup(MsgTypeID groupID);

		static MessageSystem& Instance();
	};

};