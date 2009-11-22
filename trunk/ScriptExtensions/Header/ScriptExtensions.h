
#include "Ogre.h"
#include "OgrePlugin.h"
#include "SGTMessageListener.h"

class __declspec(dllexport) ScriptExtensions : public Ogre::Plugin, public SGTMessageListener
{
public:
	static ScriptExtensions& GetInstance();

	const Ogre::String& getName() const;

	//plugin overloads
	void install();
	void initialise();
	void shutdown();
	void uninstall();
	//SGTMessageListener overloads
	void ReceiveMessage(SGTMsg &msg);
private:
	static const Ogre::String m_cstrName;
};