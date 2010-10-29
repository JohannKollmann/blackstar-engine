#include "MusicSystem.h"
#include "IceSceneManager.h"
#include "IceUtils.h"

MusicSystem::MusicSystem()
{
	Ice::ScriptSystem::GetInstance().ShareCFunction("music_play", Lua_PlaySound);
	Ice::ScriptSystem::GetInstance().ShareCFunction("music_create_sound", Lua_CreateSound);
	Ice::ScriptSystem::GetInstance().ShareCFunction("music_delete_sound", Lua_DeleteSound);
	Ice::ScriptSystem::GetInstance().ShareCFunction("music_set_volume", Lua_SetVolume);
	Ice::ScriptSystem::GetInstance().ShareCFunction("music_play_at", Lua_PlayAt);
	Ice::ScriptSystem::GetInstance().ShareCFunction("music_stop_at", Lua_StopAt);
	Ice::ScriptSystem::GetInstance().ShareCFunction("music_fade_in", Lua_FadeIn);
	Ice::ScriptSystem::GetInstance().ShareCFunction("music_fade_out", Lua_FadeOut);
	Ice::ScriptSystem::GetInstance().ShareCFunction("music_set_bpm", Lua_SetBPM);
	Ice::ScriptSystem::GetInstance().ShareCFunction("music_set_looping", Lua_SetLooping);
	Ice::ScriptSystem::GetInstance().ShareCFunction("music_get_looping", Lua_GetLooping);
	Ice::ScriptSystem::GetInstance().ShareCFunction("music_get_cursor", Lua_GetCursor);
	Ice::ScriptSystem::GetInstance().ShareCFunction("music_set_event_callback", Lua_SetEventCallback);
	Ice::ScriptSystem::GetInstance().ShareCFunction("music_set_timer", Lua_SetTimer);
	Ice::ScriptSystem::GetInstance().ShareCFunction("music_delete_timer", Lua_DeleteTimer);
	Ice::ScriptSystem::GetInstance().ShareCFunction("music_get_beat_counter", Lua_GetBeatCounter);
	Ice::ScriptSystem::GetInstance().ShareCFunction("music_get_mood", Lua_GetMood);
	Ice::ScriptSystem::GetInstance().ShareCFunction("music_set_mood", Lua_SetMood);
	Ice::ScriptSystem::GetInstance().ShareCFunction("music_post_event", Lua_PostEvent);

	Ice::MessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
}

MusicSystem&
MusicSystem::GetInstance()
{
	static MusicSystem singleton;
	return singleton;
}

void
MusicSystem::ReceiveMessage(Ice::Msg &msg)
{
	if (msg.type == "UPDATE_PER_FRAME")
	{
		float time = msg.params.GetFloat("TIME_TOTAL");
		m_fCurrTime=time;
		if(m_lTasks.size())
			for(std::list<SScheduledTask>::iterator it=m_lTasks.begin(); it->fTime<=m_fCurrTime; )
			{
				OgreOggSound::OgreOggISound* pSound=OgreOggSound::OgreOggSoundManager::getSingleton().getSound(it->strSound);
				switch(it->task)
				{
				case SScheduledTask::TASK_TYPE_FADE_IN:
					pSound->startFade(true, it->fFadeTime);
					break;
				case SScheduledTask::TASK_TYPE_FADE_OUT:
					pSound->startFade(false, it->fFadeTime);
					break;
				case SScheduledTask::TASK_TYPE_PLAY:
					pSound->stop();
					//delete potential loops associated with this sound
					pSound->play();
					if(GetInstance().m_mProperties[it->strSound].bLooping)
					{
						GetInstance().m_mProperties[it->strSound].fLoopTime;
						SScheduledTask st;
						st.fTime=it->fTime + GetInstance().m_mProperties[it->strSound].fLoopTime;
						st.strSound=it->strSound;
						st.task=SScheduledTask::TASK_TYPE_PLAY;
						st.bIsLoop=true;
						InsertEvent(st);
					}
					break;
				case SScheduledTask::TASK_TYPE_STOP:
					pSound->stop();
					//search for the looping indicator in case the sound is looping
					if(GetInstance().m_mProperties[it->strSound].bLooping)
					{
						for(std::list<SScheduledTask>::iterator itSearch=m_lTasks.begin(); itSearch!=m_lTasks.end(); itSearch++)
						{
							if(itSearch->bIsLoop && itSearch->strSound==it->strSound && itSearch->task==SScheduledTask::TASK_TYPE_PLAY)
							{
								m_lTasks.erase(itSearch);
								break;
							}
						}
					}
					break;
				case SScheduledTask::TASK_TYPE_CALL_TIMER:
				{
					Ice::ScriptSystem::RunCallbackFunction(it->callback, std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(m_fCurrTime*m_fBPM/60.0f)));
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
				if(m_lTasks.empty())
					break;
				it=m_lTasks.begin();
			}
	}
}

void
MusicSystem::SetMood(std::string strMood, bool bActive)
{
	m_mMoods[strMood]=bActive;
}

bool
MusicSystem::GetMood(std::string strMood)
{
	return m_mMoods[strMood];
}

void
MusicSystem::PostEvent(std::string strEvent)
{
	Ice::ScriptSystem::GetInstance().RunCallbackFunction(m_EventCallback, std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(strEvent)));
}

