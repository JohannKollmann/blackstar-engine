
#pragma once

#include "IceIncludes.h"
#include "IceProcessNode.h"
#include "IceMessageListener.h"

namespace Ice
{

	class AIProcess : public ProcessNode, public MessageListener
	{
	public:

	protected:
		GOCAI *mAIObject;
		AIProcess() : mAIObject(nullptr) { };

		virtual void OnReceiveMessage(Msg &msg) {}

	public:
		virtual ~AIProcess() {};

		void SetAI(GOCAI *ai) { mAIObject = ai; };

		virtual void ReceiveMessage(Msg &msg)
		{
			if (mIsActive) OnReceiveMessage(msg);
		}
	};

};