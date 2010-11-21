
#pragma once

#include "IceIncludes.h"
#include <map>
#include "IceProcessNode.h"


namespace Ice
{

	class DllExport ProcessNodeManager
	{

	private:
		int mIDCounter;

		std::map<int, std::shared_ptr<ProcessNode>> mProcessNodes;

	public:
		ProcessNodeManager() : mIDCounter(0) {}
		~ProcessNodeManager() {}

		template <class T>
		std::shared_ptr<T> CreateProcessNode()
		{
			std::shared_ptr<T> pNode = std::make_shared<T>(mProcessID);
			mProcessNodes.insert<int, std::shared_ptr<ProcessNode>>(mProcessID++, std::static_pointer_cast<ProcessNode, T>(pNode));
			return pNode;
		}
		void RemoveProcessNode(int processID);
		std::shared_ptr<ProcessNode> GetProcessNode(int processID);

		static ProcessNodeManager& Instance();
	};

}
