#pragma once

#include "IceScriptSystem.h"
#include "OgreOggSound.h"
#include "IceMessageListener.h"

class MusicSystem : public Ice::ViewMessageListener
{
public:
	MusicSystem();
	static MusicSystem& GetInstance();

	void SetMood(std::string strMood, bool bActive);
	bool GetMood(std::string strMood);
	void PostEvent(std::string strEvent);

	void ReceiveMessage(Ice::Msg &msg);

	void Clear();
private:
	static std::vector<Ice::ScriptParam> Lua_CreateSound(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_PlaySound(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_SetVolume(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_PlayAt(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_StopAt(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_FadeIn(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_FadeOut(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_SetBPM(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_SetLooping(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_GetLooping(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_GetCursor(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_SetEventCallback(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_SetTimer(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_DeleteTimer(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_GetBeatCounter(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_GetMood(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	//for tests
	static std::vector<Ice::ScriptParam> Lua_SetMood(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);
	static std::vector<Ice::ScriptParam> Lua_PostEvent(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams);


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
		bool bIsLoop;
		Ice::ScriptParam callback;
	};

	static void InsertEvent(SScheduledTask st);

	float m_fBPM;
	float m_fCurrTime;
	std::list<SScheduledTask> m_lTasks;
	std::map<std::string, bool> m_mMoods;
	struct SSoundProperty
	{
		bool bLooping;
		float fLoopTime;
	};
	std::map<std::string, SSoundProperty> m_mProperties;
	Ice::ScriptParam m_EventCallback;
};

#endif