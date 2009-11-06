
#include "SGTMainLoop.h"

SGTMainLoop::SGTMainLoop()
{
	mTimeSinceLastFrame = 0.0f;
	mTotalTimeElapsed = 0.0f;
	mTotalLastFrameTime = timeGetTime();
	mRunning = true;
	mPaused = false;
	mRunPhysics = true;
	AddState((SGTGameState*)new SGTGame());
	AddState((SGTGameState*)new SGTDefaultMenu());
	AddState((SGTGameState*)new SGTEditor());
};

void SGTMainLoop::AddState(SGTGameState* state)
{
	mStates.push_back(state);
}

void SGTMainLoop::SetState(Ogre::String name)
{
	for (std::vector<SGTGameState*>::iterator i = mStates.begin(); i != mStates.end(); i++)
	{
		if ((*i)->GetName() == name)
		{
			mCurrentState = (*i);
			mCurrentState->OnEnter();
			return;
		}
	}
}


void SGTMainLoop::startLoop()
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

bool SGTMainLoop::doLoop()
{
	if (!mPaused)
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

void SGTMainLoop::quitLoop()
{
	mRunning = false;
};

void SGTMainLoop::pauseLoop()
{
	mPaused = true;
};

void SGTMainLoop::SetPhysics(bool enable)
{
	mRunPhysics = enable;
}

void SGTMainLoop::continueLoop()
{
	mPaused = false;
};

SGTMainLoop& SGTMainLoop::Instance()
{
	static SGTMainLoop TheOneAndOnly;
	return TheOneAndOnly;
};