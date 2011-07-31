
#include "IceScriptedAIState.h"
#include "mmsystem.h"
#include "IceSceneManager.h"

namespace Ice
{

	ScriptedAIProcess::ScriptedAIProcess(std::weak_ptr<GameObject> &gameObject)
	{
		mLastUpdateCall = 0.0f;
		mGameObject = gameObject;
	}

	ScriptedAIProcess::~ScriptedAIProcess()
	{
	}

	void ScriptedAIProcess::OnSetActive(bool active)
	{
		if (active) FireEnter();
		else FireLeave();
	}

	void ScriptedAIProcess::FireEnter()
	{
	}
	void ScriptedAIProcess::FireUpdate()
	{
	}
	void ScriptedAIProcess::FireLeave()
	{
	}

	void ScriptedAIProcess::ReceiveMessage(Msg &msg)
	{
	}


	DayCycleProcess::DayCycleProcess(std::weak_ptr<GameObject> &object, std::vector<ScriptParam> params, int endtimeH, int endtimeM, bool time_abs)
	: ScriptedAIProcess(object)
	{
		mLastUpdateCall = 0.0f;
		mScriptParams = params;
		mEndTimeH = endtimeH;
		mEndTimeM = endtimeM;
		mStartTimeH = SceneManager::Instance().GetHour();
		mStartTimeM = SceneManager::Instance().GetMinutes();
		if (!time_abs)
		{
			mEndTimeM += mStartTimeH;
			if (mEndTimeM > 60)
			{
				mEndTimeM -= 60;
				mStartTimeH++;
				if (mEndTimeH > 24) mEndTimeH -= 24;
			}
			mEndTimeH += mStartTimeH; if (mEndTimeH > 24) mEndTimeH -= 24;
		}
	}

	DayCycleProcess::~DayCycleProcess()
	{
	}

	void DayCycleProcess::OnEnter()
	{
		std::vector<ScriptParam> params;
		params.push_back(ScriptParam((int)mGameObject.lock()->GetID()));
		for (std::vector<ScriptParam>::iterator i = mScriptParams.begin(); i != mScriptParams.end(); i++)
		{
			params.push_back((*i));
		}
		mScriptParams.clear();
		mScriptParams = params;
	}

	bool DayCycleProcess::Update(float time)
	{
		int hour = SceneManager::Instance().GetHour();
		int minutes = SceneManager::Instance().GetMinutes();
		if (hour == mEndTimeH && minutes > mEndTimeM) return true;

		if (timeGetTime() - mLastUpdateCall > 0.1f)
		{
			mLastUpdateCall = (float)timeGetTime();
		}
		return false;
	}

}