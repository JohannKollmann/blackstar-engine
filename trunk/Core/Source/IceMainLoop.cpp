
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
	}

	void MainLoopThread::Terminate()
	{
		mTerminate = true;
		while (mDoingStep) boost::this_thread::yield();
	}

	void MainLoopThread::doLoop()
	{
		MessageSystem::Instance().ProcessMessages(mAccessPermitionID, mSynchronized);
	}

	void MainLoopThreadSender::MsgProcessingListener::OnFinishSending(AccessPermitionID accessPermitionID)
	{
		Msg msg;
		msg.typeID = mPerLoopMsg;
		msg.params.AddFloat("TIME", mMainLoopThread->mTimeSinceLastFrameSeconds);
		msg.params.AddFloat("TIME_TOTAL", mMainLoopThread->mTotalTimeElapsedSeconds);
		MessageSystem::Instance().SendMessage(msg, accessPermitionID, mMsgReceiver);
	}

	void MainLoopThreadSender::doLoop()
	{
		MessageSystem::Instance().ProcessMessages(mAccessPermitionID, mSynchronized, &mProcessingListener);
	}

	void RenderThread::ReceiveMessage(Msg &msg)
	{
		if (msg.typeID == GlobalMessageIDs::RENDERING_BEGIN)
		{
			Msg updateMsg = msg;
			updateMsg.typeID = GlobalMessageIDs::UPDATE_PER_FRAME;
			MulticastMessage(updateMsg);

			//Sound
			Main::Instance().GetSoundManager()->update();

			//Input
			Main::Instance().GetInputManager()->Update();

			//Graphics
			//Ogre::WindowEventUtilities::messagePump();
			Ogre::Root::getSingleton().renderOneFrame();
		}
	}

	PhysicsThread::PhysicsThread()
	{
		Main::Instance().GetPhysXScene()->setSimulationListener(&mPhysicsListener);
	}
	
	void PhysicsThread::ReceiveMessage(Msg &msg)
	{
		if (msg.typeID == GlobalMessageIDs::PHYSICS_BEGIN)
		{
			MulticastMessage(msg);
			OgrePhysX::World::getSingleton().startSimulate(msg.params.GetValue<float>(0));
			OgrePhysX::World::getSingleton().syncRenderables();
			Msg endPhysicsMsg = msg;
			msg.typeID = GlobalMessageIDs::PHYSICS_END;
			MulticastMessage(msg);
		}
	}

	void PhysicsThread::PhysicsListener::onBeginSimulate(float time)
	{
		Msg msg;
		msg.params.AddFloat("TIME", time);
		msg.typeID = GlobalMessageIDs::PHYSICS_SUBSTEP;
		MessageSystem::Instance().MulticastMessage(msg, AccessPermitions::ACCESS_PHYSICS);
	}
	void PhysicsThread::PhysicsListener::onSimulate(float time)
	{
	}
	void PhysicsThread::PhysicsListener::onEndSimulate(float time)
	{
	}

};