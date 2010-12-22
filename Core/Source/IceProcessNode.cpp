

#include "IceProcessNode.h"
#include "IceProcessNodeManager.h"
#include "IceUtils.h"

namespace Ice
{
	ProcessNode::~ProcessNode()
	{
		TriggerWaitingProcesses();
	}

	void ProcessNode::AddDependencyConnection(ProcessNode *node)
	{
		node->_addTriggerOnFinish(mProcessID);
		_addDependency(node->mProcessID);
	}
	void ProcessNode::_addTriggerOnFinish(int pID)
	{
		mTriggerOnFinish.push_back(pID);
	}
	void ProcessNode::_addDependency(int pID)
	{
		mDependencies.push_back(pID);
		SetActive(false);
	}
	void ProcessNode::_notifyFinish(int pID)
	{
		mDependencies.remove(pID);
		if (mDependencies.empty())
			SetActive(true);
	}
	void ProcessNode::TriggerWaitingProcesses()
	{
		ITERATE(i, mTriggerOnFinish)
		{
			std::shared_ptr<ProcessNode> node = ProcessNodeManager::Instance().GetProcessNode(*i);
			if (node.get()) node->_notifyFinish(GetProcessID());
		}
		mTriggerOnFinish.clear();
	}

	void ProcessNode::Init(int id)
	{
		mProcessID = id;
		SetActive(mDependencies.empty());
	}

	void ProcessNode::TerminateProcess()
	{
		TriggerWaitingProcesses();
		ProcessNodeManager::Instance().RemoveProcessNode(GetProcessID());
	}

	void ProcessNode::SetActive(bool active)
	{
		if (mIsActive != active)
		{
			mIsActive = active;
			OnSetActive(active);
		}
	}

	int ProcessNode::GetProcessID() const
	{
		return mProcessID;
	}

	std::vector<ScriptParam> ProcessNode::Lua_AddDependency(Script& caller, std::vector<ScriptParam> vParams)
	{
		auto err = Utils::TestParameters(vParams, "int int");
		if (err == "")
		{
			std::shared_ptr<ProcessNode> node = ProcessNodeManager::Instance().GetProcessNode(vParams[0].getInt());
			std::shared_ptr<ProcessNode> dependency = ProcessNodeManager::Instance().GetProcessNode(vParams[1].getInt());
			if (node.get() && dependency.get()) node->AddDependencyConnection(dependency.get());
		}
		else SCRIPT_RETURNERROR(err)
		std::vector<ScriptParam> out;
		return out;
	}
	std::vector<ScriptParam> ProcessNode::Lua_KillProcess(Script& caller, std::vector<ScriptParam> vParams)
	{
		auto err = Utils::TestParameters(vParams, "int");
		if (err == "")
		{
			auto node = ProcessNodeManager::Instance().GetProcessNode(vParams[0].getInt());
			if (node) node->TerminateProcess();
		}
		else SCRIPT_RETURNERROR(err) 
		std::vector<ScriptParam> out;
		return out;
	}

	std::vector<ScriptParam> ProcessNode::Lua_SetActive(Script& caller, std::vector<ScriptParam> vParams)
	{
		auto err = Utils::TestParameters(vParams, "int bool");
		if (err == "")
		{
			auto node = ProcessNodeManager::Instance().GetProcessNode(vParams[0].getInt());
			if (node) node->SetActive(vParams[1].getBool());
		}
		else SCRIPT_RETURNERROR(err) 
		std::vector<ScriptParam> out;
		return out;
	}
	std::vector<ScriptParam> ProcessNode::Lua_TriggerWaiting(Script& caller, std::vector<ScriptParam> vParams)
	{
		auto err = Utils::TestParameters(vParams, "int");
		if (err == "")
		{
			auto node = ProcessNodeManager::Instance().GetProcessNode(vParams[0].getInt());
			if (node) node->TriggerWaitingProcesses();
		}
		else SCRIPT_RETURNERROR(err) 
		std::vector<ScriptParam> out;
		return out;
	}

}
