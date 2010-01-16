
#pragma once

#include "IceIncludes.h"
#include "IceGOCAI.h"
#include "IceScriptSystem.h"
#include "IceAIState.h"

namespace Ice
{

class ScriptedAIState : public AIState
{
protected:
	Script mScript;
	ScriptedAIState() {}
	float mLastUpdateCall;
	virtual bool OnSimulate(float time) { return false; }

public:
	ScriptedAIState(GOCAI* ai, Ogre::String scriptFileName);
	~ScriptedAIState();

	virtual void OnEnter();
	virtual bool OnUpdate(float time);
};

class DayCycle : public ScriptedAIState
{
protected:
	std::vector<ScriptParam> mScriptParams;
	int mStartTimeH;
	int mStartTimeM;
	int mEndTimeH;		//Stunden
	int mEndTimeM;		//Minuten
	bool mTimeAbs;		//Absolute oder relative Angabe?


public:
	DayCycle(GOCAI* ai, Ogre::String scriptFileName, std::vector<ScriptParam> params, int endtimeH, int endtimeM, bool time_abs);
	~DayCycle();

	void OnEnter();
	bool OnUpdate(float time);
};

};