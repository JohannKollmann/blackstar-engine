
#pragma once

#include "IceIncludes.h"
#include <vector>
#include "IceProcessNode.h"


namespace Ice
{

	class DllExport ProcessNodeQueue : public PersistentProcessNode
	{
	protected:
		virtual void _notifyFinish(int pID) override;

		std::list<int> mQueue;

	public:
		virtual ~ProcessNodeQueue() {}
		virtual void PushFront(std::shared_ptr<ProcessNode> processNode);
		virtual void Enqueue(std::shared_ptr<ProcessNode> processNode);
		virtual void OnSetActive(bool active) override;
		virtual void Remove(std::shared_ptr<ProcessNode> processNode);

		static std::vector<ScriptParam> Lua_ProcessQueue_Create(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_ProcessQueue_PushFront(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_ProcessQueue_Enqueue(Script& caller, std::vector<ScriptParam> vParams);
	};

	class DllExport LoopedProcessNodeQueue : public ProcessNodeQueue
	{
	protected:
		void _notifyFinish(int pID) override;
	public:
		~LoopedProcessNodeQueue() {}

		void PushFront(std::shared_ptr<ProcessNode> processNode) override;
		void Enqueue(std::shared_ptr<ProcessNode> processNode) override;

		static std::vector<ScriptParam> Lua_LoopedProcessQueue_Create(Script& caller, std::vector<ScriptParam> vParams);
	};

}