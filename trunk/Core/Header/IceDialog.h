#pragma once

#include "IceAIProcess.h"
#include "IceDirectionBlender.h"

namespace Ice
{

	class DllExport Dialog : public AIProcess
	{
	private:
		DirectionYawBlender mDirectionBlender;

	protected:
		void OnSetActive(bool active);
		void ReceiveMessage(Msg &msg);

	public:
		Dialog(std::shared_ptr<GOCAI> ai);
		~Dialog(void);	
	};
}