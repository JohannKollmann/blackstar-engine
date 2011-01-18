
#pragma once

#include "IceProcessNode.h"
#include "IceIncludes.h"
#include "Ogre.h"
#include "IceMessageSystem.h"
#include "IceScriptSystem.h"

namespace Ice
{
	class TimerProcess : public ProcessNode, public MessageListener
	{
	private:
		ScriptParam mScriptCallback;
		float mTimeLeft;
		float mOriginalTime;

	public:
		TimerProcess(ScriptParam scriptCallback, float time)
		{
			mScriptCallback = scriptCallback;
			mOriginalTime = time;
			mTimeLeft = time;
			MessageSystem::Instance().JoinNewsgroup(this, "REPARSE_SCRIPTS");
			MessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
		}

		void ReceiveMessage(Msg &msg)
		{
			if (msg.type == "REPARSE_SCRIPTS")
				TerminateProcess();
			else if (msg.type == "UPDATE_PER_FRAME" && mIsActive)
			{
				mTimeLeft -= msg.params.GetFloat("TIME");
				if (mTimeLeft <= 0)
				{
					std::vector<ScriptParam> in = ScriptSystem::GetInstance().RunCallbackFunction(mScriptCallback, std::vector<ScriptParam>());
					bool terminate = true;
					if (in.size() == 1)
						if (in[0].getType() == ScriptParam::PARM_TYPE_BOOL) terminate = !in[0].getBool();

					if (terminate) TerminateProcess();
					else mTimeLeft = mOriginalTime;
				}
			}
		}
	};
}