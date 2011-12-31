
#pragma once

#include <map>
#include <set>
#include "Ogre.h"
#include "IceIncludes.h"
#include "IceMsg.h"
#include "IceMessageListener.h"
#include "IceWrappedVector.h"
#include "boost/thread.hpp"

namespace Ice
{
	class DllExport MessageSystem
	{
		friend class MessageListener;

	private:
		struct Newsgroup
		{
			MsgTypeID groupID;
			std::map<AccessPermissionID, WrappedVector<MessageListener*> > receivers;
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

		struct ThreadBinding
		{
			AccessPermissionID accessPermissionID;
			bool lockedMessageProcessing;
		};
	
		struct JobQueue
		{
			bool receiveAllMessagesInstantly;
			bool sendAllMessagesInstantly;
			bool processingMessages;			
			boost::mutex flushingMutex;
			std::vector<MsgPacket> cachedPackets[MAX_NUM_JOBQUEUES];
			std::vector<MsgPacket> packets;
		};

		JobQueue mCurrentJobs[MAX_NUM_JOBQUEUES];

		std::map<MsgTypeID, Newsgroup > mNewsgroupReceivers;

		std::map<boost::thread::id, ThreadBinding> mThreadBindings;
		std::map<AccessPermissionID, std::set<boost::thread::id> > mThreadAccessPermissions;

		int mNumProcessingMessages;
		int mNumWaitingSynchronized;
		bool mSynchronizedProcessing;
		boost::condition_variable mNoMessageProcessing;
		boost::condition_variable mConcurrentMessageProcessing;
		boost::mutex mMonitorMutex;
		boost::mutex mAtomicHelperMutex;	

		MessageSystem();

		///Tests whether the calling thread has the permission to access receiverID.
		bool testThreadAccessPermission(AccessPermissionID receiverID);

		///Sends a MsgPacket.
		void sendMsgPacket(MsgPacket &packet, AccessPermissionID receiverAccessPermission);

		///Retrieves whether the caller thread is currently processing messages.
		bool isThreadProcessingMessages();

		///Retrieves whether the caller thread has called LockMessageProcessing before and sets the lock status to a new value.
		bool updateThreadLockedMessageProcessing(bool lock);

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
		void QuitAllNewsgroups(MessageListener *listener) { QuitAllNewsgroups(listener, listener->GetAccessPermissionID()); }
		void QuitAllNewsgroups(MessageListener *listener, AccessPermissionID accessPermissionID);

	public:

		/**
		* Specifies that senders from the caller thread can access receiver with accessPermissionID directly.
		*/
		void AddThisThreadAccessPermission(AccessPermissionID accessPermissionID);

		/**
		* Specifies that all messages sent from the thread that is associated with accessPermissionID are sent directly.
		*/
		void SetSendAllMessagesInstantly(AccessPermissionID accessPermissionID, bool instant);

		/**
		* Specifies that all messages received by a thread that is associated with accessPermissionID are sent directly.
		*/
		void SetReceiveAllMessagesInstantly(AccessPermissionID accessPermissionID, bool instant);

		/**
		* Registers an accessPermissionID to the caller thread.
		*/
		void RegisterThread(boost::thread::id threadID, AccessPermissionID accessPermissionID);
		void RegisterThisThread(AccessPermissionID accessPermissionID);

		/**
		* Sends a message to a message listener.
		*/
		void SendMessage(Msg &msg, std::shared_ptr<MessageListener> &receiver, bool sendInstantly = false);
		void SendMessage(Msg &msg, MessageListener *receiver, bool sendInstantly = false);

		/**
		* Multicasts a message to all Message listeners that are registered member of groupID.
		* The message is delivered instantly when senderAccessPermissionID == group.AccessPermissionID or when ProcessMessages(group.AccessPermissionID) ist called.
		*/
		void MulticastMessage(Msg &msg, bool sendInstantly = false);

		class DllExport ProcessingListener
		{
		public:
			virtual ~ProcessingListener() {}
			virtual void OnStartSending(AccessPermissionID accessPermissionID) {}
			virtual void OnFinishSending(AccessPermissionID accessPermissionID) {}
		};

		/**
		* Processes messages for all receivers with the AccessPermissionID accessPermissionID.
		* @param synchronized If set to true it is ensured that no other messages are processed in the meantime.
		* @remarks This method must be called from "outside" (mainloop or similar). NEVER call this as a message listener inside ReceiveMessage.
		*/
		void ProcessMessages(AccessPermissionID accessPermissionID, bool synchronized = false, ProcessingListener *listener = nullptr);

		/**
		* Locks all message processing, processes all messages and unlocks message processing.
		* @remarks	Intended for special cases like reloading scripts/resources.
		*/
		void ProcessAllMessagesNow();

		void EnterMessageProcessing();
		void LeaveMessageProcessing();
		void LockMessageProcessing();
		void UnlockMessageProcessing();

		/**
		* Creates a newsgroup with id groupID
		*/
		void CreateNewsgroup(MsgTypeID groupID);

		static MessageSystem& Instance();
	};

}