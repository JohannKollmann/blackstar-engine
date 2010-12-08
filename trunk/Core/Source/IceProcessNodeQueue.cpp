
#include "IceProcessNodeQueue.h"
#include "IceProcessNodeManager.h"
#include "IceUtils.h"

namespace Ice
{

	void ProcessNodeQueue::PushFront(std::shared_ptr<ProcessNode> processNode)
	{
		if (!mQueue.empty()) processNode->AddTriggerOnFinish(ProcessNodeManager::Instance().GetProcessNode(mQueue.front()));
		if (!mIsActive) processNode->_addDependency(GetProcessID());
	}
	void ProcessNodeQueue::Enqueue(std::shared_ptr<ProcessNode> processNode)
	{
		if (!mQueue.empty()) ProcessNodeManager::Instance().GetProcessNode(mQueue.front())->AddTriggerOnFinish(processNode);
		if (!mIsActive) processNode->_addDependency(GetProcessID());
	}
	void ProcessNodeQueue::_notifyFinish(int pID)
	{
		ITERATE(i, mQueue)
		{
			if (*i == pID)
			{
				mQueue.erase(i);
				if (mQueue.empty()) TriggerDependencies();
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
				ProcessNodeManager::Instance().GetProcessNode(*i)->_notifyFinish(GetProcessID());
		}
	}

	std::vector<ScriptParam> ProcessNodeQueue::Lua_ProcessQueue_Create(Script& caller, std::vector<ScriptParam> vParams)
	{
		std::vector<ScriptParam> out;
		std::shared_ptr<ProcessNodeQueue> queue = ProcessNodeManager::Instance().CreateProcessNodeQueue();
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
