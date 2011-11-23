
#include "IceProcessNodeQueue.h"
#include "IceProcessNodeManager.h"
#include "IceUtils.h"

namespace Ice
{

	void ProcessNodeQueue::PushFront(std::shared_ptr<ProcessNode> processNode)
	{
		ITERATE(i, mQueue)
		{
			if (*i == processNode->GetProcessID())
			{
				IceWarning("Process is already in queue!")
				return;
			}
		}

		ITERATE(i, mQueue)
		{
			auto curr = ProcessNodeManager::Instance().GetProcessNode(*i);
			if (curr.get()) curr->AddDependencyConnection(processNode.get());
		}
		if (!mIsActive) processNode->_addDependency(GetProcessID());
		processNode->_addTriggerOnFinish(GetProcessID());
		mQueue.push_front(processNode->GetProcessID());
	}
	void ProcessNodeQueue::Enqueue(std::shared_ptr<ProcessNode> processNode)
	{
		ITERATE(i, mQueue)
		{
			if (*i == processNode->GetProcessID())
			{
				IceWarning("Process is already in queue!")
				return;
			}
		}

		ITERATE(i, mQueue)
		{
			auto curr = ProcessNodeManager::Instance().GetProcessNode(*i);
			if (curr.get()) processNode->AddDependencyConnection(curr.get());
		}
		if (!mIsActive) processNode->_addDependency(GetProcessID());
		processNode->_addTriggerOnFinish(GetProcessID());
		mQueue.push_back(processNode->GetProcessID());
	}

	void ProcessNodeQueue::Remove(std::shared_ptr<ProcessNode> processNode)
	{
		if (!mIsActive) processNode->_notifyFinish(GetProcessID());
		_notifyFinish(processNode->GetProcessID());
	}

	void ProcessNodeQueue::_notifyFinish(int pID)
	{
		ITERATE(i, mQueue)
		{
			if (*i == pID)
			{
				mQueue.erase(i);			
				if (mQueue.empty()) TriggerWaitingProcesses();
				return;
			}
		}
		ProcessNode::_notifyFinish(pID);	//pID is a regular dependency, invoke parent method
	}
	void ProcessNodeQueue::OnSetActive(bool active)
	{
		if (active)
		{
			ITERATE(i, mQueue)
			{
				auto curr = ProcessNodeManager::Instance().GetProcessNode(*i);
				if (curr.get()) ProcessNodeManager::Instance().GetProcessNode(*i)->_notifyFinish(GetProcessID());
			}
		}
		else if (!mQueue.empty())		//all processes have to wait for this queue
		{
			ITERATE(i, mQueue)
			{
				auto curr = ProcessNodeManager::Instance().GetProcessNode(*i);
				if (curr.get()) curr->_addDependency(GetProcessID());
			}
		}
	}

	void LoopedProcessNodeQueue::_notifyFinish(int pID)
	{
		ProcessNodeQueue::_notifyFinish(pID);
		auto node = ProcessNodeManager::Instance().GetProcessNode(pID);
		if (node.get()) Enqueue(node);
	}
	void LoopedProcessNodeQueue::PushFront(std::shared_ptr<ProcessNode> processNode)
	{
		if (processNode->IsPersistent())
			ProcessNodeQueue::PushFront(processNode);
		else IceWarning("Tried to add non-persistent process to a looped process node queue!")
	}
	void LoopedProcessNodeQueue::Enqueue(std::shared_ptr<ProcessNode> processNode)
	{
		if (processNode->IsPersistent())
			ProcessNodeQueue::Enqueue(processNode);
		else IceWarning("Tried to add non-persistent process to a looped process node queue!")
	}

	std::vector<ScriptParam> ProcessNodeQueue::Lua_ProcessQueue_Create(Script& caller, std::vector<ScriptParam> vParams)
	{
		std::vector<ScriptParam> out;
		std::shared_ptr<ProcessNodeQueue> queue = ProcessNodeManager::Instance().CreateProcessNodeQueue();
		out.push_back(queue->GetProcessID());
		return out;
	}
	std::vector<ScriptParam> LoopedProcessNodeQueue::Lua_LoopedProcessQueue_Create(Script& caller, std::vector<ScriptParam> vParams)
	{
		std::vector<ScriptParam> out;
		std::shared_ptr<LoopedProcessNodeQueue> queue = ProcessNodeManager::Instance().CreateLoopedProcessNodeQueue();
		out.push_back(queue->GetProcessID());
		return out;
	}
	std::vector<ScriptParam> ProcessNodeQueue::Lua_ProcessQueue_PushFront(Script& caller, std::vector<ScriptParam> vParams)
	{
		auto err = Utils::TestParameters(vParams, "int int");
		if (err == "")
		{
			std::shared_ptr<ProcessNode> queueNode = ProcessNodeManager::Instance().GetProcessNode(vParams[0].getInt());
			std::shared_ptr<ProcessNode> node = ProcessNodeManager::Instance().GetProcessNode(vParams[1].getInt());
			if (queueNode.get() && node.get())
			{
				std::shared_ptr<ProcessNodeQueue> queue = std::dynamic_pointer_cast<ProcessNodeQueue, ProcessNode>(queueNode);
				if (queue.get()) queue->PushFront(node);
				else IceWarning("Invalid queue!")
			}
			else IceWarning("Invalid id!")
		}
		else SCRIPT_RETURNERROR(err)
		std::vector<ScriptParam> out;
		return out;
	}
	std::vector<ScriptParam> ProcessNodeQueue::Lua_ProcessQueue_Enqueue(Script& caller, std::vector<ScriptParam> vParams)
	{
		auto err = Utils::TestParameters(vParams, "int int");
		if (err == "")
		{
			std::shared_ptr<ProcessNode> queueNode = ProcessNodeManager::Instance().GetProcessNode(vParams[0].getInt());
			std::shared_ptr<ProcessNode> node = ProcessNodeManager::Instance().GetProcessNode(vParams[1].getInt());
			if (queueNode.get() && node.get())
			{
				std::shared_ptr<ProcessNodeQueue> queue = std::dynamic_pointer_cast<ProcessNodeQueue, ProcessNode>(queueNode);
				if (queue.get()) queue->Enqueue(node);
				else IceWarning("Invalid queue!")
			}
			else IceWarning("Invalid id!")
		}
		else SCRIPT_RETURNERROR(err)
		std::vector<ScriptParam> out;
		return out;
	}
}
