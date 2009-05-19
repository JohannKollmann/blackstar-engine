
#include "../Header/Testplugin.h"
#include "SGTKernel.h"
#include "SGTMain.h"
#include "SGTSceneManager.h"

const Ogre::String sPluginName = "Testplugin";
Testplugin* plugin;

extern "C" void __declspec(dllexport) dllStartPlugin(void) throw()
{
	plugin = new Testplugin();
	SGTKernel::Instance().InstallPlugin(plugin);
}

extern "C" void __declspec(dllexport) dllStopPlugin(void)
{
	SGTKernel::Instance().UninstallPlugin(plugin);
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
	//SGTMain::Instance().GetViewport()->setBackgroundColour(Ogre::ColourValue(0,0,1));
	SGTMain::Instance().GetOgreSceneMgr()->setAmbientLight(Ogre::ColourValue(0.5,0.5,0.5));
	//SGTSceneManager::Instance().CreateStaticMesh("Krypta.mesh");
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