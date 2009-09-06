
#pragma once

#include "SGTIncludes.h"
#include "SGTGOCAI.h"
#include "SGTScriptSystem.h"
#include "SGTAIState.h"

class SGTScriptedAIState : public SGTAIState
{
protected:
	SGTScript mScript;
	SGTScriptedAIState() {}
	float mLastUpdateCall;
	virtual bool OnSimulate(float time) { return false; }

public:
	SGTScriptedAIState(SGTGOCAI* ai, Ogre::String scriptFileName);
	~SGTScriptedAIState();

	virtual void OnEnter();
	virtual bool OnUpdate(float time);
};

class SGTDayCycle : public SGTScriptedAIState
{
protected:
	std::vector<SGTScriptParam> mScriptParams;
	int mStartTimeH;
	int mStartTimeM;
	int mEndTimeH;		//Stunden
	int mEndTimeM;		//Minuten
	bool mTimeAbs;		//Absolute oder relative Angabe?


public:
	SGTDayCycle(SGTGOCAI* ai, Ogre::String scriptFileName, std::vector<SGTScriptParam> params, int endtimeH, int endtimeM, bool time_abs);
	~SGTDayCycle();

	void OnEnter();
	bool OnUpdate(float time);
};