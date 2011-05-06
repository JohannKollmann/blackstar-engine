
#include "IceMessageSystem.h"
#include "IceMessageListener.h"

namespace Ice
{

	void MessageSystem::SendMessage(Msg &msg, AccessPermitionID senderAccessPermitionID, MessageListener *receiver, bool synchronized)
	{
		MsgPacket packet(msg, receiver);
		if (senderAccessPermitionID == receiver->GetAccessPermitionID()) packet.Send();
		else if (synchronized) sendSynchronizedPacket(packet);
		else mCurrentJobs[receiver->GetAccessPermitionID()].cachedPackets[senderAccessPermitionID].push_back(packet);
	};

	void MessageSystem::SendMessage(Msg &msg, MessageListener *receiver, bool synchronized)
	{
		MsgPacket packet(msg, receiver);
		if (synchronized) sendSynchronizedPacket(packet);
		else
		{
			mCurrentJobs[receiver->GetAccessPermitionID()].flushingMutex.lock();
			mCurrentJobs[receiver->GetAccessPermitionID()].packets.push_back(packet);
			mCurrentJobs[receiver->GetAccessPermitionID()].flushingMutex.unlock();
		}
	};

	void MessageSystem::MulticastMessage(Msg &msg, AccessPermitionID senderAccessPermitionID, NewsgroupID groupID, bool synchronized)
	{
		auto find = mNewsgroupReceivers.find(groupID);
		if (find == mNewsgroupReceivers.end())
		{
			IceWarning("Group ID '" + Ogre::StringConverter::toString(groupID) + "' does not exist!");
			return;
		}

		ITERATE(i, find->second.receivers)
		{
			MsgPacket packet(msg,  &i->second);
			if (i->first == senderAccessPermitionID) packet.Send();
			else if (synchronized) sendSynchronizedPacket(packet);
			else mCurrentJobs[i->first].cachedPackets[senderAccessPermitionID].push_back(packet);
		}
	};

	void MessageSystem::MulticastMessage(Msg &msg, NewsgroupID groupID, bool synchronized)
	{
		auto find = mNewsgroupReceivers.find(groupID);
		if (find == mNewsgroupReceivers.end())
		{
			IceWarning("Group ID '" + Ogre::StringConverter::toString(groupID) + "' does not exist!");
			return;
		}

		ITERATE(i, find->second.receivers)
		{
			MsgPacket packet(msg,  &i->second);
			if (synchronized) sendSynchronizedPacket(packet);
			else
			{
				mCurrentJobs[i->first].flushingMutex.lock();
				mCurrentJobs[i->first].packets.push_back(packet);
				mCurrentJobs[i->first].flushingMutex.unlock();
			}
		}
	};

	void MessageSystem::sendSynchronizedPacket(MessageSystem::MsgPacket &packet, int senderID)
	{
		for (int i = 0; i < MAX_NUM_JOBQUEUES; i++)
			if (i != senderID) mCurrentJobs[i].processingMutex.lock();
		packet.Send();
		for (int i = 0; i < MAX_NUM_JOBQUEUES; i++)
			if (i != senderID) mCurrentJobs[i].processingMutex.unlock();
	}

	void MessageSystem::ProcessMessages(AccessPermitionID accessPermitionID, bool synchronized)
	{
		if (synchronized) mProcessingMsg.lock();	//wait until no other thread processes messages
		else
		{
			mProcessingMsgCond.lock();
			if (mNumProcessingMessages == 0) mProcessingMsg.lock();		//indicate set a thread is processing messages right now
			mNumProcessingMessages++;
			mProcessingMsgCond.unlock();
		}

		mCurrentJobs[accessPermitionID].processingMutex.lock();		//indicate set this specific thread is processing messages right now. 

		mCurrentJobs[accessPermitionID].flushingMutex.lock();
		std::vector<MsgPacket> msgCopy = mCurrentJobs[accessPermitionID].packets;	//copy
		mCurrentJobs[accessPermitionID].packets.clear();	//flush
		mCurrentJobs[accessPermitionID].flushingMutex.unlock();

		ITERATE(i, msgCopy) i->Send();		//deliver messages

		//send cached messages
		for (int i = 0; i < MAX_NUM_JOBQUEUES; i++)
		{
			mCurrentJobs[i].flushingMutex.lock();
			mCurrentJobs[i].packets.insert(mCurrentJobs[i].packets.end(), mCurrentJobs[i].cachedPackets[accessPermitionID].begin(), mCurrentJobs[i].cachedPackets[accessPermitionID].end());
			mCurrentJobs[i].flushingMutex.unlock();
		}

		mCurrentJobs[accessPermitionID].processingMutex.unlock();

		if (synchronized) mProcessingMsg.unlock();
		else
		{
			mProcessingMsgCond.lock();
			mNumProcessingMessages--;
			if (mNumProcessingMessages == 0) mProcessingMsg.unlock();
			mProcessingMsgCond.unlock();
		}
	}

	void MessageSystem::CreateNewsgroup(NewsgroupID groupID)
	{
		auto find = mNewsgroupReceivers.find(groupID);
		if (find != mNewsgroupReceivers.end())
		{
			IceWarning("Error while creating Newsgroup '" + Ogre::StringConverter::toString(groupID) + "'. Group already exists!");
			return;
		}

		Newsgroup group;
		group.groupID = groupID;
		mNewsgroupReceivers.insert(std::make_pair<NewsgroupID, Newsgroup >(groupID, group));

		IceNote("Created Newsgroup \"" + Ogre::StringConverter::toString(groupID) + "\"");
	};

	void MessageSystem::JoinNewsgroup(MessageListener *listener, NewsgroupID groupID)
	{
		auto find = mNewsgroupReceivers.find(groupID);
		if (find == mNewsgroupReceivers.end())
		{
			IceWarning("Group '" + Ogre::StringConverter::toString(groupID) + "' does not exist!");
			return;
		}

		std::vector<MessageListener*> receivers =  find->second.receivers[listener->GetAccessPermitionID()].GetVector();
		auto li = receivers.begin();
		IceAssert(std::find(receivers.begin(), receivers.end(), listener) == receivers.end())
		receivers.push_back(listener);
	};

	void MessageSystem::QuitNewsgroup(MessageListener *listener, NewsgroupID groupID)
	{
		auto find = mNewsgroupReceivers.find(groupID);
		if (find == mNewsgroupReceivers.end())
		{
			IceWarning("Group '" + Ogre::StringConverter::toString(groupID) + "' does not exist!");
			return;
		}

		WrappedVector<MessageListener*> receivers =  find->second.receivers[listener->GetAccessPermitionID()];

		unsigned int index = 0;
		for (unsigned int i = 0; i < receivers.GetVector().size(); i++)
		{
			if (receivers.GetVector()[i] == listener)
			{
				receivers.Remove(i);
				break;
			}
			index++;
		}
	};

	void MessageSystem::QuitAllNewsgroups(MessageListener *listener)
	{
		for (auto ni = mNewsgroupReceivers.begin(); ni != mNewsgroupReceivers.end(); ++ni)
		{
			WrappedVector<MessageListener*> receivers = ni->second.receivers[listener->GetAccessPermitionID()];
			unsigned int index = 0;
			for (unsigned int i = 0; i < receivers.GetVector().size(); i++)
			{
				if (receivers.GetVector()[i] == listener)
				{
					receivers.Remove(i);
					break;
				}
				index++;
			}
		}
	}

};