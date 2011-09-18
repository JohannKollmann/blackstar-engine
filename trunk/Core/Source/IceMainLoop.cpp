
#include "IceMainLoop.h"
#include "IceMain.h"
#include "IceMessageSystem.h"
#include "OgreOggSound.h"
#include "IceInput.h"

namespace Ice
{

	void MainLoopThread::operator() ()
	{
		for (;;)
		{		
			if (mTerminate) break;
			Step();
		}
	}

	void MainLoopThread::Step()
	{
		if (!mPaused)
		{
			mDoingStep = true;
			DWORD time = timeGetTime();
			DWORD difference = time - mTotalLastFrameTime;
			mTimeSinceLastFrame = difference < 100 ? difference : 100;	//Max frame time: 100 ms
			if (mFixedTimeStep && mTimeSinceLastFrame < mFixedTimeStepMiliSeconds)
			{
				boost::this_thread::sleep(boost::posix_time::milliseconds(mFixedTimeStepMiliSeconds - mTimeSinceLastFrame));
				mTimeSinceLastFrame = mFixedTimeStepMiliSeconds;
				time = timeGetTime();
			}

			mTotalTimeElapsed += mTimeSinceLastFrame;	

			if (!mFixedTimeStep || mTimeSinceLastFrame >= mFixedTimeStepMiliSeconds)
			{
				mTotalLastFrameTime = time;
				mTimeSinceLastFrameSeconds = static_cast<float>(mTimeSinceLastFrame * 0.001);
				mTotalTimeElapsedSeconds = static_cast<float>(mTotalTimeElapsed * 0.001);
				doLoop();
			}
			mDoingStep = false;
		}	
		else boost::this_thread::sleep(boost::posix_time::milliseconds(20));
	}

	void MainLoopThread::SetFixedTimeStep(DWORD stepMiliSeconds)
	{
		mFixedTimeStep = true;
		mFixedTimeStepMiliSeconds = stepMiliSeconds;
	}

	void MainLoopThread::SetPaused(bool paused)
	{
		mPaused = paused;
		while (mPaused && mDoingStep) boost::this_thread::yield();
		MessageSystem::Instance().SetSendAllMessagesInstantly(mAccessPermissionID, paused);
	}

	void MainLoopThread::Terminate()
	{
		mTerminate = true;
		while (mDoingStep) boost::this_thread::yield();
	}

	void MainLoopThread::doLoop()
	{
		MessageSystem::Instance().ProcessMessages(mAccessPermissionID, mSynchronized);
	}

	void MainLoopThreadSender::MsgProcessingListener::OnFinishSending(AccessPermissionID accessPermissionID)
	{
		mMainLoopThread->onDoLoop(mMainLoopThread->mTimeSinceLastFrameSeconds, mMainLoopThread->mTotalTimeElapsedSeconds);
	}

	void MainLoopThreadSender::doLoop()
	{
		MessageSystem::Instance().ProcessMessages(mAccessPermissionID, mSynchronized, &mProcessingListener);
	}

	void RenderThread::onDoLoop(float timeRel, float timeAbs)
	{
		OgrePhysX::World::getSingleton().syncRenderables();

		Msg updateMsg;
		updateMsg.typeID = GlobalMessageIDs::UPDATE_PER_FRAME;
		updateMsg.params.AddFloat("TIME", timeRel);
		updateMsg.params.AddFloat("TIME_TOTAL", timeAbs);
		MessageSystem::Instance().MulticastMessage(updateMsg);

		//Sound
		Main::Instance().GetSoundManager()->update();

		//Input
		Main::Instance().GetInputManager()->Update();

		//Graphics
		//Ogre::WindowEventUtilities::messagePump();
		Ogre::Root::getSingleton().renderOneFrame();
	}

	PhysicsThread::PhysicsThread() : MainLoopThreadSender(AccessPermissions::ACCESS_PHYSICS)
	{
		Main::Instance().GetPhysXScene()->setSimulationListener(&mPhysicsListener);
	}
	
	void PhysicsThread::onDoLoop(float timeRel, float timeAbs)
	{
		Msg msg;
		msg.typeID = GlobalMessageIDs::PHYSICS_BEGIN;
		MessageSystem::Instance().MulticastMessage(msg);
		OgrePhysX::World::getSingleton().simulate(timeRel);
		//OgrePhysX::World::getSingleton().syncRenderables();
		Msg endPhysicsMsg = msg;
		msg.typeID = GlobalMessageIDs::PHYSICS_END;
		MessageSystem::Instance().MulticastMessage(msg);
	}

	void PhysicsThread::PhysicsListener::onBeginSimulate(float time)
	{
		Msg msg;
		msg.params.AddFloat("TIME", time);
		msg.typeID = GlobalMessageIDs::PHYSICS_SUBSTEP;
		MessageSystem::Instance().MulticastMessage(msg);
	}
	void PhysicsThread::PhysicsListener::onSimulate(float time)
	{
	}
	void PhysicsThread::PhysicsListener::onEndSimulate(float time)
	{
	}

	void SynchronisedThread::onDoLoop(float timeRel, float timeAbs)
	{
	}

};