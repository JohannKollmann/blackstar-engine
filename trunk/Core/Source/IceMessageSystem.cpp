
#include "IceMessageSystem.h"
#include "IceMessageListener.h"

namespace Ice
{

	void MessageSystem::SendMessage(Msg &msg, AccessPermitionID senderAccessPermitionID, std::shared_ptr<MessageListener> &receiver)
	{
		MsgPacket packet(msg, std::weak_ptr<MessageListener>(receiver));
		if (senderAccessPermitionID == receiver->GetAccessPermitionID() || senderAccessPermitionID == AccessPermitions::ACCESS_ALL) packet.Send();
		else mCurrentJobs[receiver->GetAccessPermitionID()].cachedPackets[senderAccessPermitionID].push_back(packet);
	};
	void MessageSystem::SendMessage(Msg &msg, AccessPermitionID senderAccessPermitionID, MessageListener *receiver)
	{
		MsgPacket packet(msg, receiver);
		if (senderAccessPermitionID == receiver->GetAccessPermitionID() || senderAccessPermitionID == AccessPermitions::ACCESS_ALL) packet.Send();
		else mCurrentJobs[receiver->GetAccessPermitionID()].cachedPackets[senderAccessPermitionID].push_back(packet);
	};

	void MessageSystem::SendMessage(Msg &msg, std::shared_ptr<MessageListener> &receiver)
	{
		MsgPacket packet(msg, std::weak_ptr<MessageListener>(receiver));
		mCurrentJobs[receiver->GetAccessPermitionID()].flushingMutex.lock();
		mCurrentJobs[receiver->GetAccessPermitionID()].packets.push_back(packet);
		mCurrentJobs[receiver->GetAccessPermitionID()].flushingMutex.unlock();
	};
	void MessageSystem::SendMessage(Msg &msg, MessageListener *receiver)
	{
		MsgPacket packet(msg, receiver);
		mCurrentJobs[receiver->GetAccessPermitionID()].flushingMutex.lock();
		mCurrentJobs[receiver->GetAccessPermitionID()].packets.push_back(packet);
		mCurrentJobs[receiver->GetAccessPermitionID()].flushingMutex.unlock();
	};

	void MessageSystem::MulticastMessage(Msg &msg, AccessPermitionID senderAccessPermitionID)
	{
		auto find = mNewsgroupReceivers.find(msg.typeID);
		if (find == mNewsgroupReceivers.end())
		{
			IceWarning("Group ID '" + Ogre::StringConverter::toString(msg.typeID) + "' does not exist!");
			return;
		}

		ITERATE(i, find->second.receivers)
		{
			MsgPacket packet(msg,  &i->second);
			if (i->first == senderAccessPermitionID || senderAccessPermitionID == AccessPermitions::ACCESS_ALL)
				packet.Send();
			else mCurrentJobs[i->first].cachedPackets[senderAccessPermitionID].push_back(packet);
		}
	};

	void MessageSystem::MulticastMessage(Msg &msg)
	{
		auto find = mNewsgroupReceivers.find(msg.typeID);
		if (find == mNewsgroupReceivers.end())
		{
			IceWarning("Group ID '" + Ogre::StringConverter::toString(msg.typeID) + "' does not exist!");
			return;
		}

		ITERATE(i, find->second.receivers)
		{
			MsgPacket packet(msg,  &i->second);
			mCurrentJobs[i->first].flushingMutex.lock();
			mCurrentJobs[i->first].packets.push_back(packet);
			mCurrentJobs[i->first].flushingMutex.unlock();
		}
	};

	void MessageSystem::LockMessageProcessing()
	{
		mNumWaitingSynchronized++;
		boost::unique_lock<boost::mutex> lock(mNoMessageProcessingMutex);
		while (mNumProcessingMessages > 0 || mSynchronizedProcessing)
		{
			mNoMessageProcessing.wait(lock);	//wait until no other thread processes messages
		}
		mNumWaitingSynchronized--;
		mSynchronizedProcessing = true;
	}
	void MessageSystem::UnlockMessageProcessing()
	{
		mConcurrentMessageProcessingMutex.lock();
		mSynchronizedProcessing = false;
		if (mNumWaitingSynchronized == 0) mConcurrentMessageProcessing.notify_all();
		else mNoMessageProcessing.notify_one();
		mConcurrentMessageProcessingMutex.unlock();
	}