std::vector<Ice::ScriptParam>
MusicSystem::Lua_CreateSound(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout(1, Ice::ScriptParam());
	std::vector<Ice::ScriptParam> vRef=std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(std::string()));
	std::string strErrString=Ice::Utils::TestParameters(vParams, vRef);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}

	Ogre::String strID=Ice::SceneManager::Instance().RequestIDStr();
	OgreOggSound::OgreOggSoundManager::getSingleton().createSound(strID, vParams[0].getString());

	SSoundProperty sp;
	sp.bLooping=false;
	sp.fLoopTime=0.0f;
	GetInstance().m_mProperties[strID]=sp;

	return std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(strID));
}


std::vector<Ice::ScriptParam>
MusicSystem::Lua_DeleteSound(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout(1, Ice::ScriptParam());
	std::vector<Ice::ScriptParam> vRef=std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(std::string()));
	std::string strErrString=Ice::Utils::TestParameters(vParams, vRef);
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
	OgreOggSound::OgreOggSoundManager::getSingleton().destroySound(pSound);
	GetInstance().m_mProperties.erase(GetInstance().m_mProperties.find(vParams[0].getString()));
	//Ogre::LogManager::getSingleton().logMessage("Successfully deleted sound object " + vParams[0].getString()  + " !");
	return std::vector<Ice::ScriptParam>();
}

std::vector<Ice::ScriptParam>
MusicSystem::Lua_PlaySound(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout(1, Ice::ScriptParam());
	std::vector<Ice::ScriptParam> vRef=std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(std::string()));
	std::string strErrString=Ice::Utils::TestParameters(vParams, vRef);
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
	return std::vector<Ice::ScriptParam>();
}

std::vector<Ice::ScriptParam>
MusicSystem::Lua_SetVolume(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout(1, Ice::ScriptParam());
	std::vector<Ice::ScriptParam> vRef=std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(std::string()));
	vRef.push_back(Ice::ScriptParam(1.0));
	std::string strErrString=Ice::Utils::TestParameters(vParams, vRef);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	/*errout.push_back(std::string("not implemented!!"));
	return errout;*/
	OgreOggSound::OgreOggISound* pSound;
	if((pSound=OgreOggSound::OgreOggSoundManager::getSingleton().getSound(vParams[0].getString()))==NULL)
	{
		errout.push_back(std::string("found no sound for the given ID"));
		return errout;
	}
	pSound->setMaxVolume((float)vParams[1].getFloat());
	
	return std::vector<Ice::ScriptParam>();
}

