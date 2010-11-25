
#pragma once

#include "IceIncludes.h"
#include <map>
#include "IceProcessNode.h"


namespace Ice
{
	class PlayAnimationProcess;
	class ProcessNodeQueue;

	class DllExport ProcessNodeManager
	{

	private:
		int mIDCounter;

		std::map<int, std::shared_ptr<ProcessNode>> mProcessNodes;

	public:
		ProcessNodeManager() : mIDCounter(0) {}
		virtual ~ProcessNodeManager() {}

		std::shared_ptr<PlayAnimationProcess> CreatePlayAnimationProcess(Ogre::AnimationState *state);
		std::shared_ptr<ProcessNodeQueue> CreateProcessNodeQueue();
		void RemoveProcessNode(int processID);
		std::shared_ptr<ProcessNode> GetProcessNode(int processID);

		static ProcessNodeManager& Instance();
	};

}