	void MessageSystem::ProcessMessages(AccessPermitionID accessPermitionID, bool synchronized, ProcessingListener *listener)
	{
		if (synchronized) LockMessageProcessing();
		else
		{
			boost::unique_lock<boost::mutex> lock(mConcurrentMessageProcessingMutex);
			while (mNumWaitingSynchronized > 0 || mSynchronizedProcessing)
			{
				mConcurrentMessageProcessing.wait(lock);	//wait until no other thread processes messages
			}
			mNumProcessingMessages++;
		}

		if (listener) listener->OnStartSending(accessPermitionID);

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
			mCurrentJobs[i].cachedPackets[accessPermitionID].clear();
			mCurrentJobs[i].flushingMutex.unlock();
		}

		if (listener) listener->OnFinishSending(accessPermitionID);

		if (synchronized) UnlockMessageProcessing();
		else
		{
			mNoMessageProcessingMutex.lock();
			mNumProcessingMessages--;
			if (mNumProcessingMessages == 0) mNoMessageProcessing.notify_one();
			mNoMessageProcessingMutex.unlock();
		}
	}

	void MessageSystem::CreateNewsgroup(MsgTypeID groupID)
	{
		auto find = mNewsgroupReceivers.find(groupID);
		if (find != mNewsgroupReceivers.end())
		{
			IceWarning("Error while creating Newsgroup '" + Ogre::StringConverter::toString(groupID) + "'. Group already exists!");
			return;
		}

		Newsgroup group;
		group.groupID = groupID;
		mNewsgroupReceivers.insert(std::make_pair<MsgTypeID, Newsgroup >(groupID, group));

		IceNote("Created Newsgroup \"" + Ogre::StringConverter::toString(groupID) + "\"");
	};

	void MessageSystem::JoinNewsgroup(MessageListener *listener, MsgTypeID groupID)
	{
		auto find = mNewsgroupReceivers.find(groupID);
		if (find == mNewsgroupReceivers.end())
		{
			IceWarning("Group '" + Ogre::StringConverter::toString(groupID) + "' does not exist!");
			return;
		}

		std::vector<MessageListener*> *receivers = find->second.receivers[listener->GetAccessPermitionID()].GetVector();
		IceAssert(std::find(receivers->begin(), receivers->end(), listener) == receivers->end())
		receivers->push_back(listener);
	};

	void MessageSystem::QuitNewsgroup(MessageListener *listener, MsgTypeID groupID)
	{
		auto find = mNewsgroupReceivers.find(groupID);
		if (find == mNewsgroupReceivers.end())
		{
			IceWarning("Group '" + Ogre::StringConverter::toString(groupID) + "' does not exist!");
			return;
		}

		WrappedVector<MessageListener*> receivers =  find->second.receivers[listener->GetAccessPermitionID()];

		unsigned int index = 0;
		for (unsigned int i = 0; i < receivers.GetVector()->size(); i++)
		{
			if ((*receivers.GetVector())[i] == listener)
			{
				receivers.Remove(i);
				break;
			}
			index++;
		}
	};

	void MessageSystem::QuitAllNewsgroups(MessageListener *listener, AccessPermitionID accessPermitionID)
	{
		for (auto ni = mNewsgroupReceivers.begin(); ni != mNewsgroupReceivers.end(); ++ni)
		{
			WrappedVector<MessageListener*> receivers = ni->second.receivers[accessPermitionID];
			unsigned int index = 0;
			for (unsigned int i = 0; i < receivers.GetVector()->size(); i++)
			{
				if ((*receivers.GetVector())[i] == listener)
				{
					receivers.Remove(i);
					break;
				}
				index++;
			}
		}
	}

	MessageSystem& MessageSystem::Instance()
	{
		static MessageSystem TheOneAndOnly;
		return TheOneAndOnly;
	};

};