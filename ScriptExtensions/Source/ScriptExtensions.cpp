
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
	Ice::Log::Instance().LogMessage("Script Extensions installed");
}


void
ScriptExtensions::initialise()
{
	Ice::Log::Instance().LogMessage("ScriptExtensions: initializing resident variables");
	ResidentManager::GetInstance();
	Ice::Log::Instance().LogMessage("ScriptExtensions: initializing scriptable instances");
	ScriptableInstances::GetInstance();
	Ice::Log::Instance().LogMessage("ScriptExtensions: initializing GUI");
	GUISystem::GetInstance();
	Ice::Log::Instance().LogMessage("ScriptExtensions: initializing scripted controls");
	ScriptedControls::GetInstance();
	Ice::Log::Instance().LogMessage("ScriptExtensions: initializing music system");
	MusicSystem::GetInstance();

	TextureDebugger::Instance();
	JoinNewsgroup(Ice::GlobalMessageIDs::REPARSE_SCRIPTS_PRE);
}


void
ScriptExtensions::shutdown()
{
	Ice::Log::Instance().LogMessage("Script Extensions shut down");
}


void
ScriptExtensions::uninstall()
{
	Ice::Log::Instance().LogMessage("Script Extensions uninstalled");
}

void
ScriptExtensions::ReceiveMessage(Ice::Msg &msg)
{
	if(msg.typeID == Ice::GlobalMessageIDs::REPARSE_SCRIPTS_PRE)
	{
		GUISystem::GetInstance().Clear();
		MusicSystem::GetInstance().Clear();
		ResidentManager::GetInstance().Clear();
		ScriptableInstances::GetInstance().Clear();
	}
}