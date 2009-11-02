
#pragma once

#include "SGTincludes.h"
#include "windows.h"
#include "Ogre.h"
#include "SGTGameState.h"

class SGTDllExport SGTMainLoop
{
protected:
	bool mRunning;
	bool mPaused;
	bool mRunPhysics;
	DWORD mTotalLastFrameTime;
	float mTimeSinceLastFrame;
	float mTotalTimeElapsed;
	//Ogre::Timer *mTimer;

	std::vector<SGTGameState*> mStates;
	SGTGameState* mCurrentState;

public:

	SGTMainLoop();
	~SGTMainLoop() { };

	void startLoop();
	void quitLoop();
	void pauseLoop();
	void continueLoop();
	void SetPhysics(bool enable);
	bool GetRunPhysics() { return mRunPhysics; };

	bool doLoop();

	void AddState(SGTGameState* state);
	void SetState(Ogre::String name);

	//Singleton
	static SGTMainLoop& Instance();

};