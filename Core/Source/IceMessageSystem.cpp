
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

		IceNote("Created Newsgroup '" + groupname + "'.");
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
			IceWarning("Newsgroup \"" + groupname + "\" doesn't exist!");
			return;
		}

		std::list<MessageListener*>::iterator li = (*ni).mListeners.begin();
		for (; li != (*ni).mListeners.end(); li++)
		{
			if ((*li)->GetListenerPriority() < listener->GetListenerPriority())
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
		/*std::list<Newsgroup>::iterator ni = mNewsgroups.begin();
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
		for (std::list<MessageListener*>::iterator x = (*ni).mListeners.begin(); x != (*ni).mListeners.end(); x++)
		{
			if ((*x) == listener)
			{
				(*ni).mListeners.erase(x);
				break;
			}
		}*/
		//Ogre::LogManager::getSingleton().logMessage("MessageSystem::QuitNewsgroup: Quitted listener to Newsgroup '" + groupname + "'.");
	};

	void MessageSystem::QuitAllNewsgroups(MessageListener *listener)
	{
		for (std::list<Newsgroup>::iterator ni = mNewsgroups.begin(); ni != mNewsgroups.end(); ++ni)
		{
			for (std::list<MessageListener*>::iterator li = (*ni).mListeners.begin(); li != (*ni).mListeners.end(); ++li)
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
			for (std::vector<Msg>::iterator MsgIter = tempMessages.begin(); MsgIter != tempMessages.end(); MsgIter++)
			{
				for (std::list<MessageListener*>::iterator li = (*ni).mListeners.begin(); li != (*ni).mListeners.end(); li++)
				{
					//Ogre::LogManager::getSingleton().logMessage((*MsgIter).type);
					(*li)->ReceiveMessage((*MsgIter));
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
				for (std::list<MessageListener*>::iterator li = (*ni).mListeners.begin(); li != (*ni).mListeners.end(); li++)
				{
					(*li)->ReceiveMessage(msg);
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