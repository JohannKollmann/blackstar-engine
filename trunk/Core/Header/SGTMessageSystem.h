
#ifndef __SGTMessageSystem_H__
#define __SGTMessageSystem_H__

#include <map>
#include "Ogre.h"
#include "SGTIncludes.h"
#include "SGTMsg.h"
#include "SGTMessageListener.h"


struct SGTDllExport SGTNewsgroup
{
	float mPriority;
	std::list<SGTMessageListener*> mListeners;
	Ogre::String mName;

	std::vector<SGTMsg> mCurrentMessages;
};

class SGTDllExport SGTMessageSystem
{
protected:
	SGTMessageSystem();
	~SGTMessageSystem();

private:

	/*
	Die Newsgroups. Jeder Newsgroup ist ein Vektor von MessageListenern zugeordnet, die der Newsgroup
	angehören.
	*/
	std::list<SGTNewsgroup> mNewsgroups;

public:

	/*
	Verschickt eine Message. Ist der Empfänger Zeiger der Message NULL, wird die Message an alle
	Newsgroupmitglieder versendet. Eine Newsgroup muss in jedem Fall angegeben werden, da sie
	gleichzeitig zur Message-Identifikation dient.
	*/
	void SendMessage(SGTMsg &msg);

	void SendInstantMessage(SGTMsg &msg);

	/*
	Erstellt eine Newsgroup mit dem Namen groupname.
	*/
	void CreateNewsgroup(Ogre::String groupname, float priority = 1.0f);

	/*
	Registriert einen MessageListener in einer Newsgroup.
	*/
	void JoinNewsgroup(SGTMessageListener *listener, Ogre::String groupname);

	/*
	Entfernt einen MessageListener aus einer Newsgroup.
	*/
	void QuitNewsgroup(SGTMessageListener *listener, Ogre::String groupname);

	/*
	Arbeitet alle abzuschickenden Messages ab.
	*/
	void Update();


	//Singleton
	static SGTMessageSystem& Instance();
};

#endif