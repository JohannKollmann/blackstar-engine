
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
	if (SGTMainLoop::Instance().GetRunPhysics())
		OgrePhysX::World::getSingleton().startSimulate(time);

	if (SGTMainLoop::Instance().GetRunPhysics())
		OgrePhysX::World::getSingleton().fetchSimulate();
	OgrePhysX::World::getSingleton().syncRenderables();

	SGTMsg msg;
	msg.mNewsgroup = "UPDATE_PER_FRAME";
	msg.mData.AddFloat("TIME", time);
	msg.mData.AddFloat("TIME_TOTAL", time_total);
	SGTMessageSystem::Instance().SendMessage(msg);

	SGTMain::Instance().GetInputManager()->Update();

	SGTMessageSystem::Instance().Update();

	//Sound
	SGTMain::Instance().GetSoundManager()->update();

	SGTSceneManager::Instance().UpdateGameObjects();

	bool render = Ogre::Root::getSingleton().renderOneFrame();

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