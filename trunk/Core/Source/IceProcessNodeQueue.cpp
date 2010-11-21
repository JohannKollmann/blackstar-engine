
#include "IceProcessNodeQueue.h"

namespace Ice
{

	void ProcessNodeQueue::PushProcessNode(std::shared_ptr<ProcessNode> processNode)
	{
		mProcessNodes.push_back(processNode);
	}

}