std::vector<Ice::ScriptParam>
MusicSystem::Lua_PlayAt(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout(1, Ice::ScriptParam());
	std::vector<Ice::ScriptParam> vRef=std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(std::string()));
	vRef.push_back(Ice::ScriptParam(0.1));
	std::string strErrString=Ice::Utils::TestParameters(vParams, vRef);
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
	st.fTime=(float)vParams[1].getFloat()*60.0f/GetInstance().m_fBPM;
	st.strSound=vParams[0].getString();
	st.task=SScheduledTask::TASK_TYPE_PLAY;
	st.bIsLoop=false;
	InsertEvent(st);
	return std::vector<Ice::ScriptParam>();
}
std::vector<Ice::ScriptParam>
MusicSystem::Lua_StopAt(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout(1, Ice::ScriptParam());
	std::vector<Ice::ScriptParam> vRef=std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(std::string()));
	vRef.push_back(Ice::ScriptParam(0.1));
	std::string strErrString=Ice::Utils::TestParameters(vParams, vRef);
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
	st.fTime=(float)vParams[1].getFloat()*60.0f/GetInstance().m_fBPM;
	st.strSound=vParams[0].getString();
	st.task=SScheduledTask::TASK_TYPE_STOP;
	InsertEvent(st);
	return std::vector<Ice::ScriptParam>();
}
std::vector<Ice::ScriptParam>
MusicSystem::Lua_FadeIn(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout(1, Ice::ScriptParam());
	std::vector<Ice::ScriptParam> vRef=std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(std::string()));
	vRef.push_back(Ice::ScriptParam(0.1));
	vRef.push_back(Ice::ScriptParam(0.1));
	std::string strErrString=Ice::Utils::TestParameters(vParams, vRef);
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
	st.fTime=(float)vParams[1].getFloat()*60.0f/GetInstance().m_fBPM;
	st.strSound=vParams[0].getString();
	st.fFadeTime=(float)vParams[2].getFloat()*60.0f/GetInstance().m_fBPM;
	st.task=SScheduledTask::TASK_TYPE_FADE_IN;
	InsertEvent(st);
	return std::vector<Ice::ScriptParam>();
}
std::vector<Ice::ScriptParam>
MusicSystem::Lua_FadeOut(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout(1, Ice::ScriptParam());
	std::vector<Ice::ScriptParam> vRef=std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(std::string()));
	vRef.push_back(Ice::ScriptParam(0.1));
	vRef.push_back(Ice::ScriptParam(0.1));
	std::string strErrString=Ice::Utils::TestParameters(vParams, vRef);
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
	st.fTime=(float)vParams[1].getFloat()*60.0f/GetInstance().m_fBPM;
	st.strSound=vParams[0].getString();
	st.fFadeTime=(float)vParams[2].getFloat()*60.0f/GetInstance().m_fBPM;
	st.task=SScheduledTask::TASK_TYPE_FADE_OUT;
	InsertEvent(st);
	return std::vector<Ice::ScriptParam>();
}
std::vector<Ice::ScriptParam>
MusicSystem::Lua_SetBPM(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout(1, Ice::ScriptParam());
	std::string strErrString=Ice::Utils::TestParameters(vParams, std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(0.1)));
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	GetInstance().m_fBPM=vParams[0].getFloat();
	return std::vector<Ice::ScriptParam>();
}
std::vector<Ice::ScriptParam>
MusicSystem::Lua_SetLooping(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout(1, Ice::ScriptParam());
	std::vector<Ice::ScriptParam> vRef=std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(std::string()));
	vRef.push_back(Ice::ScriptParam(true));
	vRef.push_back(Ice::ScriptParam(0.0));
	std::string strErrString=Ice::Utils::TestParameters(vParams, vRef);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	/*OgreOggSound::OgreOggISound* pSound;
	if((pSound=OgreOggSound::OgreOggSoundManager::getSingleton().getSound(vParams[0].getString()))==NULL)
	{
		errout.push_back(std::string("found no sound for the given ID"));
		return errout;
	}
	pSound->loop(vParams[1].getBool());*/
	GetInstance().m_mProperties[vParams[0].getString()].bLooping=vParams[1].getBool();
	GetInstance().m_mProperties[vParams[0].getString()].fLoopTime=(float)vParams[2].getFloat()*60.0f/GetInstance().m_fBPM;
	return std::vector<Ice::ScriptParam>();
}
std::vector<Ice::ScriptParam>
MusicSystem::Lua_GetLooping(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout(1, Ice::ScriptParam());
	std::vector<Ice::ScriptParam> vRef=std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(std::string()));
	std::string strErrString=Ice::Utils::TestParameters(vParams, vRef);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	/*OgreOggSound::OgreOggISound* pSound;
	if((pSound=OgreOggSound::OgreOggSoundManager::getSingleton().getSound(vParams[0].getString()))==NULL)
	{
		errout.push_back(std::string("found no sound for the given ID"));
		return errout;
	}
	errout.push_back(std::string("not implemented!!"));*/
	return std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(GetInstance().m_mProperties[vParams[0].getString()].bLooping));
}

