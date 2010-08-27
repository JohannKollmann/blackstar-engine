
#include "ScriptExtensions.h"
#include "IceMain.h"

#include "MusicSystem.h"
#include "ScriptedControls.h"
#include "ResidentVariables.h"
#include "GUISystem.h"
#include "ScriptableInstances.h"
#include "TextureDebugger.h"
#include "IceMessageSystem.h"

const Ogre::String ScriptExtensions::m_cstrName=Ogre::String("ScriptExtensions");

extern "C" void __declspec(dllexport) dllStartPlugin(void) throw()
{
	Ice::Main::Instance().InstallPlugin(&ScriptExtensions::GetInstance());
}

extern "C" void __declspec(dllexport) dllStopPlugin(void)
{
	Ice::Main::Instance().UninstallPlugin(&ScriptExtensions::GetInstance());
}

ScriptExtensions&
ScriptExtensions::GetInstance()
{
	static ScriptExtensions singleton;
	return singleton;
}

const Ogre::String&
ScriptExtensions::getName() const
{
	return m_cstrName;
}


void
ScriptExtensions::install()
{
	Ogre::LogManager::getSingleton().logMessage("Script Extensions installed");
}


void
ScriptExtensions::initialise()
{
	Ogre::LogManager::getSingleton().logMessage("ScriptExtensions: initializing resident variables");
	ResidentManager::GetInstance();
	Ogre::LogManager::getSingleton().logMessage("ScriptExtensions: initializing scriptable instances");
	ScriptableInstances::GetInstance();
	Ogre::LogManager::getSingleton().logMessage("ScriptExtensions: initializing GUI");
	GUISystem::GetInstance();
	Ogre::LogManager::getSingleton().logMessage("ScriptExtensions: initializing scripted controls");
	ScriptedControls::GetInstance();
	Ogre::LogManager::getSingleton().logMessage("ScriptExtensions: initializing music system");
	MusicSystem::GetInstance();

	TextureDebugger::Instance();
	Ice::MessageSystem::Instance().JoinNewsgroup(this, "REPARSE_SCRIPTS");
}


void
ScriptExtensions::shutdown()
{
	Ogre::LogManager::getSingleton().logMessage("Script Extensions shut down");
	Ice::MessageSystem::Instance().QuitNewsgroup(this, "REPARSE_SCRIPTS");
}


void
ScriptExtensions::uninstall()
{
	Ogre::LogManager::getSingleton().logMessage("Script Extensions uninstalled");
}

void
ScriptExtensions::ReceiveMessage(Ice::Msg &msg)
{
	if(msg.type == "REPARSE_SCRIPTS")
	{
		GUISystem::GetInstance().Clear();
		MusicSystem::GetInstance().Clear();
		ResidentManager::GetInstance().Clear();
		ScriptableInstances::GetInstance().Clear();
	}
}