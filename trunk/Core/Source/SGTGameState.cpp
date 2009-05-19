
#include "SGTGameState.h"
#include "OgrePlugin.h"
#include "OgreDynLibManager.h"
#include "OgreDynLib.h"
#include "SGTMessageSystem.h"
#include "SGTMain.h"
#include "SGTSceneManager.h"


typedef void (*DLL_START_PLUGIN)(void);
typedef void (*DLL_STOP_PLUGIN)(void);

void SGTEditor::OnUpdate(float time, float time_total)
{
	SGTMsg msg;
	msg.mNewsgroup = "UPDATE_PER_FRAME";
	msg.mData.AddFloat("TIME", time);
	msg.mData.AddFloat("TIME_TOTAL", time_total);
	SGTMessageSystem::Instance().SendMessage(msg);

	SGTMessageSystem::Instance().Update();

	if (SGTKernel::Instance().GetRunPhysics())
	{
		SGTMain::Instance().GetNxWorld()->getPhysXDriver()->simulate(time);
	}
	SGTMain::Instance().GetNxWorld()->render(time);

	//Sound
	SGTMain::Instance().GetSoundManager()->update();

	SGTSceneManager::Instance().UpdateGameObjects();

	Ogre::Root::getSingleton().renderOneFrame();
}

void SGTGame::OnUpdate(float time, float time_total)
{
	SGTMsg msg;
	msg.mNewsgroup = "UPDATE_PER_FRAME";
	msg.mData.AddFloat("TIME", time);
	msg.mData.AddFloat("TIME_TOTAL", time_total);
	SGTMessageSystem::Instance().SendMessage(msg);

	//float BeforeMsgTime = timeGetTime();
	SGTMessageSystem::Instance().Update();
	//float AfterMsgTime = timeGetTime() - BeforeMsgTime;
	//Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(AfterMsgTime));

	SGTMain::Instance().GetNxWorld()->getPhysXDriver()->simulate(time);
	SGTMain::Instance().GetNxWorld()->render(time);

	//Sound
	SGTMain::Instance().GetSoundManager()->update();

	SGTSceneManager::Instance().UpdateGameObjects();

	Ogre::Root::getSingleton().renderOneFrame();
}

void SGTGame::OnEnter()
{
	if (!mInitialized)
	{
		SGTScript script = SGTScriptSystem::GetInstance().CreateInstance(SCRIPT_BASE_DIR + "NewGame.lua");
		std::vector<SGTScriptParam> params;
		script.CallFunction("NewGame", params);
		SGTSceneManager::Instance().CreatePlayer();
	}
	mInitialized = true;
}

void SGTDefaultMenu::OnUpdate(float time, float time_total)
{
	Ogre::Root::getSingleton().renderOneFrame();
}

void SGTDefaultMenu::OnEnter()
{
}

SGTKernel::SGTKernel()
{
	mTimeSinceLastFrame = 0.0f;
	mTotalTimeElapsed = 0.0f;
	mTotalLastFrameTime = timeGetTime();
	mRunning = true;
	mPaused = false;
	mRunPhysics = true;
	AddState((SGTGameState*)new SGTGame());
	AddState((SGTGameState*)new SGTDefaultMenu());
	AddState((SGTGameState*)new SGTEditor());
};

void SGTKernel::AddState(SGTGameState* state)
{
	mStates.push_back(state);
}

void SGTKernel::SetState(Ogre::String name)
{
	for (std::vector<SGTGameState*>::iterator i = mStates.begin(); i != mStates.end(); i++)
	{
		if ((*i)->GetName() == name)
		{
			mCurrentState = (*i);
			mCurrentState->OnEnter();
			return;
		}
	}
}


void SGTKernel::startLoop()
{
	mPaused = false;
	mTimeSinceLastFrame = 0.0f;
	mTotalTimeElapsed = 0.0f;
	mTotalLastFrameTime = timeGetTime();
	while (doLoop())
	{
		Ogre::WindowEventUtilities::messagePump();
	}
};

bool SGTKernel::doLoop()
{
	if (!mPaused)
	{
		float difference = timeGetTime() - mTotalLastFrameTime;
		mTimeSinceLastFrame = difference * 0.001f;
		mTotalLastFrameTime = timeGetTime();
		mTotalTimeElapsed += mTimeSinceLastFrame;

		mCurrentState->OnUpdate(mTimeSinceLastFrame, mTotalLastFrameTime);
	}
	return true;
};

void SGTKernel::quitLoop()
{
	mRunning = false;
};

void SGTKernel::pauseLoop()
{
	mPaused = true;
};

void SGTKernel::SetPhysics(bool enable)
{
	mRunPhysics = enable;
}

void SGTKernel::continueLoop()
{
	mPaused = false;
};

void SGTKernel::InstallPlugin(Ogre::Plugin* plugin)
{
	Ogre::LogManager::getSingleton().logMessage("Installing SGT plugin: " + plugin->getName());

	mPlugins.push_back(plugin);
	plugin->install();

	plugin->initialise();

	Ogre::LogManager::getSingleton().logMessage("SGT Plugin successfully installed");
}
//---------------------------------------------------------------------
void SGTKernel::UninstallPlugin(Ogre::Plugin* plugin)
{
	Ogre::LogManager::getSingleton().logMessage("Uninstalling SGT plugin: " + plugin->getName());
	std::vector<Ogre::Plugin*>::iterator i = std::find(mPlugins.begin(), mPlugins.end(), plugin);
	if (i != mPlugins.end())
	{
		plugin->uninstall();
		mPlugins.erase(i);
	}

	Ogre::LogManager::getSingleton().logMessage("SGT Plugin successfully uninstalled");

}
//-----------------------------------------------------------------------
void SGTKernel::LoadPlugin(const Ogre::String& pluginName)
{
	// Load plugin library
    Ogre::DynLib* lib = Ogre::DynLibManager::getSingleton().load( pluginName );
	// Store for later unload
	mPluginLibs.push_back(lib);

	// Call startup function
	DLL_START_PLUGIN pFunc = (DLL_START_PLUGIN)lib->getSymbol("dllStartPlugin");

	if (!pFunc)
		Ogre::LogManager::getSingleton().logMessage("Cannot find symbol dllStartPlugin in library " + pluginName);

	// This must call installPlugin
	pFunc();

}
//-----------------------------------------------------------------------
void SGTKernel::UnloadPlugin(const Ogre::String& pluginName)
{
	std::vector<Ogre::DynLib*>::iterator i;

    for (i = mPluginLibs.begin(); i != mPluginLibs.end(); ++i)
	{
		if ((*i)->getName() == pluginName)
		{
			// Call plugin shutdown
			DLL_STOP_PLUGIN pFunc = (DLL_STOP_PLUGIN)(*i)->getSymbol("dllStopPlugin");
			// this must call uninstallPlugin
			pFunc();
			// Unload library (destroyed by DynLibManager)
			Ogre::DynLibManager::getSingleton().unload(*i);
			mPluginLibs.erase(i);
			return;
		}

	}
}

void SGTKernel::ClearPlugins()
{
	for (std::vector<Ogre::Plugin*>::iterator i = mPlugins.begin(); i != mPlugins.end(); i++)
	{
		(*i)->shutdown();
	}
	mPlugins.clear();

	for (std::vector<Ogre::DynLib*>::iterator i = mPluginLibs.begin(); i != mPluginLibs.end(); i++)
	{
		Ogre::DynLibManager::getSingleton().unload(*i);
	}
	mPluginLibs.clear();
}


SGTKernel& SGTKernel::Instance()
{
	static SGTKernel TheOneAndOnly;
	return TheOneAndOnly;
};