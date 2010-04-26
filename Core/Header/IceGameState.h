
#pragma once

#include "IceIncludes.h"
#include "Ogre.h"
#include <vector>

namespace Ice
{

class GameState
{
public:

	GameState() {};
	virtual ~GameState() {};

	virtual Ogre::String GetName() = 0;
	virtual bool OnUpdate(float time, float time_total) { return false; };
	virtual void OnEnter() {};
};

class DllExport Game : GameState
{
private:
	bool mInitialized;

public:
	Game() { mInitialized = false; };
	~Game() {};

	Ogre::String GetName() { return "Game"; };
	bool OnUpdate(float time, float time_total);
	void OnEnter();
};

class DllExport Editor : GameState
{

public:
	Editor() {};
	~Editor() {};

	Ogre::String GetName() { return "Editor"; };
	bool OnUpdate(float time, float time_total);
};

class DllExport DefaultMenu : GameState
{
public:
	DefaultMenu() {};
	~DefaultMenu() {};

	Ogre::String GetName() { return "DefaultMenu"; };
	bool OnUpdate(float time, float time_total);
	void OnEnter();
};

};