
#include "Ogre.h"
#include "OgrePlugin.h"

class __declspec(dllexport) ScriptExtensions : public Ogre::Plugin
{
public:
	static ScriptExtensions& GetInstance();

	const Ogre::String& getName() const;

	void install();
	void initialise();
	void shutdown();
	void uninstall();
private:
	static const Ogre::String m_cstrName;
};