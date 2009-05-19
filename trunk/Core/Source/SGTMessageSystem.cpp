
#include "SGTMessageSystem.h"
#include "SGTMessageListener.h"

SGTMessageSystem::SGTMessageSystem()
{
	CreateNewsgroup("COMMON");
	CreateNewsgroup("UPDATE_PER_FRAME");
	CreateNewsgroup("KEY_UP");
	CreateNewsgroup("KEY_DOWN");
	CreateNewsgroup("MOUSE_DOWN");
	CreateNewsgroup("MOUSE_UP");
	CreateNewsgroup("MOUSE_MOVE");
	CreateNewsgroup("CONSOLE_INGAME");
}

SGTMessageSystem::~SGTMessageSystem()
{
	mNewsgroups.clear();
}


void SGTMessageSystem::SendMessage(SGTMsg &msg)
{
	for (std::list<SGTNewsgroup>::iterator ni = mNewsgroups.begin(); ni != mNewsgroups.end(); ni++)
	{
		if ((*ni).mName == msg.mNewsgroup)
		{
			(*ni).mCurrentMessages.push_back(msg);
		}
	}
};

void SGTMessageSystem::CreateNewsgroup(Ogre::String groupname, float priority)
{
	for (std::list<SGTNewsgroup>::iterator ni = mNewsgroups.begin(); ni != mNewsgroups.end(); ni++)
	{
		if ((*ni).mName == groupname)
		{
			Ogre::LogManager::getSingleton().logMessage("Error while creating Newsgroup '" + groupname + "'. Group already exists!");
			return;
		}
	}

	SGTNewsgroup newsgroup;
	newsgroup.mName = groupname;
	newsgroup.mPriority = priority;
	std::list<SGTNewsgroup>::iterator i = mNewsgroups.begin();
	for (unsigned int x = 0; x < mNewsgroups.size(); x++)
	{
		if ((*i).mPriority < priority) break;
		i++;
	}
	mNewsgroups.insert(i, newsgroup);

	Ogre::LogManager::getSingleton().logMessage("SGTMessageSystem::CreateNewsgroup: Created Newsgroup '" + groupname + "'.");
};

void SGTMessageSystem::JoinNewsgroup(SGTMessageListener *listener, Ogre::String groupname)
{
	std::list<SGTNewsgroup>::iterator ni = mNewsgroups.begin();
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
		Ogre::LogManager::getSingleton().logMessage("Warning: SGTMessageSystem::JoinNewsgroup: Newsgroup '" + groupname + "' doesn't exist!");
		return;
	}

	std::list<SGTMessageListener*>::iterator li = (*ni).mListeners.begin();
	for (; li != (*ni).mListeners.end(); li++)
	{
		if ((*li)->GetListenerPriority() < listener->GetListenerPriority())
		{
			break;
		}
	}

	(*ni).mListeners.insert(li, listener);
	//Ogre::LogManager::getSingleton().logMessage("SGTMessageSystem::JoinNewsgroup: Added listener to Newsgroup '" + groupname + "'.");
};

void SGTMessageSystem::QuitNewsgroup(SGTMessageListener *listener, Ogre::String groupname)
{
	std::list<SGTNewsgroup>::iterator ni = mNewsgroups.begin();
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
		Ogre::LogManager::getSingleton().logMessage("Warning: SGTMessageSystem::QuitNewsgroup: Newsgroup '" + groupname + "' doesn't exist!");
		return;
	}

	//(*i).second->remove(listener);
	for (std::list<SGTMessageListener*>::iterator x = (*ni).mListeners.begin(); x != (*ni).mListeners.end(); x++)
	{
		if ((*x) == listener)
		{
			(*ni).mListeners.erase(x);
			break;
		}
	}
	//Ogre::LogManager::getSingleton().logMessage("SGTMessageSystem::QuitNewsgroup: Quitted listener to Newsgroup '" + groupname + "'.");
};

void SGTMessageSystem::Update()
{
	for (std::list<SGTNewsgroup>::iterator ni = mNewsgroups.begin(); ni != mNewsgroups.end(); ni++)
	{
		std::vector<SGTMsg> TempMessages = (*ni).mCurrentMessages;	//Damit, falls Listener dieser Message diese Message senden, es nicht zum rekursiven Chaos kommt.
		(*ni).mCurrentMessages.clear();
		for (std::vector<SGTMsg>::iterator MsgIter = TempMessages.begin(); MsgIter != TempMessages.end(); MsgIter++)
		{
			for (std::list<SGTMessageListener*>::iterator li = (*ni).mListeners.begin(); li != (*ni).mListeners.end(); li++)
			{
				(*li)->ReceiveMessage((*MsgIter));
			}
		}
	}
};


SGTMessageSystem& SGTMessageSystem::Instance()
{
	static SGTMessageSystem TheOneAndOnly;
	return TheOneAndOnly;
};