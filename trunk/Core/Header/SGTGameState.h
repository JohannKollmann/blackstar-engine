
#ifndef __SGTKernel_H__
#define __SGTKernel_H__

#include "SGTincludes.h"
#include "windows.h"
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

class SGTDllExport SGTKernel
{
protected:
	bool mRunning;
	bool mPaused;
	bool mRunPhysics;
	DWORD mTotalLastFrameTime;
	float mTimeSinceLastFrame;
	float mTotalTimeElapsed;
	//Ogre::Timer *mTimer;

	// List of plugin DLLs loaded
	std::vector<Ogre::DynLib*> mPluginLibs;
	// List of Plugin instances registered
	std::vector<Ogre::Plugin*> mPlugins;

	std::vector<SGTGameState*> mStates;
	SGTGameState* mCurrentState;

public:

	SGTKernel();
	~SGTKernel() { };

	void startLoop();
	void quitLoop();
	void pauseLoop();
	void continueLoop();
	void SetPhysics(bool enable);
	bool GetRunPhysics() { return mRunPhysics; };

	bool doLoop();

	void AddState(SGTGameState* state);
	void SetState(Ogre::String name);

	//Muss vom Plugin aus in dllStartPlugin aufgerufen werden!
	void InstallPlugin(Ogre::Plugin* plugin);

	//Muss vom Plugin aus dllStopPlugin aufgerufen werden!
	void UninstallPlugin(Ogre::Plugin* plugin);

	void LoadPlugin(const Ogre::String& pluginName);

	void UnloadPlugin(const Ogre::String& pluginName);

	void ClearPlugins();

	//Singleton
	static SGTKernel& Instance();

};

#endif