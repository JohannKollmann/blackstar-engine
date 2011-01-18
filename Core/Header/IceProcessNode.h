
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

		int mProcessID;

		void Init(int id);

	public:
		ProcessNode() : mProcessID(0), mIsActive(true) {}
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
		void SetActive(bool active);

		/**
		Retrieves the process ID.
		*/
		int GetProcessID() const;

		static std::vector<ScriptParam> Lua_AddDependency(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_SetActive(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_TriggerWaiting(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_KillProcess(Script& caller, std::vector<ScriptParam> vParams);

	 protected:

		 virtual void OnSetActive(bool active)  {}
	};

}
