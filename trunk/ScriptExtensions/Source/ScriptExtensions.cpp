
#include "ScriptExtensions.h"
#include "SGTMain.h"

#include "SGTMusicSystem.h"
#include "ScriptedControls.h"
#include "ResidentVariables.h"
#include "GUISystem.h"
#include "SGTScriptableInstances.h"

#include "SGTMessageSystem.h"

const Ogre::String ScriptExtensions::m_cstrName=Ogre::String("ScriptExtensions");

extern "C" void __declspec(dllexport) dllStartPlugin(void) throw()
{
	SGTMain::Instance().InstallPlugin(&ScriptExtensions::GetInstance());
}

extern "C" void __declspec(dllexport) dllStopPlugin(void)
{
	SGTMain::Instance().UninstallPlugin(&ScriptExtensions::GetInstance());
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
	SGTScriptableInstances::GetInstance();
	Ogre::LogManager::getSingleton().logMessage("ScriptExtensions: initializing GUI");
	SGTGUISystem::GetInstance();
	Ogre::LogManager::getSingleton().logMessage("ScriptExtensions: initializing scripted controls");
	ScriptedControls::GetInstance();
	Ogre::LogManager::getSingleton().logMessage("ScriptExtensions: initializing music system");
	SGTMusicSystem::GetInstance();
	SGTMessageSystem::Instance().JoinNewsgroup(this, "REPARSE_SCRIPTS");
}


void
ScriptExtensions::shutdown()
{
	Ogre::LogManager::getSingleton().logMessage("Script Extensions shut down");
	SGTMessageSystem::Instance().QuitNewsgroup(this, "REPARSE_SCRIPTS");
}


void
ScriptExtensions::uninstall()
{
	Ogre::LogManager::getSingleton().logMessage("Script Extensions uninstalled");
}

void
ScriptExtensions::ReceiveMessage(SGTMsg &msg)
{
	if(msg.mNewsgroup == "REPARSE_SCRIPTS")
	{
		SGTGUISystem::GetInstance().Clear();
		SGTMusicSystem::GetInstance().Clear();
		ResidentManager::GetInstance().Clear();
		SGTScriptableInstances::GetInstance().Clear();
	}
}