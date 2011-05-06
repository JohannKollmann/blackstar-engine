
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

	enum GlobalMessageIDs
	{
		UPDATE_PER_FRAME,													//called per frame
		BEGIN_PHYSICS, PHYSICS_SUBSTEP, END_PHYSICS,						//called per physics update (can be multiple times per frame)
		KEY_DOWN, KEY_UP, MOUSE_DOWN, MOUSE_UP, MOUSE_MOVE,					//user input
		MATERIAL_CONTACT, ACTOR_ONSLEEP, ACTOR_ONWAKE,						//physics callbacks
		REPARSE_SCRIPT_PRE, REPARSE_SCRIPTS_POST,							//reload scripts, reload materials
		LOADLEVEL_BEGIN, LOADLEVEL_END,	SAVELEVEL_BEGIN, SAVELEVEL_END,		//load level, save level
		ENABLE_GAME_CLOCK
	};

	/*
	render thread (variable timestep)
	while (1)
	{
		MessageSystem::MulticastMessage(simulateMsg, Newsgroups::BEGIN_RENDERING);
		MessageSystem::ProcessMessages(AccessPermitionIDs::ACCESS_VIEW);
		MessageSystem::MulticastMessage(simulateMsg, Newsgroups::END_RENDERING);
		MessageSystem::ProcessMessages(AccessPermitionIDs::ACCESS_VIEW);
	}

	simulation thread (fixed timestep)
	while (1)
	{
		MessageSystem::ProcessMessages(AccessPermitionIDs::ACCESS_PHYSICS);
		MessageSystem::MulticastMessage(simulateMsg, AccessPermitionIDs::ACCESS_PHYSICS, Newsgroups::BEGIN_PHYSICS);
		MessageSystem::MulticastMessage(simulateMsg, AccessPermitionIDs::ACCESS_PHYSICS, Newsgroups::END_PHYSICS);
		MessageSystem::ProcessMessages(AccessPermitionIDs::ACCESS_ALL, true);	//exclusive
	}

	*/

	class DllExport MessageSystem
	{
	private:
		struct Newsgroup
		{
			NewsgroupID groupID;
			std::map<AccessPermitionID, WrappedVector<MessageListener*> > receivers;
		};

		class DllExport MsgPacket
		{
		private:
			Msg mMsg;
			MessageListener *mReceiver;
			WrappedVector<MessageListener*> *mReceivers;
			bool mMulticast;

		public:
			MsgPacket(Msg &msg, WrappedVector<MessageListener*> *receivers)
			{
				mMsg = msg;
				mReceivers = receivers;
				mMulticast = true;
			}
			MsgPacket(Msg &msg, MessageListener *receiver)
			{
				mMsg = msg;
				mReceiver = receiver;
				mMulticast = false;
			}
			~MsgPacket() {}
			void Send()
			{
				if (mMulticast)
				{
					for (mReceivers->Init(); mReceivers->HasNext();)
						mReceivers->GetNext()->ReceiveMessage(mMsg);
				}
				else
				{
					mReceiver->ReceiveMessage(mMsg);
				}
			}
		};

		static const int MAX_NUM_JOBQUEUES = 10;
	
		struct JobQueue
		{
			boost::mutex flushingMutex;
			boost::mutex processingMutex;
			std::vector<MsgPacket> cachedPackets[MAX_NUM_JOBQUEUES];
			std::vector<MsgPacket> packets;
		};

		static JobQueue mCurrentJobs[MAX_NUM_JOBQUEUES];

		static std::map<NewsgroupID, Newsgroup > mNewsgroupReceivers;

		static boost::mutex mProcessingMsgCond;
		static boost::mutex mProcessingMsg;
		static int mNumProcessingMessages;

		static void sendSynchronizedPacket(MsgPacket &packet, int sender = -1);

	public:

		///Must be called at startup.
		static void Init() { mNumProcessingMessages = 0; }

		/**
		* Send a message to a message listener.
		* The message is delivered instantly when senderAccessPermitionID == receiver.AccessPermitionID or when ProcessMessages(receiver.AccessPermitionID) ist called.
		* @param lock specifies whether the receiver queue shall be locked, when senderAccessPermitionID != receiver.AccessPermitionID)
		*/
		static void SendMessage(Msg &msg, AccessPermitionID senderAccessPermitionID, MessageListener *receiver, bool synchronized = false);

		/**
		* Sends a message to a message listener.
		* @param synchronized If set to true, the message es processed immediately and it is ensured that now other messages are processed in the meantime (avoid this if you can). 
		*/
		static void SendMessage(Msg &msg, MessageListener *receiver, bool synchronized = false);

		/**
		* Multicasts a message to all Message listeners that are registered member of groupID.
		* The message is delivered instantly when senderAccessPermitionID == group.AccessPermitionID or when ProcessMessages(group.AccessPermitionID) ist called.
		*/
		static void MulticastMessage(Msg &msg, AccessPermitionID senderAccessPermitionID, NewsgroupID groupID, bool synchronized = false);

		/**
		* Multicasts a message to all Message listeners that are registered member of groupID (always asynchron).
		*/
		static void MulticastMessage(Msg &msg, NewsgroupID groupID, bool synchronized = false);

		/**
		* Processes messages for all receivers with the AccessPermitionID accessPermitionID.
		* @param synchronized If set to true it is ensured that no other messages are processed in the meantime.
		*/
		static void ProcessMessages(AccessPermitionID accessPermitionID, bool synchronized = false);

		/**
		* Creates a newsgroup with id groupID
		*/
		static void CreateNewsgroup(NewsgroupID groupID);

		/**
		* Registers a message listener in a newsgroup.
		*/
		static void JoinNewsgroup(MessageListener *listener, NewsgroupID groupname);

		/**
		* Removes a message listener from a Newsgroup.
		*/
		static void QuitNewsgroup(MessageListener *listener, NewsgroupID groupname);

		/**
		* removes a message listener from all newsgroups.
		*/
		static void QuitAllNewsgroups(MessageListener *listener);
	};

};