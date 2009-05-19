
#ifndef __SGTConsole_H__
#define __SGTConsole_H__

#include "SGTIncludes.h"
#include "Ogre.h"
#include "SGTMessageListener.h"


class SGTDllExport SGTConsole : public SGTMessageListener
{
private:
	std::map<Ogre::String, std::vector<Ogre::String> > mCommands;
	Ogre::String mCurrentPrompt;

	bool mActive;
	bool mInitialized;

	Ogre::Rectangle2D   *mRect;
    Ogre::SceneNode      *mNode;
    Ogre::OverlayElement *mTextbox;
    Ogre::Overlay      *mOverlay; 
	float mHeight;
	std::list<Ogre::String> mLines;
	bool mUpdateOverlay;
	unsigned int mStartLine;

public:
	SGTConsole();
	~SGTConsole();

	void Init();
	void Shutdown();

	void Show(bool show);

	/*
	Benutzung:
	AddCommand("change_res", "int int");
	*/
	void AddCommand(Ogre::String name, Ogre::String parameters);

	void ExecCommand(Ogre::String command);

	void Print(Ogre::String text);

	void ReceiveMessage(SGTMsg &msg);

	//Singleton
	static SGTConsole& Instance();
};

#endif