
#include "IceProcessNodeQueue.h"
#include "IceProcessNodeManager.h"
#include "IceUtils.h"

namespace Ice
{

	void ProcessNodeQueue::PushFront(std::shared_ptr<ProcessNode> processNode)
	{
		if (!mDependencies.empty()) processNode->AddTriggerOnFinish(ProcessNodeManager::Instance().GetProcessNode(mDependencies.front()));
		processNode->AddTriggerOnFinish(ProcessNodeManager::Instance().GetProcessNode(GetProcessID()));
	}
	void ProcessNodeQueue::Enqueue(std::shared_ptr<ProcessNode> processNode)
	{
		if (!mDependencies.empty()) ProcessNodeManager::Instance().GetProcessNode(mDependencies.back())->AddTriggerOnFinish(processNode);
		processNode->AddTriggerOnFinish(ProcessNodeManager::Instance().GetProcessNode(GetProcessID()));
	}
	void ProcessNodeQueue::OnSetActive(bool active)
	{
		if (active) TriggerDependencies();		//Trigger dependencies when queue is empty
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
		if (Ice::Utils::TestParameters(caller, vParams, "int int"))
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
		std::vector<ScriptParam> out;
		return out;
	}
	std::vector<ScriptParam> ProcessNodeQueue::Lua_ProcessQueue_Enqueue(Script& caller, std::vector<ScriptParam> vParams)
	{
		if (Ice::Utils::TestParameters(caller, vParams, "int int"))
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
		std::vector<ScriptParam> out;
		return out;
	}
}
