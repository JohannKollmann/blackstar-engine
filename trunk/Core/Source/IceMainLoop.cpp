
#include "IceMainLoop.h"

namespace Ice
{

MainLoop::MainLoop()
{
	mTimeSinceLastFrame = 0.0f;
	mTotalTimeElapsed = 0.0f;
	mTotalLastFrameTime = timeGetTime();
	mRunning = true;
	mPaused = false;
	mRunPhysics = true;
	mCurrentState = 0;
	AddState((GameState*)new Game());
	AddState((GameState*)new DefaultMenu());
	AddState((GameState*)new Editor());
};

void MainLoop::AddState(GameState* state)
{
	mStates.push_back(state);
}

void MainLoop::SetState(Ogre::String name)
{
	for (std::vector<GameState*>::iterator i = mStates.begin(); i != mStates.end(); i++)
	{
		if ((*i)->GetName() == name)
		{
			mCurrentState = (*i);
			mCurrentState->OnEnter();
			return;
		}
	}
}


void MainLoop::startLoop()
{
	mPaused = false;
	mTimeSinceLastFrame = 0.0f;
	mTotalTimeElapsed = 0.0f;
	mTotalLastFrameTime = timeGetTime();
	while (true)
	{
		Ogre::WindowEventUtilities::messagePump();
		if (!doLoop()) break;;
	}
};

bool MainLoop::doLoop()
{
	if (!mPaused && mCurrentState)
	{
		DWORD time = timeGetTime();
		DWORD difference = time - mTotalLastFrameTime;
		mTotalLastFrameTime = time;
		mTimeSinceLastFrame = static_cast<float>(difference * 0.001f);
		mTotalTimeElapsed += mTimeSinceLastFrame;

		return mCurrentState->OnUpdate(mTimeSinceLastFrame, mTotalTimeElapsed);
	}
	return true;
};

void MainLoop::quitLoop()
{
	mRunning = false;
	mCurrentState = 0;
};

void MainLoop::pauseLoop()
{
	mPaused = true;
};

void MainLoop::SetPhysics(bool enable)
{
	mRunPhysics = enable;
}

void MainLoop::continueLoop()
{
	mPaused = false;
};

MainLoop& MainLoop::Instance()
{
	static MainLoop TheOneAndOnly;
	return TheOneAndOnly;
};

};