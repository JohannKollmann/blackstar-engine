
#ifndef __SGTMainLoop_H__
#define __SGTMainLoop_H__

#include "SGTincludes.h"
#include "Ogre.h"
#include <vector>

class SGTGameState
{
public:

	SGTGameState() {};
	virtual ~SGTGameState() {};

	virtual Ogre::String GetName() = 0;
	virtual void OnUpdate(float time, float time_total) {};
	virtual void OnEnter() {};
};

class SGTDllExport SGTGame : SGTGameState
{
private:
	bool mInitialized;

public:
	SGTGame() { mInitialized = false; };
	~SGTGame() {};

	Ogre::String GetName() { return "Game"; };
	void OnUpdate(float time, float time_total);
	void OnEnter();
};

class SGTDllExport SGTEditor : SGTGameState
{

public:
	SGTEditor() {};
	~SGTEditor() {};

	Ogre::String GetName() { return "Editor"; };
	void OnUpdate(float time, float time_total);
};

class SGTDllExport SGTDefaultMenu : SGTGameState
{
public:
	SGTDefaultMenu() {};
	~SGTDefaultMenu() {};

	Ogre::String GetName() { return "DefaultMenu"; };
	void OnUpdate(float time, float time_total);
	void OnEnter();
};

#endif