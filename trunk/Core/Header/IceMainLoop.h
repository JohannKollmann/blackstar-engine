
#pragma once

#include "IceIncludes.h"
#include "windows.h"
#include "Ogre.h"
#include "IceGameState.h"


namespace Ice
{

class DllExport MainLoop
{
protected:
	bool mRunning;
	bool mPaused;
	bool mRunPhysics;
	DWORD mTotalLastFrameTime;
	float mTimeSinceLastFrame;
	float mTotalTimeElapsed;
	//Ogre::Timer *mTimer;

	std::vector<GameState*> mStates;
	GameState* mCurrentState;

public:

	MainLoop();
	~MainLoop() { };

	void startLoop();
	void quitLoop();
	void pauseLoop();
	void continueLoop();
	void SetPhysics(bool enable);
	bool GetRunPhysics() { return mRunPhysics; };

	bool doLoop();

	void AddState(GameState* state);
	void SetState(Ogre::String name);

	//Singleton
	static MainLoop& Instance();

};

};