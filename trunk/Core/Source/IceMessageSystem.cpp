
#include "IceMessageSystem.h"
#include "IceMessageListener.h"

namespace Ice
{
	const long MAX_WAITTIME_SECONDS = 5l;

	MessageSystem::MessageSystem()
		: mNumProcessingMessages(0), mNumWaitingSynchronized(0), mSynchronizedProcessing(false)
	{
		for (int i = 0; i < MAX_NUM_JOBQUEUES; i++)
		{
			mCurrentJobs[i].sendAllMessagesInstantly = false;
			mCurrentJobs[i].processingMessages = false;
		}
	}

	void MessageSystem::AddThisThreadAccessPermition(AccessPermitionID accessPermitionID)
	{
		mThreadAccessPermitions[accessPermitionID].insert(boost::this_thread::get_id());
	}

	void MessageSystem::RegisterThread(boost::thread::id threadID, AccessPermitionID accessPermitionID)
	{
		auto existing = mThreadBindings.find(threadID);
		if (existing != mThreadBindings.end())
		{
			IceWarning("Changing existing thread binding!")
			existing->second.accessPermitionID = accessPermitionID;
		}

		ThreadBinding tb;
		tb.accessPermitionID = accessPermitionID;
		tb.lockedMessageProcessing = false;
		mThreadBindings.insert(std::make_pair<boost::thread::id, ThreadBinding>(threadID, tb));

		mThreadAccessPermitions[accessPermitionID].insert(threadID);
	}

	void MessageSystem::RegisterThisThread(AccessPermitionID accessPermitionID)
	{
		RegisterThread(boost::this_thread::get_id(), accessPermitionID);
	}

	void MessageSystem::SetSendAllMessagesInstantly(AccessPermitionID accessPermitionID, bool instant)
	{
		mCurrentJobs[accessPermitionID].sendAllMessagesInstantly = instant;
	}

	bool MessageSystem::testThreadAccessPermition(AccessPermitionID receiverID)
	{
		auto i = mThreadBindings.find(boost::this_thread::get_id());
		if (i == mThreadBindings.end() || i->second.accessPermitionID < 0) return false;

		AccessPermitionID senderAccessPermitionID = i->second.accessPermitionID;
		if (mCurrentJobs[senderAccessPermitionID].sendAllMessagesInstantly)
			return true;

		auto allowedThreads = mThreadAccessPermitions.find(receiverID);
		if (allowedThreads == mThreadAccessPermitions.end())
			return false;

		return (allowedThreads->second.find(boost::this_thread::get_id()) != allowedThreads->second.end());
	}

	bool MessageSystem::isThreadProcessingMessages()
	{
		auto i = mThreadBindings.find(boost::this_thread::get_id());
		if (i == mThreadBindings.end() || i->second.accessPermitionID < 0) return false;
		else return mCurrentJobs[i->second.accessPermitionID].processingMessages;
	}

	bool MessageSystem::updateThreadLockedMessageProcessing(bool lock)
	{
		//boost::thread::id threadID = boost::this_thread::get_id();
		auto i = mThreadBindings.find(boost::this_thread::get_id());
		if (i == mThreadBindings.end())
		{
			ThreadBinding tb;
			tb.accessPermitionID = -1;
			tb.lockedMessageProcessing = lock;
			mThreadBindings.insert(std::make_pair<boost::thread::id, ThreadBinding>(boost::this_thread::get_id(), tb));
			return false;
		}
		else
		{
			bool ret = i->second.lockedMessageProcessing;
			i->second.lockedMessageProcessing = lock;
			return ret;
		}
	}

	void MessageSystem::sendMsgPacket(MsgPacket &packet, AccessPermitionID receiverAccessPermition)
	{
		if (testThreadAccessPermition(receiverAccessPermition)) packet.Send();
		else
		{
			auto iSenderAccessPermitionID = mThreadBindings.find(boost::this_thread::get_id());
			if (iSenderAccessPermitionID != mThreadBindings.end() && iSenderAccessPermitionID->second.accessPermitionID > 0)
				mCurrentJobs[receiverAccessPermition].cachedPackets[iSenderAccessPermitionID->second.accessPermitionID].push_back(packet);
			else
			{
				mCurrentJobs[receiverAccessPermition].flushingMutex.lock();
				mCurrentJobs[receiverAccessPermition].packets.push_back(packet);
				mCurrentJobs[receiverAccessPermition].flushingMutex.unlock();
			}
		}
	}

	void MessageSystem::SendMessage(Msg &msg, std::shared_ptr<MessageListener> &receiver, bool sendInstantly)
	{
		MsgPacket packet(msg, std::weak_ptr<MessageListener>(receiver));
		if (sendInstantly)
		{
			LockMessageProcessing();
			packet.Send();
			UnlockMessageProcessing();
		}
		else sendMsgPacket(packet, receiver->GetAccessPermitionID());
	};
	void MessageSystem::SendMessage(Msg &msg, MessageListener *receiver, bool sendInstantly)
	{
		MsgPacket packet(msg, receiver);
		if (sendInstantly)
		{
			LockMessageProcessing();
			packet.Send();
			UnlockMessageProcessing();
		}
		else sendMsgPacket(packet, receiver->GetAccessPermitionID());
	};

