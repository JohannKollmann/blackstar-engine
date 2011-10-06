
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
	* Runs an infinite loop. In every pass, the messages of a certain AccessPermissionID are processed.
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

		AccessPermissionID mAccessPermissionID;

		bool mDoingStep;

		virtual void doLoop();

	public:
		MainLoopThread(AccessPermissionID accessPermissionID) : 
			mAccessPermissionID(accessPermissionID), mDoingStep(false), mSynchronized(false), mFixedTimeStep(false), mPaused(false), mTerminate(false), mTotalTimeElapsed(0), mTotalLastFrameTime(timeGetTime()), mTimeSinceLastFrame(0) {}

		virtual ~MainLoopThread() {}

		void operator() ();

		AccessPermissionID GetAccessPermissionID() { return mAccessPermissionID; }

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
			MainLoopThreadSender *mMainLoopThread;

			void OnFinishSending(AccessPermissionID accessPermissionID);
		};
		MsgProcessingListener mProcessingListener;

		virtual void doLoop();

		virtual void onDoLoop(float timeRel, float timeAbs) = 0;

	public:

		static const MsgTypeID FINISH_MESSAGEPROCESSING = 112358;

		MainLoopThreadSender(AccessPermissionID accessPermissionID) : MainLoopThread(accessPermissionID) { mProcessingListener.mMainLoopThread = this; }
		virtual ~MainLoopThreadSender() {}
	};


	class DllExport PhysicsThread : public MainLoopThreadSender
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

		void onDoLoop(float timeRel, float timeAbs);
	};

	class DllExport RenderThread : public MainLoopThreadSender
	{
	public:
		RenderThread() : MainLoopThreadSender(AccessPermissions::ACCESS_VIEW) {}
		void onDoLoop(float timeRel, float timeAbs);
	};

	class DllExport SynchronisedThread : public MainLoopThreadSender
	{
	public:
		SynchronisedThread() : MainLoopThreadSender(AccessPermissions::ACCESS_ALL) {}
		void onDoLoop(float timeRel, float timeAbs);
	};

	class DllExport IndependantThread : public MainLoopThreadSender
	{
	public:
		IndependantThread() : MainLoopThreadSender(AccessPermissions::ACCESS_NONE) {}
		void onDoLoop(float timeRel, float timeAbs);
	};
};