#ifndef __SOUND_SYSTEM__
#define __SOUND_SYSTEM__

#include "SGTScriptSystem.h"
#include "OgreOggSound.h"
#include "SGTMessageListener.h"

class SGTMusicSystem : public SGTMessageListener
{
public:
	SGTMusicSystem();
	static SGTMusicSystem& GetInstance();

	void SetMood(std::string strMood, bool bActive);
	bool GetMood(std::string strMood);
	void PostEvent(std::string strEvent);

	void ReceiveMessage(SGTMsg &msg);
private:
	static std::vector<SGTScriptParam> Lua_CreateSound(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_PlaySound(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_SetVolume(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_PlayAt(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_StopAt(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_FadeIn(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_FadeOut(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_SetBPM(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_SetLooping(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_GetLooping(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_GetCursor(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_SetEventCallback(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_SetTimer(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_DeleteTimer(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_GetBeatCounter(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_GetMood(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_SetMood(SGTScript& caller, std::vector<SGTScriptParam> vParams);

	struct SScheduledTask
	{
		float fTime;
		enum ETaskType
		{
			TASK_TYPE_PLAY,
			TASK_TYPE_STOP,
			TASK_TYPE_FADE_IN,
			TASK_TYPE_FADE_OUT,
			TASK_TYPE_CALL_TIMER
		} task;
		std::string strSound;
		float fFadeTime;
		SGTScriptParam callback;
	};

	float m_fBPM;
	float m_fCurrTime;
	std::list<SScheduledTask> m_lTasks;
	std::map<std::string, bool> m_mMoods;
	SGTScriptParam m_EventCallback;
};

#endif