	void MessageSystem::MulticastMessage(Msg &msg, bool sendInstantly)
	{
		auto find = mNewsgroupReceivers.find(msg.typeID);
		if (find == mNewsgroupReceivers.end())
		{
			IceWarning("Group ID '" + Ogre::StringConverter::toString(msg.typeID) + "' does not exist!");
			return;
		}
		if (sendInstantly)
		{
			LockMessageProcessing();
			ITERATE(i, find->second.receivers)
			{
				MsgPacket packet(msg,  &i->second);
				packet.Send();
			}
			UnlockMessageProcessing();
		}
		else
		{
			ITERATE(i, find->second.receivers)
			{
				if (i->second.GetVector()->size() > 0)
				{
					MsgPacket packet(msg,  &i->second);
					sendMsgPacket(packet, i->first);
				}
			}
		}
	};

	void MessageSystem::LockMessageProcessing()
	{
		if (updateThreadLockedMessageProcessing(true)) return;

		boost::unique_lock<boost::mutex> lock(mMonitorMutex);

		mNumWaitingSynchronized++;
		if (isThreadProcessingMessages()) mNumProcessingMessages--;
		while (mNumProcessingMessages > 0 || mSynchronizedProcessing)	
		{	//wait until no other thread processes messages
			if (!mNoMessageProcessing.timed_wait(lock, boost::posix_time::seconds(MAX_WAITTIME_SECONDS)))
			{
				std::cout << "Locking timeout - possible deadlock!" << std::endl;
			}
		}
		mAtomicHelperMutex.lock();
		if (isThreadProcessingMessages()) mNumProcessingMessages++;
		mNumWaitingSynchronized--;
		mSynchronizedProcessing = true;
		mAtomicHelperMutex.unlock();
	}
	void MessageSystem::EnterMessageProcessing()
	{
		boost::unique_lock<boost::mutex> lock(mMonitorMutex);
		while (mNumWaitingSynchronized > 0 || mSynchronizedProcessing)
		{	//wait until no other thread processes messages
			if (!mConcurrentMessageProcessing.timed_wait(lock, boost::posix_time::seconds(MAX_WAITTIME_SECONDS)))
			{
				std::cout << "Locking timeout - possible deadlock!" << std::endl;
			}
		}
		mAtomicHelperMutex.lock();
		mNumProcessingMessages++;
		mAtomicHelperMutex.unlock();
	}

	void MessageSystem::UnlockMessageProcessing()
	{
		if (!updateThreadLockedMessageProcessing(false)) return;

		boost::lock_guard<boost::mutex> lock(mMonitorMutex);

		mSynchronizedProcessing = false;
		if (mNumWaitingSynchronized == 0) mConcurrentMessageProcessing.notify_all();
		else mNoMessageProcessing.notify_one();
	}
	void MessageSystem::LeaveMessageProcessing()
	{
		boost::lock_guard<boost::mutex> lock(mMonitorMutex);

		mNumProcessingMessages--;
		if (mNumProcessingMessages == 0) mNoMessageProcessing.notify_one();
	}

	void MessageSystem::ProcessMessages(AccessPermitionID accessPermitionID, bool synchronized, ProcessingListener *listener)
	{
		mCurrentJobs[accessPermitionID].processingMessages = true;

		if (synchronized) LockMessageProcessing();
		else EnterMessageProcessing();

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
		else LeaveMessageProcessing();

		mCurrentJobs[accessPermitionID].processingMessages = false;
	}

	void MessageSystem::ProcessAllMessagesNow()
	{
		LockMessageProcessing();
		for (int accessPermitionID = 0; accessPermitionID < MAX_NUM_JOBQUEUES; accessPermitionID++)
		{
			std::vector<MsgPacket> msgCopy = mCurrentJobs[accessPermitionID].packets;	//copy
			mCurrentJobs[accessPermitionID].packets.clear();	//flush

			ITERATE(i, msgCopy) i->Send();		//deliver messages

			//send cached messages
			for (int i = 0; i < MAX_NUM_JOBQUEUES; i++)
			{
				mCurrentJobs[i].packets.insert(mCurrentJobs[i].packets.end(), mCurrentJobs[i].cachedPackets[accessPermitionID].begin(), mCurrentJobs[i].cachedPackets[accessPermitionID].end());
				mCurrentJobs[i].cachedPackets[accessPermitionID].clear();
			}
		}
		UnlockMessageProcessing();
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

		WrappedVector<MessageListener*> *receivers = &find->second.receivers[listener->GetAccessPermitionID()];

		unsigned int index = 0;
		std::vector<MessageListener*> *pRecVec = receivers->GetVector();
		for (unsigned int i = 0; i < pRecVec->size(); i++)
		{
			if ((*pRecVec)[i] == listener)
			{
				receivers->Remove(i);
				break;
			}
			index++;
		}
	};

	void MessageSystem::QuitAllNewsgroups(MessageListener *listener, AccessPermitionID accessPermitionID)
	{
		for (auto ni = mNewsgroupReceivers.begin(); ni != mNewsgroupReceivers.end(); ++ni)
		{
			WrappedVector<MessageListener*> *receivers = &ni->second.receivers[accessPermitionID];
			std::vector<MessageListener*> *pRecVec = receivers->GetVector();
			for (unsigned int i = 0; i < pRecVec->size(); i++)
			{
				if ((*pRecVec)[i] == listener)
				{
					receivers->Remove(i);
					break;
				}
			}
		}
	}

	MessageSystem& MessageSystem::Instance()
	{
		static MessageSystem TheOneAndOnly;
		return TheOneAndOnly;
	};

};