
#include "IceGameState.h"
#include "IceMessageSystem.h"
#include "IceMain.h"
#include "IceSceneManager.h"
#include "IceInput.h"
#include "IceMainLoop.h"
#include "OgrePhysX.h"
#include "OgreOggSound.h"

namespace Ice
{

bool Editor::OnUpdate(float time, float time_total)
{
	Msg msg;
	msg.params.AddFloat("TIME", time);
	msg.params.AddFloat("TIME_TOTAL", time_total);

	if (MainLoop::Instance().GetRunPhysics())
	{
		msg.type = "START_PHYSICS";
		MessageSystem::Instance().SendInstantMessage(msg);
		OgrePhysX::World::getSingleton().startSimulate(time);		//non-blocking
	}

	//Input
	Main::Instance().GetInputManager()->Update();
	//Sound
	Main::Instance().GetSoundManager()->update();

	if (MainLoop::Instance().GetRunPhysics())
	{
		OgrePhysX::World::getSingleton().fetchSimulate();	//blocking
		OgrePhysX::World::getSingleton().syncRenderables();
		msg.type = "END_PHYSICS";
		MessageSystem::Instance().SendInstantMessage(msg);
	}

	//Game stuff
	msg.type = "UPDATE_PER_FRAME";
	MessageSystem::Instance().SendInstantMessage(msg);

	//Process all messages
	MessageSystem::Instance().Update();

	SceneManager::Instance().UpdateGameObjects();

	msg.type = "START_RENDERING";
	MessageSystem::Instance().SendInstantMessage(msg);
	bool render = Ogre::Root::getSingleton().renderOneFrame();
	msg.type = "END_RENDERING";
	MessageSystem::Instance().SendInstantMessage(msg);

	return render;
}

bool Game::OnUpdate(float time, float time_total)
{
	Msg msg;
	msg.type = "START_PHYSICS";
	msg.params.AddFloat("TIME", time);
	msg.params.AddFloat("TIME_TOTAL", time_total);
	MessageSystem::Instance().SendInstantMessage(msg);

	if (MainLoop::Instance().GetRunPhysics())
		OgrePhysX::World::getSingleton().startSimulate(time);		//non-blocking

	//Input
	Main::Instance().GetInputManager()->Update();
	//Sound
	Main::Instance().GetSoundManager()->update();

	//Game stuff
	msg.type = "UPDATE_PER_FRAME";
	MessageSystem::Instance().SendInstantMessage(msg);

	if (MainLoop::Instance().GetRunPhysics())
	{
		OgrePhysX::World::getSingleton().fetchSimulate();	//blocking
		OgrePhysX::World::getSingleton().syncRenderables();
		msg.type = "END_PHYSICS";
		MessageSystem::Instance().SendInstantMessage(msg);
	}

	//Process all messages
	MessageSystem::Instance().Update();

	SceneManager::Instance().UpdateGameObjects();

	msg.type = "START_RENDERING";
	MessageSystem::Instance().SendInstantMessage(msg);
	bool render = Ogre::Root::getSingleton().renderOneFrame();
	msg.type = "END_RENDERING";
	MessageSystem::Instance().SendInstantMessage(msg);

	return render;
}

void Game::OnEnter()
{
	if (!mInitialized)
	{
		Script script = ScriptSystem::GetInstance().CreateInstance("NewGame.lua");
		std::vector<ScriptParam> params;
		script.CallFunction("NewGame", params);
	}
	mInitialized = true;
}

bool DefaultMenu::OnUpdate(float time, float time_total)
{
	return Ogre::Root::getSingleton().renderOneFrame();
}

void DefaultMenu::OnEnter()
{
}

};