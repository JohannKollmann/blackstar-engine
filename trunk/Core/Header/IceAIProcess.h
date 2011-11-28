
#pragma once

#include "IceIncludes.h"
#include "IceProcessNode.h"
#include "IceMessageListener.h"

namespace Ice
{

	class DllExport AIProcess : public ProcessNode, public IndependantMessageListener
	{
	public:

	protected:
		std::weak_ptr<GOCAI> mAIObject;
		AIProcess() : mAIObject(std::weak_ptr<GOCAI>()) { };

	public:
		virtual ~AIProcess() {};

		void SetAI(std::shared_ptr<GOCAI> ai) { mAIObject = std::weak_ptr<GOCAI>(ai); };
	};

};