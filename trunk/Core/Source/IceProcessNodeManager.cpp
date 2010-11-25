
#include "IceProcessNodeManager.h"
#include "IcePlayAnimationProcess.h"
#include "IceProcessNodeQueue.h"

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

	std::shared_ptr<ProcessNodeQueue> ProcessNodeManager::CreateProcessNodeQueue()
	{
		std::shared_ptr<ProcessNodeQueue> pNode = std::make_shared<ProcessNodeQueue>();
		pNode->Init(mIDCounter);
		mProcessNodes.insert(std::make_pair<int, std::shared_ptr<ProcessNode>>(mIDCounter++, std::static_pointer_cast<ProcessNode, ProcessNodeQueue>(pNode)));
		return pNode;
	}

	std::shared_ptr<PlayAnimationProcess> ProcessNodeManager::CreatePlayAnimationProcess(Ogre::AnimationState *state)
	{
		std::shared_ptr<PlayAnimationProcess> pNode = std::make_shared<PlayAnimationProcess>(state);
		pNode->Init(mIDCounter);
		mProcessNodes.insert(std::make_pair<int, std::shared_ptr<ProcessNode>>(mIDCounter++, std::static_pointer_cast<ProcessNode, PlayAnimationProcess>(pNode)));
		return pNode;
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