std::vector<Ice::ScriptParam>
MusicSystem::Lua_GetCursor(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout(1, Ice::ScriptParam());
	std::vector<Ice::ScriptParam> vRef=std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(std::string()));
	std::string strErrString=Ice::Utils::TestParameters(vParams, vRef);
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
	return std::vector<Ice::ScriptParam>();
}
std::vector<Ice::ScriptParam>
MusicSystem::Lua_SetEventCallback(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout(1, Ice::ScriptParam());
	std::string strErrString=Ice::Utils::TestParameters(vParams, std::vector<Ice::ScriptParam>(1, Ice::ScriptParam("", caller)));
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	GetInstance().m_EventCallback=vParams[0];
	return std::vector<Ice::ScriptParam>();
}
std::vector<Ice::ScriptParam>
MusicSystem::Lua_SetTimer(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout(1, Ice::ScriptParam());
	std::vector<Ice::ScriptParam> vRef=std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(0.1));
	vRef.push_back(Ice::ScriptParam(std::string(), caller));
	std::string strErrString=Ice::Utils::TestParameters(vParams, vRef);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	SScheduledTask st;
	st.fTime=(float)vParams[0].getFloat()/GetInstance().m_fBPM*60.0f;
	st.task=SScheduledTask::TASK_TYPE_CALL_TIMER;
	st.callback=vParams[1];
	InsertEvent(st);

	return std::vector<Ice::ScriptParam>();
}
std::vector<Ice::ScriptParam>
MusicSystem::Lua_DeleteTimer(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams){return std::vector<Ice::ScriptParam>();}
std::vector<Ice::ScriptParam>
MusicSystem::Lua_GetBeatCounter(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	return std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(GetInstance().m_fCurrTime*GetInstance().m_fBPM/60.0f));
}

std::vector<Ice::ScriptParam>
MusicSystem::Lua_GetMood(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout(1, Ice::ScriptParam());
	std::string strErrString=Ice::Utils::TestParameters(vParams, std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(std::string())));
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	return std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(GetInstance().GetMood(vParams[0].getString())));
}

std::vector<Ice::ScriptParam>
MusicSystem::Lua_SetMood(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout(1, Ice::ScriptParam());
	std::vector<Ice::ScriptParam> vRef=std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(std::string()));
	vRef.push_back(Ice::ScriptParam(true));
	std::string strErrString=Ice::Utils::TestParameters(vParams, vRef);
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	GetInstance().SetMood(vParams[0].getString(), vParams[1].getBool());
	return std::vector<Ice::ScriptParam>();
}

std::vector<Ice::ScriptParam>
MusicSystem::Lua_PostEvent(Ice::Script& caller, std::vector<Ice::ScriptParam> vParams)
{
	std::vector<Ice::ScriptParam> errout(1, Ice::ScriptParam());
	std::string strErrString=Ice::Utils::TestParameters(vParams, std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(std::string())));
	if(strErrString.length())
	{
		errout.push_back(strErrString);
		return errout;
	}
	GetInstance().PostEvent(vParams[0].getString());
	return std::vector<Ice::ScriptParam>();
}

void
MusicSystem::Clear()
{
	m_lTasks.clear();
	m_mMoods.clear();
	//OgreOggSound::OgreOggSoundManager::getSingleton().destroyAllSounds();
	for(auto it=GetInstance().m_mProperties.begin(); it!=GetInstance().m_mProperties.end(); it++)
		Lua_DeleteSound(Ice::Script(), std::vector<Ice::ScriptParam>(1, Ice::ScriptParam(it->first)));
		
	m_mProperties.clear();
}

void
MusicSystem::InsertEvent(SScheduledTask st)
{
	for(std::list<SScheduledTask>::iterator it=GetInstance().m_lTasks.begin(); it!=GetInstance().m_lTasks.end(); it++)
	{
		if(it->fTime>st.fTime)
		{
			GetInstance().m_lTasks.insert(it, st);
			return;
		}
	}
	GetInstance().m_lTasks.push_back(st);
}