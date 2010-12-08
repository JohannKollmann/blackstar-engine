

#include "IceProcessNode.h"
#include "IceProcessNodeManager.h"
#include "IceUtils.h"

namespace Ice
{
	ProcessNode::~ProcessNode()
	{
		TriggerDependencies();
	}

	void ProcessNode::AddTriggerOnFinish(std::shared_ptr<ProcessNode> pNode)
	{
		mTriggerOnFinish.push_back(std::weak_ptr<ProcessNode>(pNode));
		pNode->_addDependency(mProcessID);
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
	void ProcessNode::TriggerDependencies()
	{
		ITERATE(i, mTriggerOnFinish)
		{
			if (!(*i).expired())
				(*i).lock()->_notifyFinish(GetProcessID());
		}
		mTriggerOnFinish.clear();
	}

	void ProcessNode::Init(int id)
	{
		mProcessID = id;
		SetActive(mDependencies.empty());
	}

	void ProcessNode::Terminate()
	{
		SetActive(false);
		TriggerDependencies();
		ProcessNodeManager::Instance().RemoveProcessNode(mProcessID);
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
			if (node.get() && dependency.get()) dependency->AddTriggerOnFinish(node);
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
			std::shared_ptr<ProcessNode> node = ProcessNodeManager::Instance().GetProcessNode(vParams[0].getInt());
			if (node.get()) node->Terminate();
		}
		else SCRIPT_RETURNERROR(err) 
		std::vector<ScriptParam> out;
		return out;
	}

}
