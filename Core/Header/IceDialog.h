#pragma once

#include "IceAIState.h"
#include "IceDirectionBlender.h"

namespace Ice
{

	class DllExport Dialog : public AIState
	{
	private:
		DirectionYawBlender mDirectionBlender;

	public:
		Dialog(GOCAI *ai);
		~Dialog(void);

		float GetPriority() const { return 1.0f; }

		void OnEnter();
		bool Update(float time);
		void Leave();
	};
}