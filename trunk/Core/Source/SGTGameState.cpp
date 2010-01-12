
#include "SGTGameState.h"
#include "SGTMessageSystem.h"
#include "SGTMain.h"
#include "SGTSceneManager.h"
#include "SGTInput.h"
#include "SGTMainLoop.h"
#include "OgrePhysX.h"
#include "OgreOggSound.h"

bool SGTEditor::OnUpdate(float time, float time_total)
{
	SGTMsg msg;
	msg.mNewsgroup = "START_PHYSICS";
	msg.mData.AddFloat("TIME", time);
	msg.mData.AddFloat("TIME_TOTAL", time_total);
	SGTMessageSystem::Instance().SendInstantMessage(msg);

	if (SGTMainLoop::Instance().GetRunPhysics())
		OgrePhysX::World::getSingleton().startSimulate(time);		//non-blocking

	//Input
	SGTMain::Instance().GetInputManager()->Update();
	//Sound
	SGTMain::Instance().GetSoundManager()->update();

	//Game stuff
	msg.mNewsgroup = "UPDATE_PER_FRAME";
	SGTMessageSystem::Instance().SendInstantMessage(msg);

	if (SGTMainLoop::Instance().GetRunPhysics())
	{
		OgrePhysX::World::getSingleton().fetchSimulate();	//blocking
		OgrePhysX::World::getSingleton().syncRenderables();
		msg.mNewsgroup = "END_PHYSICS";
		SGTMessageSystem::Instance().SendInstantMessage(msg);
	}

	//Process all messages
	SGTMessageSystem::Instance().Update();

	SGTSceneManager::Instance().UpdateGameObjects();

	msg.mNewsgroup = "START_RENDERING";
	SGTMessageSystem::Instance().SendInstantMessage(msg);
	bool render = Ogre::Root::getSingleton().renderOneFrame();
	msg.mNewsgroup = "END_RENDERING";
	SGTMessageSystem::Instance().SendInstantMessage(msg);

	return render;
}

bool SGTGame::OnUpdate(float time, float time_total)
{
	SGTMsg msg;
	msg.mNewsgroup = "UPDATE_PER_FRAME";
	msg.mData.AddFloat("TIME", time);
	msg.mData.AddFloat("TIME_TOTAL", time_total);
	SGTMessageSystem::Instance().SendMessage(msg);

	SGTMain::Instance().GetInputManager()->Update();

	if (SGTMainLoop::Instance().GetRunPhysics())
		OgrePhysX::World::getSingleton().startSimulate(time);

	OgrePhysX::World::getSingleton().syncRenderables();

	//Sound
	SGTMain::Instance().GetSoundManager()->update();

	SGTSceneManager::Instance().UpdateGameObjects();

	bool render = Ogre::Root::getSingleton().renderOneFrame();

	if (SGTMainLoop::Instance().GetRunPhysics())
		OgrePhysX::World::getSingleton().fetchSimulate();

	return render;
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