
#pragma once

#include "IceIncludes.h"
#include <vector>
#include "IceProcessNode.h"


namespace Ice
{

	class ProcessNodeQueue : public ProcessNode
	{
	private:
		std::list<int> mQueue;

	protected:
		void _notifyFinish(int pID) override;

	public:

		void PushFront(std::shared_ptr<ProcessNode> processNode);
		void Enqueue(std::shared_ptr<ProcessNode> processNode);
		void OnSetActive(bool active) override;

		static std::vector<ScriptParam> Lua_ProcessQueue_Create(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_ProcessQueue_PushFront(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_ProcessQueue_Enqueue(Script& caller, std::vector<ScriptParam> vParams);
	};

}