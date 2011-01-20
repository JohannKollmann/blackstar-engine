
#include "IceMessageSystem.h"
#include "IceMessageListener.h"

namespace Ice
{

	MessageSystem::MessageSystem()
	{
		CreateNewsgroup("COMMON");

		CreateNewsgroup("UPDATE_PER_FRAME");
		CreateNewsgroup("START_PHYSICS");
		CreateNewsgroup("SIMULATING_PHYSICS");
		CreateNewsgroup("END_PHYSICS");
		CreateNewsgroup("START_RENDERING");
		CreateNewsgroup("END_RENDERING");

		CreateNewsgroup("KEY_UP");
		CreateNewsgroup("KEY_DOWN");
		CreateNewsgroup("MOUSE_DOWN");
		CreateNewsgroup("MOUSE_UP");
		CreateNewsgroup("MOUSE_MOVE");

		CreateNewsgroup("CONSOLE_INGAME");
	}

	MessageSystem::~MessageSystem()
	{
		mNewsgroups.clear();
	}


	void MessageSystem::SendMessage(Msg &msg)
	{
		auto find = mNewsgroups.find(msg.type);
		if (find == mNewsgroups.end())
		{
			IceWarning("Group '" + msg.type + "' does not exist!");
			return;
		}

		find->second.mCurrentMessages.push_back(msg);
	};

	void MessageSystem::CreateNewsgroup(NewsgroupID groupname, float priority)
	{
		auto find = mNewsgroups.find(groupname);
		if (find != mNewsgroups.end())
		{
			IceWarning("Error while creating Newsgroup '" + groupname + "'. Group already exists!");
			return;
		}

		Newsgroup newsgroup;
		newsgroup.mID = groupname;
		newsgroup.mPriority = priority;

		mNewsgroups.insert(std::make_pair<NewsgroupID, Newsgroup>(groupname, newsgroup));

		IceNote("Created Newsgroup \"" + groupname + "\"");
	};

	void MessageSystem::JoinNewsgroup(MessageListener *listener, NewsgroupID groupname)
	{
		auto find = mNewsgroups.find(groupname);
		if (find == mNewsgroups.end())
		{
			IceWarning("Group '" + groupname + "' does not exist!");
			return;
		}

		auto li = find->second.mListeners.begin();
		IceAssert(std::find(find->second.mListeners.begin(), find->second.mListeners.end(), listener) == find->second.mListeners.end())
		for (; li != find->second.mListeners.end(); li++)
		{
			if ((*li)->GetListenerPriority() <= listener->GetListenerPriority())
			{
				find->second.mListeners.insert(li, listener);
				return;
			}
		}

		find->second.mListeners.push_back(listener);
	};

	void MessageSystem::QuitNewsgroup(MessageListener *listener, NewsgroupID groupname)
	{
		auto find = mNewsgroups.find(groupname);
		if (find == mNewsgroups.end())
		{
			IceWarning("Group '" + groupname + "' does not exist!");
			return;
		}

		unsigned int index = 0;
		for (auto x = find->second.mListeners.begin(); x != find->second.mListeners.end(); x++)
		{
			if ((*x) == listener)
			{
				find->second.mListeners.erase(x);
				if (index < find->second._currIterateIndex) find->second._currIterateIndex--;
				break;
			}
			index++;
		}
	};

	void MessageSystem::QuitAllNewsgroups(MessageListener *listener)
	{
		for (auto ni = mNewsgroups.begin(); ni != mNewsgroups.end(); ++ni)
		{
			unsigned int index = 0;
			for (auto li = ni->second.mListeners.begin(); li != ni->second.mListeners.end(); li++)
			{
				if (*li == listener)
				{
					ni->second.mListeners.erase(li);
					if (index < ni->second._currIterateIndex) ni->second._currIterateIndex--;
					break;
				}
				index++;
			}
		}
	}

	void MessageSystem::Update()
	{
		for (auto ni = mNewsgroups.begin(); ni != mNewsgroups.end(); ni++)
		{
			std::vector<Msg> tempMessages = ni->second.mCurrentMessages;	//Damit, falls Listener dieser Message diese Message senden, es nicht zum rekursiven Chaos kommt.
			 ni->second.mCurrentMessages.clear();
			for (std::vector<Msg>::iterator msgIter = tempMessages.begin(); msgIter != tempMessages.end(); msgIter++)
			{
				for (ni->second._currIterateIndex = 0; ni->second._currIterateIndex < ni->second.mListeners.size(); ni->second._currIterateIndex++)
				{
					 ni->second.mListeners[ni->second._currIterateIndex]->ReceiveMessage(*msgIter);
				}
			}
		}
	};

	void MessageSystem::SendInstantMessage(Msg &msg)
	{
		auto find = mNewsgroups.find(msg.type);
		if (find == mNewsgroups.end())
		{
			IceWarning("Group '" + msg.type + "' does not exist!");
			return;
		}

		for (find->second._currIterateIndex = 0; find->second._currIterateIndex < find->second.mListeners.size(); find->second._currIterateIndex++)
		{
			find->second.mListeners[find->second._currIterateIndex]->ReceiveMessage(msg);
		}
	}


	MessageSystem& MessageSystem::Instance()
	{
		static MessageSystem TheOneAndOnly;
		return TheOneAndOnly;
	};

};