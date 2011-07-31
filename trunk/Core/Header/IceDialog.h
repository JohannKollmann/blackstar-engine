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
		void OnReceiveMessage(Msg &msg);

	public:
		Dialog(GOCAI *ai);
		~Dialog(void);	
	};
}