
#pragma once

#include "Ogre.h"
#include "OgrePlugin.h"
#include "IceMessageListener.h"

class __declspec(dllexport) ScriptExtensions : public Ogre::Plugin, public Ice::MessageListener
{
public:
	static ScriptExtensions& GetInstance();

	const Ogre::String& getName() const;

	//plugin overloads
	void install();
	void initialise();
	void shutdown();
	void uninstall();
	//Ice::MessageListener overloads
	void ReceiveMessage(Ice::Msg &msg);
private:
	static const Ogre::String m_cstrName;
};