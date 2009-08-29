
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
	Ogre::String mWaypoint;
	int mStartTimeH;
	int mStartTimeM;
	int mEndTimeH;		//Stunden
	int mEndTimeM;		//Minuten
	bool mTimeAbs;		//Absolute oder relative Angabe?


public:
	SGTDayCycle(SGTGOCAI* ai, Ogre::String scriptFileName, Ogre::String waypoint, int endtimeH, int endtimeM, bool time_abs);
	~SGTDayCycle();

	bool OnUpdate(float time);
};