
#include "WaterPlugin.h"

const Ogre::String WaterPlugin::m_cstrName=Ogre::String("WaterPlugin");

extern "C" void __declspec(dllexport) dllStartPlugin(void) throw()
{
	Ice::Main::Instance().InstallPlugin(&WaterPlugin::Instance());
}

extern "C" void __declspec(dllexport) dllStopPlugin(void)
{
	Ice::Main::Instance().UninstallPlugin(&WaterPlugin::Instance());
}

WaterPlugin& WaterPlugin::Instance()
{
	static WaterPlugin singleton;
	return singleton;
}