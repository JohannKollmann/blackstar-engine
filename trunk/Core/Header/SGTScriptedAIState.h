
#pragma once

#include "SGTIncludes.h"
#include "SGTGOCAI.h"
#include "SGTScriptSystem.h"

class SGTScriptedAIState
{
protected:
	SGTGOCAI *mAIObject;
	SGTScript mScript;
	int mStartTimeH;
	int mStartTimeM;
	int mEndTimeH;		//Stunden
	int mEndTimeM;		//Minuten
	bool mTimeAbs;		//Absolute oder relative Angabe?


public:
	SGTScriptedAIState(SGTGOCAI* ai, Ogre::String scriptFileName, int endtimeH, int endtimeM, bool time_abs);
	~SGTScriptedAIState();

	void OnEnter();
	bool OnUpdate();
};