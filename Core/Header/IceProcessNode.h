
#pragma once

#include "IceIncludes.h"
#include <list>
#include <vector>

namespace Ice
{

	class DllExport ProcessNode
	{
		friend class ProcessNodeManager;

	private:
		std::list<int> mDependencies;		//list of process ids
		std::vector<std::weak_ptr<ProcessNode>> mTriggerOnFinish;
		bool mIsActive;

		int mProcessID;

		void _notifyFinish(int pID);

		ProcessNode(int id) : mProcessID(id) {}

	public:

		virtual ~ProcessNode() {}

		void AddTriggerOnFinish(std::shared_ptr<ProcessNode> pNode);

		void Terminate();

		void SetActive(bool active);

		int GetProcessID() const;

	 protected:

		virtual void OnSetActive(bool active)  = 0;
	};

}
