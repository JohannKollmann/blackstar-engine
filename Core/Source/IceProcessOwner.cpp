
#include "IceProcessOwner.h"
#include "IceProcessNodeManager.h"

namespace Ice
{

	ProcessOwner::~ProcessOwner(void)
	{
		destroyAllProcesses();
	}

	void ProcessOwner::destroyAllProcesses()
	{
		ITERATE(i, mCreatedProcesses)
			ProcessNodeManager::Instance().RemoveProcessNode(*i);
		mCreatedProcesses.clear();
	}

	void ProcessOwner::registerProcess(int processID)
	{
		mCreatedProcesses.push_back(processID);
	}

}