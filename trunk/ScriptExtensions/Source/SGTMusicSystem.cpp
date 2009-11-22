#include "SGTMusicSystem.h"
#include "SGTSceneManager.h"
#include "SGTUtils.h"

SGTMusicSystem::SGTMusicSystem()
{
	SGTScriptSystem::GetInstance().ShareCFunction("music_play", Lua_PlaySound);
	SGTScriptSystem::GetInstance().ShareCFunction("music_create_sound", Lua_CreateSound);
	SGTScriptSystem::GetInstance().ShareCFunction("music_set_volume", Lua_SetVolume);
	SGTScriptSystem::GetInstance().ShareCFunction("music_play_at", Lua_PlayAt);
	SGTScriptSystem::GetInstance().ShareCFunction("music_stop_at", Lua_StopAt);
	SGTScriptSystem::GetInstance().ShareCFunction("music_fade_in", Lua_FadeIn);
	SGTScriptSystem::GetInstance().ShareCFunction("music_fade_out", Lua_FadeOut);
	SGTScriptSystem::GetInstance().ShareCFunction("music_set_bpm", Lua_SetBPM);
	SGTScriptSystem::GetInstance().ShareCFunction("music_set_looping", Lua_SetLooping);
	SGTScriptSystem::GetInstance().ShareCFunction("music_get_looping", Lua_GetLooping);
	SGTScriptSystem::GetInstance().ShareCFunction("music_get_cursor", Lua_GetCursor);
	SGTScriptSystem::GetInstance().ShareCFunction("music_set_event_callback", Lua_SetEventCallback);
	SGTScriptSystem::GetInstance().ShareCFunction("music_set_timer", Lua_SetTimer);
	SGTScriptSystem::GetInstance().ShareCFunction("music_delete_timer", Lua_DeleteTimer);
	SGTScriptSystem::GetInstance().ShareCFunction("music_get_beat_counter", Lua_GetBeatCounter);
	SGTScriptSystem::GetInstance().ShareCFunction("music_get_mood", Lua_GetMood);
	SGTScriptSystem::GetInstance().ShareCFunction("music_set_mood", Lua_SetMood);

	SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
}

SGTMusicSystem&
SGTMusicSystem::GetInstance()
{
	static SGTMusicSystem singleton;
	return singleton;
}

void
SGTMusicSystem::ReceiveMessage(SGTMsg &msg)
{
	if (msg.mNewsgroup == "UPDATE_PER_FRAME")
	{
		float time = msg.mData.GetFloat("TIME_TOTAL");
		m_fCurrTime=time;
		for(std::list<SScheduledTask>::iterator it=m_lTasks.begin(); it->fTime<=m_fCurrTime; )
		{
			OgreOggSound::OgreOggISound* pSound=OgreOggSound::OgreOggSoundManager::getSingleton().getSound(it->strSound);
			switch(it->task)
			{
			case SScheduledTask::TASK_TYPE_FADE_IN:
				break;
			case SScheduledTask::TASK_TYPE_FADE_OUT:
				break;
			case SScheduledTask::TASK_TYPE_PLAY:
				pSound->stop();
				pSound->play();
				break;
			case SScheduledTask::TASK_TYPE_STOP:
				pSound->stop();
				break;
			case SScheduledTask::TASK_TYPE_CALL_TIMER:
			{
				SGTScriptSystem::RunCallbackFunction(it->callback, std::vector<SGTScriptParam>(1, SGTScriptParam(m_fCurrTime*m_fBPM/60.0f)));
				/*float fInsertionTime=ceilf(m_fCurrTime/it->fFadeTime)*it->fFadeTime;
				for(std::list<SScheduledTask>::iterator it2=m_lTasks.begin(); it2!=m_lTasks.end(); it2++)
				{
					if(it2->fTime>fInsertionTime)
					{
						SScheduledTask st;
						st.fTime=fInsertionTime;
						st.task=SScheduledTask::TASK_TYPE_CALL_TIMER;
						st.fFadeTime=it->fFadeTime;
						st.callback=it->callback;
						GetInstance().m_lTasks.insert(it2, st);
						break;
					}
				}*/
				break;
			}
			}
			m_lTasks.pop_front();
			it=m_lTasks.begin();
		}
	}
}

