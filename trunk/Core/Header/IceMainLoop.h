
#pragma once

#include "IceIncludes.h"
#define NOMINMAX
#include "windows.h"
#include "mmsystem.h"
#include "Ogre.h"
#include "OgrePhysX.h"
#include "boost/thread.hpp"
#include "IceMessageSystem.h"

namespace Ice
{
	/**
	* Manages an infinite loop where it sends a specified message to a specified receiver.
	* Can be used with boost::thread.
	*/
	class DllExport MainLoopThread
	{
	protected:
		bool mPaused;
		bool mTerminate;
		bool mFixedTimeStep;
		DWORD mFixedTimeStepMiliSeconds;
		DWORD mTotalLastFrameTime;
		DWORD mTimeSinceLastFrame;
		DWORD mTotalTimeElapsed;
		float mTimeSinceLastFrameSeconds;
		float mTotalTimeElapsedSeconds;
		
		bool mSynchronized;

		AccessPermitionID mAccessPermitionID;

		bool mDoingStep;

		virtual void doLoop();

	public:
		MainLoopThread(AccessPermitionID accessPermitionID) : 
			mAccessPermitionID(accessPermitionID), mDoingStep(false), mSynchronized(false), mFixedTimeStep(false), mPaused(false), mTerminate(false), mTotalTimeElapsed(0), mTotalLastFrameTime(timeGetTime()), mTimeSinceLastFrame(0) {}

		virtual ~MainLoopThread() {}

		void operator() ();

		AccessPermitionID GetAccessPermitionID() { return mAccessPermitionID; }

		void Step();

		void SetSynchronized(bool synchronized) { mSynchronized = synchronized; }

		void SetFixedTimeStep(DWORD stepMiliSeconds);

		void SetPaused(bool paused);

		void Terminate();
	};

	class DllExport MainLoopThreadSender : public MainLoopThread
	{
	protected:	
		class  DllExport MsgProcessingListener : public MessageSystem::ProcessingListener
		{
		public:
			MessageListener *mMsgReceiver;
			MainLoopThreadSender *mMainLoopThread;

			void OnFinishSending(AccessPermitionID accessPermitionID);
		};
		MsgProcessingListener mProcessingListener;

		virtual void doLoop();

	public:

		static const MsgTypeID FINISH_MESSAGEPROCESSING = 112358;

		MainLoopThreadSender(MessageListener *msgReceiver) : MainLoopThread(msgReceiver->GetAccessPermitionID()) { mProcessingListener.mMainLoopThread = this; mProcessingListener.mMsgReceiver = msgReceiver; }
		virtual ~MainLoopThreadSender() {}
	};


	class DllExport PhysicsThread : public PhysicsMessageListener
	{
	private:
		class DllExport PhysicsListener : public OgrePhysX::Scene::SimulationListener
		{
			void onBeginSimulate(float time);
			void onSimulate(float time);
			void onEndSimulate(float time);
		};
		PhysicsListener mPhysicsListener;

	public:
		PhysicsThread();
		void ReceiveMessage(Msg &msg);
	};

	class DllExport RenderThread : public ViewMessageListener
	{
	public:
		void ReceiveMessage(Msg &msg);
	};
};