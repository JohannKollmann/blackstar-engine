
#pragma once

#include "IceIncludes.h"
#include "IceGOCAI.h"
#include "IceScriptSystem.h"

namespace Ice
{

	class ScriptedProcess : public ProcessNode, public IndependantMessageListener
	{
	protected:
		float mLastUpdateCall;
		std::weak_ptr<GameObject> mGameObject;

		std::vector<ScriptParam> mScriptParams;
		ScriptParam mEnterCallback;
		ScriptParam mUpdateCallback;
		ScriptParam mLeaveCallback;

		virtual void OnEnter();
		virtual void OnLeave();
		virtual void OnUpdate(float time);

		virtual void OnReceiveMessage(Msg &msg);

	public:
		ScriptedProcess(std::weak_ptr<GameObject> &gameObject, std::vector<ScriptParam> params);
		virtual ~ScriptedProcess();

		void OnSetActive(bool active);

		void ReceiveMessage(Msg &msg);

		static std::vector<ScriptParam> Lua_SetEnterCallback(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_SetUpdateCallback(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_SetLeaveCallback(Script& caller, std::vector<ScriptParam> vParams);
	};

	class DayCycleProcess : public ScriptedProcess
	{
	protected:
		int mStartTimeH;
		int mStartTimeM;
		int mEndTimeH;		//Stunden
		int mEndTimeM;		//Minuten
		bool mTimeAbs;		//Absolute oder relative Angabe?


	public:
		DayCycleProcess(std::weak_ptr<GameObject> &object, std::vector<ScriptParam> params, int endtimeH, int endtimeM, bool time_abs);
		~DayCycleProcess();

		void OnUpdate(float time);
	};

}