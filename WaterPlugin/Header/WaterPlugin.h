
#pragma once

#include "Ogre.h"
#include "OgrePlugin.h"
#include "GOCHydrax.h"
#include "GOCSimpleWater.h"

class __declspec(dllexport) WaterPlugin : public Ogre::Plugin
{
public:

	static WaterPlugin& Instance();

	const Ogre::String& getName() const
	{
		return m_cstrName;
	}

	//plugin overloads
	void install()
	{
	}
	void initialise()
	{
		GOCHydrax::InstallPlugin();
		GOCSimpleWater::InstallPlugin();
	}
	void shutdown()
	{
	}
	void uninstall()
	{
	}

private:
	static const Ogre::String m_cstrName;
};