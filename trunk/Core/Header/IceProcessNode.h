
#pragma once

#include "IceIncludes.h"
#include <list>
#include <vector>
#include "IceScriptSystem.h"

namespace Ice
{

	class DllExport ProcessNode
	{
		friend class ProcessNodeManager;

	protected:
		std::list<int> mDependencies;		//list of process ids
		std::vector<int> mTriggerOnFinish;
		bool mIsActive;
		bool mIsPaused;

		int mProcessID;

		void Init(int id);

	public:
		ProcessNode() : mProcessID(0), mIsActive(true), mIsPaused(false) {}
		virtual ~ProcessNode();

		/**
		Notifies the process that another process terminated.
		@param pID process ID of the process that terminated
		*/
		virtual void _notifyFinish(int pID);

		/**
		Adds a dependency process. This process wil be stopped until all dependency have terminated.
		@param pID the dependency process
		*/
		virtual void _addDependency(int pID);

		/**
		Adds a process that is notified when this process terminates.
		@param pID the process that gets notified
		*/
		void _addTriggerOnFinish(int pID);

		/**
		Notifies all waiting processes that this process terminated.
		*/
		virtual void TriggerWaitingProcesses();

		/**
		Adds a process as dependency of this one.
		@param pNode The dependency process ID.
		*/
		virtual void AddDependencyConnection(ProcessNode *node);

		/**
		Terminates the process.
		*/
		virtual void TerminateProcess();

		/**
		Pauses/Unpauses the process, for example because a dependency was added / finished.
		@param active unpause or pause
		*/
		void _setActive(bool active);

		/**
		Calls _setActive(true) if all requirements are met.
		*/
		void _refreshIsActive();

		///Notifies all processes waiting for this one, clears all dependencies and pauses this process.
		void Suspend();

		///Unpauses the process.
		void Activate();

		/**
		Retrieves whether the process is active or not.
		@return active yes/no
		*/
		bool GetIsActive() { return mIsActive; }

		/**
		Retrieves whether the process is paused or not.
		GetIsPaused implies !GetIsActive. 
		@return active yes/no
		*/
		bool GetIsPaused() { return mIsPaused; }

		/**
		Retrieves the process ID.
		*/
		int GetProcessID() const;

		static std::vector<ScriptParam> Lua_AddDependency(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_Activate(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_Suspend(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_KillProcess(Script& caller, std::vector<ScriptParam> vParams);

	 protected:

		 virtual void OnSetActive(bool active)  {}
	};

}
