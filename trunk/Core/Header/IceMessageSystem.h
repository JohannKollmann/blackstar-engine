
#pragma once

#include <map>
#include "Ogre.h"
#include "IceIncludes.h"
#include "IceMsg.h"
#include "IceMessageListener.h"

namespace Ice
{

struct DllExport Newsgroup
{
	float mPriority;
	std::list<MessageListener*> mListeners;
	Ogre::String mName;

	std::vector<Msg> mCurrentMessages;
};

class DllExport MessageSystem
{
protected:
	MessageSystem();
	~MessageSystem();

private:

	/*
	Die Newsgroups. Jeder Newsgroup ist ein Vektor von MessageListenern zugeordnet, die der Newsgroup
	angehören.
	*/
	std::list<Newsgroup> mNewsgroups;

public:

	/*
	Verschickt eine Message. Ist der Empfänger Zeiger der Message NULL, wird die Message an alle
	Newsgroupmitglieder versendet. Eine Newsgroup muss in jedem Fall angegeben werden, da sie
	gleichzeitig zur Message-Identifikation dient.
	*/
	void SendMessage(Msg &msg);

	void SendInstantMessage(Msg &msg);

	/*
	Erstellt eine Newsgroup mit dem Namen groupname.
	*/
	void CreateNewsgroup(Ogre::String groupname, float priority = 1.0f);

	/*
	Registriert einen MessageListener in einer Newsgroup.
	*/
	void JoinNewsgroup(MessageListener *listener, Ogre::String groupname);

	/*
	Entfernt einen MessageListener aus einer Newsgroup.
	*/
	void QuitNewsgroup(MessageListener *listener, Ogre::String groupname);

	/*
	Arbeitet alle abzuschickenden Messages ab.
	*/
	void Update();


	//Singleton
	static MessageSystem& Instance();
};

};