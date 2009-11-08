
#include "../Header/Testplugin.h"
#include "SGTMain.h"

const Ogre::String sPluginName = "Testplugin";
Testplugin* plugin;

extern "C" void __declspec(dllexport) dllStartPlugin(void) throw()
{
	plugin = new Testplugin();
	SGTMain::Instance().InstallPlugin(plugin);
}

extern "C" void __declspec(dllexport) dllStopPlugin(void)
{
	SGTMain::Instance().UninstallPlugin(plugin);
	delete plugin;
}

	/// @copydoc Plugin::getName
const Ogre::String& Testplugin::getName() const
{
	return sPluginName;
}

/// @copydoc Plugin::install
void Testplugin::install()
{
	Ogre::LogManager::getSingleton().logMessage("Testplugin installiert!");
}

/// @copydoc Plugin::initialise
void Testplugin::initialise()
{
	Ogre::LogManager::getSingleton().logMessage("Testplugin initialisiert!");
}

/// @copydoc Plugin::shutdown
void Testplugin::shutdown()
{
	Ogre::LogManager::getSingleton().logMessage("Testplugin deinitialisiert!");
}

/// @copydoc Plugin::uninstall
void Testplugin::uninstall()
{
	Ogre::LogManager::getSingleton().logMessage("Testplugin deinstaliert!");
}