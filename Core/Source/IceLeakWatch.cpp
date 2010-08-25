#include "IceLeakWatch.h"

namespace Ice
{
	LeakManager& LeakManager::getInstance()
	{
		static LeakManager singleton;
		return singleton;
	}

	void LeakWatch::addPointer(void* pPointer)
	{
		mPointer=pPointer;
		LeakManager::getInstance().addPointer(this);
	}
};