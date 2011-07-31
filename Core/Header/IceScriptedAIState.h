
#pragma once

#include "IceIncludes.h"
#include "IceGOCAI.h"
#include "IceScriptSystem.h"

namespace Ice
{

	class ScriptedAIProcess : public ProcessNode, public IndependantMessageListener
	{
	protected:
		float mLastUpdateCall;
		std::weak_ptr<GameObject> mGameObject;

	public:
		ScriptedAIProcess(std::weak_ptr<GameObject> &gameObject);
		~ScriptedAIProcess();

		void OnSetActive(bool active) override;

		void FireEnter();
		void FireLeave();
		void FireUpdate();

		void ReceiveMessage(Msg &msg);
	};

	class DayCycleProcess : public ScriptedAIProcess
	{
	protected:
		std::vector<ScriptParam> mScriptParams;
		int mStartTimeH;
		int mStartTimeM;
		int mEndTimeH;		//Stunden
		int mEndTimeM;		//Minuten
		bool mTimeAbs;		//Absolute oder relative Angabe?


	public:
		DayCycleProcess(std::weak_ptr<GameObject> &object, std::vector<ScriptParam> params, int endtimeH, int endtimeM, bool time_abs);
		~DayCycleProcess();

		void OnEnter();
		bool Update(float time);
	};

}