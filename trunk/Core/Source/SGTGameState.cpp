
#include "SGTGameState.h"
#include "SGTMessageSystem.h"
#include "SGTMain.h"
#include "SGTSceneManager.h"
#include "SGTInput.h"
#include "SGTMainLoop.h"
#include "NxOgre.h"
#include "OgreOggSound.h"

bool SGTEditor::OnUpdate(float time, float time_total)
{
	SGTMsg msg;
	msg.mNewsgroup = "UPDATE_PER_FRAME";
	msg.mData.AddFloat("TIME", time);
	msg.mData.AddFloat("TIME_TOTAL", time_total);
	SGTMessageSystem::Instance().SendMessage(msg);

	SGTMain::Instance().GetInputManager()->Update();

	SGTMessageSystem::Instance().Update();

	if (SGTMainLoop::Instance().GetRunPhysics())
	{
		SGTMain::Instance().GetNxWorld()->getPhysXDriver()->simulate(time);
	}
	SGTMain::Instance().GetNxWorld()->render(time);

	//Sound
	SGTMain::Instance().GetSoundManager()->update();

	SGTSceneManager::Instance().UpdateGameObjects();

	return Ogre::Root::getSingleton().renderOneFrame();
}

bool SGTGame::OnUpdate(float time, float time_total)
{
	SGTMsg msg;
	msg.mNewsgroup = "UPDATE_PER_FRAME";
	msg.mData.AddFloat("TIME", time);
	msg.mData.AddFloat("TIME_TOTAL", time_total);
	SGTMessageSystem::Instance().SendMessage(msg);

	SGTMain::Instance().GetInputManager()->Update();

	//float BeforeMsgTime = timeGetTime();
	SGTMessageSystem::Instance().Update();
	//float AfterMsgTime = timeGetTime() - BeforeMsgTime;
	//Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(AfterMsgTime));

	SGTMain::Instance().GetNxWorld()->getPhysXDriver()->simulate(time);
	SGTMain::Instance().GetNxWorld()->render(time);

	//Sound
	SGTMain::Instance().GetSoundManager()->update();

	SGTSceneManager::Instance().UpdateGameObjects();

	return Ogre::Root::getSingleton().renderOneFrame();
}

void SGTGame::OnEnter()
{
	if (!mInitialized)
	{
		SGTScript script = SGTScriptSystem::GetInstance().CreateInstance("NewGame.lua");
		std::vector<SGTScriptParam> params;
		script.CallFunction("NewGame", params);
	}
	mInitialized = true;
}

bool SGTDefaultMenu::OnUpdate(float time, float time_total)
{
	return Ogre::Root::getSingleton().renderOneFrame();
}

void SGTDefaultMenu::OnEnter()
{
}