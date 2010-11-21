

#include "IceProcessNode.h"
#include "IceProcessNodeManager.h"

namespace Ice
{

	void ProcessNode::AddTriggerOnFinish(std::shared_ptr<ProcessNode> pNode)
	{
		mTriggerOnFinish.push_back(std::weak_ptr<ProcessNode>(pNode));
		pNode->mDependencies.push_back(mProcessID);
		pNode->SetActive(false);
	}

	void ProcessNode::_notifyFinish(int pID)
	{
		mDependencies.remove(pID);
		if (mDependencies.empty())
			SetActive(true);
	}

	void ProcessNode::Terminate()
	{
		SetActive(false);
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

}
