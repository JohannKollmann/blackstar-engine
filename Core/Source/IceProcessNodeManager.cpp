
#include "IceProcessNodeManager.h"

namespace Ice
{

	std::shared_ptr<ProcessNode> ProcessNodeManager::GetProcessNode(int processID)
	{
		auto i = mProcessNodes.find(processID);
		if (i == mProcessNodes.end())
		{
			IceWarning("Process ID not found")
			return nullptr;
		}
		return i->second;
	}


	void ProcessNodeManager::RemoveProcessNode(int processID)
	{
		auto i = mProcessNodes.find(processID);
		IceAssert(i != mProcessNodes.end())
		mProcessNodes.erase(i);
	}

	//Singleton
	ProcessNodeManager& ProcessNodeManager::Instance()
	{
		static ProcessNodeManager TheOneAndOnly;
		return TheOneAndOnly;
	}

}