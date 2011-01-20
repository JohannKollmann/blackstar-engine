
#pragma once

#include <map>
#include "Ogre.h"
#include "IceIncludes.h"
#include "IceMsg.h"
#include "IceMessageListener.h"

namespace Ice
{

	class DllExport MessageSystem
	{
	public:
		typedef Ogre::String NewsgroupID;

		struct DllExport Newsgroup
		{
			float mPriority;
			std::vector<MessageListener*> mListeners;
			NewsgroupID mID;
			std::vector<Msg> mCurrentMessages;

			unsigned int _currIterateIndex;	//hack for remove while iterating
		};

	protected:
		MessageSystem();
		~MessageSystem();

	private:

		/*
		Die Newsgroups. Jeder Newsgroup ist ein Vektor von MessageListenern zugeordnet, die der Newsgroup
		angehören.
		*/
		std::map<NewsgroupID, Newsgroup> mNewsgroups;

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
		void CreateNewsgroup(NewsgroupID groupname, float priority = 1.0f);

		/*
		Registriert einen MessageListener in einer Newsgroup.
		*/
		void JoinNewsgroup(MessageListener *listener, NewsgroupID groupname);

		/*
		Entfernt einen MessageListener aus einer Newsgroup.
		*/
		void QuitNewsgroup(MessageListener *listener, NewsgroupID groupname);

		void QuitAllNewsgroups(MessageListener *listener);

		/*
		Arbeitet alle abzuschickenden Messages ab.
		*/
		void Update();


		//Singleton
		static MessageSystem& Instance();
	};

};