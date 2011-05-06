
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
	//time = 1/60.0f;
	Msg msg;
	msg.params.AddFloat("TIME", time);
	msg.params.AddFloat("TIME_TOTAL", time_total);

	//Input
	Main::Instance().GetInputManager()->Update();
	//Sound
	Main::Instance().GetSoundManager()->update();

	//Game stuff
	msg.type = GlobalMessageIDs::UPDATE_PER_FRAME;
	MessageSystem::SendInstantMessage(msg);

	//Process all messages
	MessageSystem::Update();

	SceneManager::Instance().UpdateGameObjects();

	if (MainLoop::Instance().GetRunPhysics())
	{
		OgrePhysX::World::getSingleton().startSimulate(time);
		OgrePhysX::World::getSingleton().syncRenderables();
	}

	msg.type = "START_RENDERING";
	MessageSystem::SendInstantMessage(msg);
	bool render = Ogre::Root::getSingleton().renderOneFrame();
	msg.type = "END_RENDERING";
	MessageSystem::SendInstantMessage(msg);

	return render;
}

bool Game::OnUpdate(float time, float time_total)
{
	Msg msg;
	msg.params.AddFloat("TIME", time);
	msg.params.AddFloat("TIME_TOTAL", time_total);

	//Input
	Main::Instance().GetInputManager()->Update();
	//Sound
	Main::Instance().GetSoundManager()->update();

	//Game stuff
	msg.type = GlobalMessageIDs::UPDATE_PER_FRAME;
	MessageSystem::SendInstantMessage(msg);

	//Process all messages
	MessageSystem::Update();

	SceneManager::Instance().UpdateGameObjects();

	if (MainLoop::Instance().GetRunPhysics())
	{
		OgrePhysX::World::getSingleton().startSimulate(time);
		OgrePhysX::World::getSingleton().syncRenderables();
	}

	msg.type = "START_RENDERING";
	MessageSystem::SendInstantMessage(msg);
	Ogre::WindowEventUtilities::messagePump();
	bool render = Ogre::Root::getSingleton().renderOneFrame();
	msg.type = "END_RENDERING";
	MessageSystem::SendInstantMessage(msg);

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