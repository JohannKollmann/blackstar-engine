
#include "Ogre.h"
#include "OgrePlugin.h"

class __declspec(dllexport) Testplugin : public Ogre::Plugin
{
public:
	Testplugin() { };



	/// @copydoc Plugin::getName
	const Ogre::String& getName() const;

	/// @copydoc Plugin::install
	void install();

	/// @copydoc Plugin::initialise
	void initialise();

	/// @copydoc Plugin::shutdown
	void shutdown();

	/// @copydoc Plugin::uninstall
	void uninstall();
};