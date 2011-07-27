
#pragma once

#include "IceProcessNode.h"
#include "IceIncludes.h"
#include "Ogre.h"
#include "IceMessageSystem.h"
#include "IceScriptSystem.h"

namespace Ice
{
	class TimerProcess : public ProcessNode, public SynchronizedMessageListener
	{
	private:
		ScriptParam mScriptCallback;
		float mTimeLeft;
		float mOriginalTime;

	public:
		TimerProcess(float time)
		{
			mOriginalTime = time;
			mTimeLeft = time;
			JoinNewsgroup(GlobalMessageIDs::REPARSE_SCRIPTS_PRE);
			JoinNewsgroup(GlobalMessageIDs::UPDATE_PER_FRAME);
		}
		~TimerProcess()
		{
		}

		void SetScriptCallback(ScriptParam callback)
		{
			mScriptCallback = callback;
		}

		void ReceiveMessage(Msg &msg)
		{
			if (msg.typeID == GlobalMessageIDs::REPARSE_SCRIPTS_PRE)
			{
				mScriptCallback = ScriptParam();
				TerminateProcess();
			}
			else if (msg.typeID == GlobalMessageIDs::UPDATE_PER_FRAME && mIsActive)
			{
				mTimeLeft -= msg.params.GetFloat("TIME");
				if (mTimeLeft <= 0)
				{
					bool terminate = true;

					if (mScriptCallback.getType() == ScriptParam::PARM_TYPE_FUNCTION)
					{
						std::vector<ScriptParam> in = ScriptSystem::GetInstance().RunCallbackFunction(mScriptCallback, std::vector<ScriptParam>(1, ScriptParam(this->GetProcessID())));
						if (in.size() == 1)
							if (in[0].getType() == ScriptParam::PARM_TYPE_BOOL) terminate = !in[0].getBool();
					}

					if (terminate) TerminateProcess();
					else mTimeLeft = mOriginalTime;
				}
			}
		}
	};
}