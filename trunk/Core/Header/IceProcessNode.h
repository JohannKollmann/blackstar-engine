
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
		std::vector<std::weak_ptr<ProcessNode>> mTriggerOnFinish;
		bool mIsActive;

		int mProcessID;

		virtual void _notifyFinish(int pID);
		virtual void _addDependency(int pID);

		void Init(int id);

		void Terminate();
		virtual void TriggerDependencies();

	public:
		ProcessNode() : mProcessID(0) {}
		virtual ~ProcessNode();

		virtual void AddTriggerOnFinish(std::shared_ptr<ProcessNode> pNode);

		void SetActive(bool active);

		int GetProcessID() const;

		static std::vector<ScriptParam> Lua_AddDependency(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_KillProcess(Script& caller, std::vector<ScriptParam> vParams);

	 protected:

		virtual void OnSetActive(bool active)  = 0;
	};

}