void
SGTMusicSystem::SetMood(std::string strMood, bool bActive)
{
	m_mMoods[strMood]=bActive;
}

bool
SGTMusicSystem::GetMood(std::string strMood)
{
	return m_mMoods[strMood];
}

void
SGTMusicSystem::PostEvent(std::string strEvent)
{
	SGTScriptSystem::GetInstance().RunCallbackFunction(m_EventCallback, std::vector<SGTScriptParam>(1, SGTScriptParam(strEvent)));
}

std::vector<SGTScriptParam>
SGTMusicSystem::Lua_CreateSound(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> errout(1, SGTScriptParam());
	std::vector<SGTScriptParam> vRef=std::vector<SGTScriptParam>(1, SGTScriptParam(std::string()));
	std::string strErrString=SGTUtils::TestParameters(vParams, vRef);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}

	Ogre::String strID=SGTSceneManager::Instance().RequestIDStr();
	OgreOggSound::OgreOggSoundManager::getSingleton().createSound(strID, vParams[0].getString());

	return std::vector<SGTScriptParam>(1, SGTScriptParam(strID));
}

std::vector<SGTScriptParam>
SGTMusicSystem::Lua_PlaySound(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> errout(1, SGTScriptParam());
	std::vector<SGTScriptParam> vRef=std::vector<SGTScriptParam>(1, SGTScriptParam(std::string()));
	std::string strErrString=SGTUtils::TestParameters(vParams, vRef);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	OgreOggSound::OgreOggISound* pSound;
	if((pSound=OgreOggSound::OgreOggSoundManager::getSingleton().getSound(vParams[0].getString()))==NULL)
	{
		errout.push_back(std::string("found no sound for the given ID"));
		return errout;
	}
	pSound->stop();
	pSound->play();
	return std::vector<SGTScriptParam>();
}

std::vector<SGTScriptParam>
SGTMusicSystem::Lua_SetVolume(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> errout(1, SGTScriptParam());
	std::vector<SGTScriptParam> vRef=std::vector<SGTScriptParam>(1, SGTScriptParam(0.1));
	std::string strErrString=SGTUtils::TestParameters(vParams, vRef);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	errout.push_back(std::string("not implemented!!"));
	return errout;
	
	return std::vector<SGTScriptParam>();
}

