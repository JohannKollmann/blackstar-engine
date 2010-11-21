
#pragma once

#include "IceIncludes.h"
#include <vector>
#include "IceProcessNode.h"


namespace Ice
{

	class ProcessNodeQueue : public ProcessNode
	{

	private:

		std::vector<std::shared_ptr<ProcessNode>> mProcessNodes;

	public:

		void PushProcessNode(std::shared_ptr<ProcessNode> processNode);
	};

}