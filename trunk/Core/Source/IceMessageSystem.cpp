
#include "IceMessageSystem.h"
#include "IceMessageListener.h"

namespace Ice
{

	MessageSystem::MessageSystem()
	{
		CreateNewsgroup("COMMON");

		CreateNewsgroup("UPDATE_PER_FRAME");
		CreateNewsgroup("START_PHYSICS");
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
		for (std::list<Newsgroup>::iterator ni = mNewsgroups.begin(); ni != mNewsgroups.end(); ni++)
		{
			if ((*ni).mName == msg.type)
			{
				(*ni).mCurrentMessages.push_back(msg);
			}
		}
	};

	void MessageSystem::CreateNewsgroup(Ogre::String groupname, float priority)
	{
		for (std::list<Newsgroup>::iterator ni = mNewsgroups.begin(); ni != mNewsgroups.end(); ni++)
		{
			if ((*ni).mName == groupname)
			{
				Ogre::LogManager::getSingleton().logMessage("Error while creating Newsgroup '" + groupname + "'. Group already exists!");
				return;
			}
		}

		Newsgroup newsgroup;
		newsgroup.mName = groupname;
		newsgroup.mPriority = priority;
		std::list<Newsgroup>::iterator i = mNewsgroups.begin();
		for (unsigned int x = 0; x < mNewsgroups.size(); x++)
		{
			if ((*i).mPriority < priority) break;
			i++;
		}
		mNewsgroups.insert(i, newsgroup);

		IceNote("Created Newsgroup \"" + groupname + "\"");
	};

	void MessageSystem::JoinNewsgroup(MessageListener *listener, Ogre::String groupname)
	{
		std::list<Newsgroup>::iterator ni = mNewsgroups.begin();
		bool found = false;
		for (; ni != mNewsgroups.end(); ni++)
		{
			if ((*ni).mName == groupname)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			IceWarning("Newsgroup \"" + groupname + "\" does not exist");
			return;
		}

		auto li = (*ni).mListeners.begin();
		IceAssert(std::find((*ni).mListeners.begin(), (*ni).mListeners.end(), listener) == (*ni).mListeners.end())
		for (; li != (*ni).mListeners.end(); li++)
		{
			if ((*li)->GetListenerPriority() <= listener->GetListenerPriority())
			{
				(*ni).mListeners.insert(li, listener);
				return;
			}
		}

		(*ni).mListeners.push_back(listener);
		//Ogre::LogManager::getSingleton().logMessage("MessageSystem::JoinNewsgroup: Added listener to Newsgroup '" + groupname + "'.");
	};

	void MessageSystem::QuitNewsgroup(MessageListener *listener, Ogre::String groupname)
	{
		std::list<Newsgroup>::iterator ni = mNewsgroups.begin();
		bool found = false;
		for (; ni != mNewsgroups.end(); ni++)
		{
			if ((*ni).mName == groupname)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			Ogre::LogManager::getSingleton().logMessage("Warning: MessageSystem::QuitNewsgroup: Newsgroup '" + groupname + "' doesn't exist!");
			return;
		}

		//(*i).second->remove(listener);
		for (auto x = (*ni).mListeners.begin(); x != (*ni).mListeners.end(); x++)
		{
			if ((*x) == listener)
			{
				(*ni).mListeners.erase(x);
				break;
			}
		}
		//Ogre::LogManager::getSingleton().logMessage("MessageSystem::QuitNewsgroup: Quitted listener to Newsgroup '" + groupname + "'.");
	};

	void MessageSystem::QuitAllNewsgroups(MessageListener *listener)
	{
		for (std::list<Newsgroup>::iterator ni = mNewsgroups.begin(); ni != mNewsgroups.end(); ++ni)
		{
			for (auto li = (*ni).mListeners.begin(); li != (*ni).mListeners.end(); ++li)
			{
				if ((*li) == listener)
				{
					(*ni).mListeners.erase(li);
					break;
				}
			}
		}
	}

	void MessageSystem::Update()
	{
		for (std::list<Newsgroup>::iterator ni = mNewsgroups.begin(); ni != mNewsgroups.end(); ni++)
		{
			std::vector<Msg> tempMessages = (*ni).mCurrentMessages;	//Damit, falls Listener dieser Message diese Message senden, es nicht zum rekursiven Chaos kommt.
			(*ni).mCurrentMessages.clear();
			for (std::vector<Msg>::iterator msgIter = tempMessages.begin(); msgIter != tempMessages.end(); msgIter++)
			{
				for (int li = 0; li < (*ni).mListeners.size(); li++)
				{
					(*ni).mListeners[li]->ReceiveMessage(*msgIter);
				}
			}
		}
	};

	void MessageSystem::SendInstantMessage(Msg &msg)
	{
		for (std::list<Newsgroup>::iterator ni = mNewsgroups.begin(); ni != mNewsgroups.end(); ni++)
		{
			if ((*ni).mName == msg.type)
			{
				for (int li = 0; li < (*ni).mListeners.size(); li++)
				{
					(*ni).mListeners[li]->ReceiveMessage(msg);
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