std::vector<SGTScriptParam>
SGTMusicSystem::Lua_PlayAt(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> errout(1, SGTScriptParam());
	std::vector<SGTScriptParam> vRef=std::vector<SGTScriptParam>(1, SGTScriptParam(std::string()));
	vRef.push_back(SGTScriptParam(0.1));
	std::string strErrString=SGTUtils::TestParameters(vParams, vRef);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	if((OgreOggSound::OgreOggSoundManager::getSingleton().getSound(vParams[0].getString()))==NULL)
	{
		errout.push_back(std::string("found no sound for the given ID"));
		return errout;
	}
	for(std::list<SScheduledTask>::iterator it=GetInstance().m_lTasks.begin(); it!=GetInstance().m_lTasks.end(); it++)
	{
		SScheduledTask st;
		st.fTime=(float)vParams[1].getFloat();
		st.strSound=vParams[0].getString();
		st.task=SScheduledTask::TASK_TYPE_PLAY;
		if(it->fTime>vParams[1].getFloat())
			GetInstance().m_lTasks.insert(it, st);
	}
	return std::vector<SGTScriptParam>();
}
std::vector<SGTScriptParam>
SGTMusicSystem::Lua_StopAt(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> errout(1, SGTScriptParam());
	std::vector<SGTScriptParam> vRef=std::vector<SGTScriptParam>(1, SGTScriptParam(std::string()));
	vRef.push_back(SGTScriptParam(0.1));
	std::string strErrString=SGTUtils::TestParameters(vParams, vRef);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	if((OgreOggSound::OgreOggSoundManager::getSingleton().getSound(vParams[0].getString()))==NULL)
	{
		errout.push_back(std::string("found no sound for the given ID"));
		return errout;
	}
	for(std::list<SScheduledTask>::iterator it=GetInstance().m_lTasks.begin(); it!=GetInstance().m_lTasks.end(); it++)
	{
		SScheduledTask st;
		st.fTime=(float)vParams[1].getFloat();
		st.strSound=vParams[0].getString();
		st.task=SScheduledTask::TASK_TYPE_STOP;
		if(it->fTime>vParams[1].getFloat())
			GetInstance().m_lTasks.insert(it, st);
	}
	return std::vector<SGTScriptParam>();
}
std::vector<SGTScriptParam>
SGTMusicSystem::Lua_FadeIn(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> errout(1, SGTScriptParam());
	std::vector<SGTScriptParam> vRef=std::vector<SGTScriptParam>(1, SGTScriptParam(std::string()));
	vRef.push_back(SGTScriptParam(0.1));
	vRef.push_back(SGTScriptParam(0.1));
	std::string strErrString=SGTUtils::TestParameters(vParams, vRef);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	if((OgreOggSound::OgreOggSoundManager::getSingleton().getSound(vParams[0].getString()))==NULL)
	{
		errout.push_back(std::string("found no sound for the given ID"));
		return errout;
	}
	SScheduledTask st;
	st.fTime=(float)vParams[1].getFloat();
	st.strSound=vParams[0].getString();
	st.fFadeTime=(float)vParams[2].getFloat();
	st.task=SScheduledTask::TASK_TYPE_FADE_IN;
	for(std::list<SScheduledTask>::iterator it=GetInstance().m_lTasks.begin(); it!=GetInstance().m_lTasks.end(); it++)
	{
		if(it->fTime>st.fTime)
		{
			GetInstance().m_lTasks.insert(it, st);
			return std::vector<SGTScriptParam>();
		}
	}
	GetInstance().m_lTasks.push_back(st);
	return std::vector<SGTScriptParam>();
}
std::vector<SGTScriptParam>
SGTMusicSystem::Lua_FadeOut(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> errout(1, SGTScriptParam());
	std::vector<SGTScriptParam> vRef=std::vector<SGTScriptParam>(1, SGTScriptParam(std::string()));
	vRef.push_back(SGTScriptParam(0.1));
	vRef.push_back(SGTScriptParam(0.1));
	std::string strErrString=SGTUtils::TestParameters(vParams, vRef);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	if((OgreOggSound::OgreOggSoundManager::getSingleton().getSound(vParams[0].getString()))==NULL)
	{
		errout.push_back(std::string("found no sound for the given ID"));
		return errout;
	}
	SScheduledTask st;
	st.fTime=(float)vParams[1].getFloat();
	st.strSound=vParams[0].getString();
	st.fFadeTime=(float)vParams[2].getFloat();
	st.task=SScheduledTask::TASK_TYPE_FADE_OUT;
	for(std::list<SScheduledTask>::iterator it=GetInstance().m_lTasks.begin(); it!=GetInstance().m_lTasks.end(); it++)
	{
		if(it->fTime>st.fTime)
		{
			GetInstance().m_lTasks.insert(it, st);
			return std::vector<SGTScriptParam>();
		}
	}
	GetInstance().m_lTasks.push_back(st);
	return std::vector<SGTScriptParam>();
}
std::vector<SGTScriptParam>
SGTMusicSystem::Lua_SetBPM(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> errout(1, SGTScriptParam());
	std::string strErrString=SGTUtils::TestParameters(vParams, std::vector<SGTScriptParam>(1, SGTScriptParam(0.1)));
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	GetInstance().m_fBPM=vParams[0].getFloat();
	return std::vector<SGTScriptParam>();
}
std::vector<SGTScriptParam>
SGTMusicSystem::Lua_SetLooping(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> errout(1, SGTScriptParam());
	std::vector<SGTScriptParam> vRef=std::vector<SGTScriptParam>(1, SGTScriptParam(std::string()));
	vRef.push_back(SGTScriptParam(true));
	std::string strErrString=SGTUtils::TestParameters(vParams, vRef);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	OgreOggSound::OgreOggISound* pSound;
	if((pSound=OgreOggSound::OgreOggSoundManager::getSingleton().getSound(vParams[0].getString()))==NULL)
	{
		errout.push_back(std::string("found no sound for the given ID"));
		return errout;
	}
	pSound->loop(vParams[1].getBool());
	return std::vector<SGTScriptParam>();
}
std::vector<SGTScriptParam>
SGTMusicSystem::Lua_GetLooping(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> errout(1, SGTScriptParam());
	std::vector<SGTScriptParam> vRef=std::vector<SGTScriptParam>(1, SGTScriptParam(std::string()));
	std::string strErrString=SGTUtils::TestParameters(vParams, vRef);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	OgreOggSound::OgreOggISound* pSound;
	if((pSound=OgreOggSound::OgreOggSoundManager::getSingleton().getSound(vParams[0].getString()))==NULL)
	{
		errout.push_back(std::string("found no sound for the given ID"));
		return errout;
	}
	errout.push_back(std::string("not implemented!!"));
	return errout;
	return std::vector<SGTScriptParam>();
}
std::vector<SGTScriptParam>
SGTMusicSystem::Lua_GetCursor(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> errout(1, SGTScriptParam());
	std::vector<SGTScriptParam> vRef=std::vector<SGTScriptParam>(1, SGTScriptParam(std::string()));
	std::string strErrString=SGTUtils::TestParameters(vParams, vRef);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	OgreOggSound::OgreOggISound* pSound;
	if((pSound=OgreOggSound::OgreOggSoundManager::getSingleton().getSound(vParams[0].getString()))==NULL)
	{
		errout.push_back(std::string("found no sound for the given ID"));
		return errout;
	}
	errout.push_back(std::string("not implemented!!"));
	return errout;
	return std::vector<SGTScriptParam>();
}
std::vector<SGTScriptParam>
SGTMusicSystem::Lua_SetEventCallback(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> errout(1, SGTScriptParam());
	std::string strErrString=SGTUtils::TestParameters(vParams, std::vector<SGTScriptParam>(1, SGTScriptParam("", caller)));
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	GetInstance().m_EventCallback=vParams[0];
	return std::vector<SGTScriptParam>();
}
std::vector<SGTScriptParam>
SGTMusicSystem::Lua_SetTimer(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> errout(1, SGTScriptParam());
	std::vector<SGTScriptParam> vRef=std::vector<SGTScriptParam>(1, SGTScriptParam(0.1));
	vRef.push_back(SGTScriptParam(std::string(), caller));
	std::string strErrString=SGTUtils::TestParameters(vParams, vRef);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	SScheduledTask st;
	st.fTime=(float)vParams[0].getFloat()/GetInstance().m_fBPM*60.0f;
	st.task=SScheduledTask::TASK_TYPE_CALL_TIMER;
	st.callback=vParams[1];
	for(std::list<SScheduledTask>::iterator it=GetInstance().m_lTasks.begin(); it!=GetInstance().m_lTasks.end(); it++)
	{
		if(it->fTime>st.fTime)
		{
			GetInstance().m_lTasks.insert(it, st);
			return std::vector<SGTScriptParam>();
		}
	}
	GetInstance().m_lTasks.push_back(st);

	return std::vector<SGTScriptParam>();
}
std::vector<SGTScriptParam>
SGTMusicSystem::Lua_DeleteTimer(SGTScript& caller, std::vector<SGTScriptParam> vParams){return std::vector<SGTScriptParam>();}
std::vector<SGTScriptParam>
SGTMusicSystem::Lua_GetBeatCounter(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	return std::vector<SGTScriptParam>(1, SGTScriptParam(GetInstance().m_fCurrTime*GetInstance().m_fBPM/60.0f));
}

std::vector<SGTScriptParam>
SGTMusicSystem::Lua_GetMood(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> errout(1, SGTScriptParam());
	std::string strErrString=SGTUtils::TestParameters(vParams, std::vector<SGTScriptParam>(1, SGTScriptParam(std::string())));
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	return std::vector<SGTScriptParam>(1, SGTScriptParam(GetInstance().GetMood(vParams[0].getString())));
}

std::vector<SGTScriptParam>
SGTMusicSystem::Lua_SetMood(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> errout(1, SGTScriptParam());
	std::vector<SGTScriptParam> vRef=std::vector<SGTScriptParam>(1, SGTScriptParam(std::string()));
	vRef.push_back(SGTScriptParam(true));
	std::string strErrString=SGTUtils::TestParameters(vParams, vRef);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	GetInstance().SetMood(vParams[0].getString(), vParams[1].getBool());
	return std::vector<SGTScriptParam>();
}

void
SGTMusicSystem::Clear()
{
	m_lTasks.clear();
	m_mMoods.clear();
}