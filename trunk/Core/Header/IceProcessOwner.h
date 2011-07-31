#pragma once

#include "IceIncludes.h"
#include "IceGOComponent.h"
#include <vector>

namespace Ice
{
	class DllExport ProcessOwner
	{
	private:
		std::vector<int> mCreatedProcesses;

	public:
		ProcessOwner(void) {}
		virtual ~ProcessOwner(void);

	protected:
		void destroyAllProcesses();
		void registerProcess(int processID);
	};
}