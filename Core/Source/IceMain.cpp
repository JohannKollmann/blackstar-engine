
#include "OgrePhysX.h"
#include "OgreOggSound.h"

#include "IceMain.h"
#include "IceGameState.h"
#include "IceInput.h"
#include "IceMessageSystem.h"
#include "IceCameraController.h"
#include "IceConsole.h"
#include "IceSceneManager.h"
#include "IceSceneListener.h"
#include "HDRListener.h"
#include "IceWeatherController.h"
#include "IceCollisionCallback.h"

#include "OgrePlugin.h"
#include "OgreDynLibManager.h"
#include "OgreDynLib.h"

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"

#include "IceMainLoop.h"

#include "IceGOCPhysics.h"

#include "IceLeakWatch.h"

#define USE_REMOTEDEBUGGER 1

namespace Ice
{

//loader for ScriptSystem
std::string
OgreFileLoader(lua_State* pState, std::string strFile)
{
	if(!Ogre::ResourceGroupManager::getSingleton().resourceExists(Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, strFile))
		return std::string("file not found");
	Ogre::DataStreamPtr ds = Ogre::ResourceGroupManager::getSingleton().openResource(strFile);
	char *buffer = ICE_NEW char[ds->size()];
	ds->read(buffer, ds->size());
	ds->close();

	if(luaL_loadbuffer(pState, buffer, ds->size(), strFile.c_str()) || lua_pcall(pState, 0, 0, 0))
	{
		ICE_DELETE buffer;
		return std::string(lua_tostring(pState, -1));
	}
	ICE_DELETE buffer;
	return std::string();
}

void
ScriptLogFn(std::string strScript, int iLine, std::string strError)
{
	Ogre::LogManager::getSingleton().logMessage(std::string("[Script]Error ") + (strScript.length() ? (std::string("in \"") + strScript + std::string("\"")) : std::string("")) + (iLine==-1 ? std::string("") : (std::string(", line ") + Ogre::StringConverter::toString(iLine))) + std::string(": ") + strError);
}

Main::Main()
{
	mMainSceneMgr = true;
}

Main::~Main()
{
	std::cout << "Bye bye." << std::endl;
}

bool Main::Run()
{
	ResetConfig();
	GetConfig();

#if		_DEBUG
		mRoot = ICE_NEW Ogre::Root("","","ogre.graphics.log");
#else
		mRoot = ICE_NEW Ogre::Root("","","ogre.graphics.log");
#endif

	setupRenderSystem();

	Ogre::LogManager::getSingleton().logMessage("Main Run");

	mWindow = mRoot->initialise(true, "Blackstar Engine");

	size_t window_hwnd;
	mWindow->getCustomAttribute("WINDOW", &window_hwnd);
	createInputSystem(window_hwnd);

	initScene();

	MainLoop::Instance().SetState("Game");

	return true;
}

void Main::ExternInit()
{
	mRoot = Ogre::Root::getSingletonPtr();

	ResetConfig();
	GetConfig();

	setupRenderSystem();
	mRoot->initialise(false);
}

bool Main::Run(Ogre::RenderWindow *window, size_t OISInputWindow)
{
	Ogre::LogManager::getSingleton().logMessage("Main (Embedded) Run");

	mWindow = window;

	createInputSystem(OISInputWindow, true);

	initScene();

	MainLoop::Instance().SetState("Editor");

	return true;
};

void Main::AddOgreResourcePath(Ogre::String dir)
{
	if (dir.find("svn") != Ogre::String::npos) return;
	boost::filesystem::path path(dir.c_str());
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(dir, "FileSystem");
	for (boost::filesystem::directory_iterator i(path); i != boost::filesystem::directory_iterator(); i++)
	{
		if (boost::filesystem::is_directory((*i))) AddOgreResourcePath((*i).path().directory_string().c_str());
	}
}

void Main::initScene()
{
	Ogre::LogManager::getSingleton().logMessage("Main initScene");

	//Start up OgrePhysX
	OgrePhysX::World::getSingleton().init();
#if USE_REMOTEDEBUGGER
	OgrePhysX::World::getSingleton().getSDK()->getFoundationSDK().getRemoteDebugger()->connect("localhost", 5425);
#endif

	//Create Scene
	NxSceneDesc desc;
	desc.gravity = NxVec3(0, -9.81f, 0);
	desc.simType = NX_SIMULATION_SW;
	desc.userNotify = ICE_NEW PhysXUserCallback();
	mPhysXScene = OgrePhysX::World::getSingleton().addScene("Main", desc);
	OgrePhysX::World::getSingleton().getSDK()->setParameter(NxParameter::NX_BOUNCE_THRESHOLD, -1);
	//OgrePhysX::World::getSingleton().getSDK()->setParameter(NxParameter::NX_ADAPTIVE_FORCE, 0.1f);
	//OgrePhysX::World::getSingleton().getSDK()->setParameter(NxParameter::NX_CONTINUOUS_CD, 1);

	//Ground
	mPhysXScene->createActor(OgrePhysX::PlaneShape(Ogre::Vector3(0, 1, 0), -500));

	//Collision Callback
	mPhysXScene->setTriggerReport(ICE_NEW TriggerCallback());

	mPhysXScene->getNxScene()->setGroupCollisionFlag(CollisionGroups::CHARACTER, CollisionGroups::BONE, false);
	mPhysXScene->getNxScene()->setGroupCollisionFlag(CollisionGroups::BONE, CollisionGroups::CHARACTER, false);

	mPhysXScene->getNxScene()->setGroupCollisionFlag(CollisionGroups::AI, CollisionGroups::BONE, false);
	mPhysXScene->getNxScene()->setGroupCollisionFlag(CollisionGroups::AI, CollisionGroups::CHARACTER, false);
	mPhysXScene->getNxScene()->setGroupCollisionFlag(CollisionGroups::AI, CollisionGroups::DEFAULT, false);
	//mPhysXScene->getNxScene()->setGroupCollisionFlag(CollisionGroups::AI, CollisionGroups::LEVELMESH, false);

	mPhysXScene->getNxScene()->setActorGroupPairFlags(CollisionGroups::DEFAULT, CollisionGroups::DEFAULT, NX_NOTIFY_ON_START_TOUCH|NX_NOTIFY_FORCES);
	mPhysXScene->getNxScene()->setActorGroupPairFlags(CollisionGroups::DEFAULT, CollisionGroups::LEVELMESH, NX_NOTIFY_ON_START_TOUCH|NX_NOTIFY_FORCES);

	//Init Ogre Resources
	for (std::vector<KeyVal>::iterator i = mSettings["Resources"].begin(); i != mSettings["Resources"].end(); i++)
	{
		if (i->Key == "Zip") Ogre::ResourceGroupManager::getSingleton().addResourceLocation(i->Val, "Zip");
		if (i->Key == "FileSystem") AddOgreResourcePath(i->Val);
	}

	mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC, "Esgaroth");
	mCamera = mSceneMgr->createCamera("MainCamera");
	mCamera->lookAt(Ogre::Vector3(0,0,1));
	mCamera->setNearClipDistance(0.4f);
	mCamera->setFarClipDistance(50000);

	mViewport = mWindow->addViewport(mCamera);
	mViewport->setBackgroundColour(Ogre::ColourValue::Black);
	mCamera->setAspectRatio(Ogre::Real(mViewport->getActualWidth()) / Ogre::Real(mViewport->getActualHeight()));

	mPreviewSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC, "Esgaroth_Preview");

	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
	Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(Ogre::TFO_ANISOTROPIC);
	Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(8); 
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	//mPreviewSceneMgr->setSkyBox(true, "Sky/ClubTropicana", 2000);
	mPreviewSceneMgr->setAmbientLight(Ogre::ColourValue(0.8f, 0.8f, 0.8f));
	Ogre::Light *ambientlight = mPreviewSceneMgr->createLight("SkyLight");
	ambientlight->setType(Ogre::Light::LightTypes::LT_DIRECTIONAL);
	ambientlight->setDirection(Ogre::Vector3(0,-1,0.4f).normalisedCopy());
	ambientlight->setDiffuseColour(Ogre::ColourValue(1,1,1));
	ambientlight->setSpecularColour(Ogre::ColourValue(1,1,1));

	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.3f,0.2f,0.2f));
	//mSceneMgr->setFog(Ogre::FOG_LINEAR, Ogre::ColourValue::Blue, 0.015);

	mCameraController = ICE_NEW FreeFlightCameraController();
	SceneManager::Instance().AcquireCamera(mCameraController);

	//init scripting stuff
	ScriptSystem::GetInstance();
	LuaScript::SetLoader(OgreFileLoader);
	LuaScript::SetLogFn(ScriptLogFn);
/*	GUISystem::GetInstance();
	MusicSystem::GetInstance();
	ScriptedControls::GetInstance();*/

	//sound
	mSoundManager = OgreOggSound::OgreOggSoundManager::getSingletonPtr();
	mSoundManager->init("");
	mSoundManager->setSceneManager(mSceneMgr);
	mSoundManager->setDistanceModel(AL_LINEAR_DISTANCE);
	//mCamera->getParentSceneNode()->attachObject(mSoundManager->getListener());

	/*Ogre::CompositorManager::getSingleton().addCompositor(GetViewport(), "RenderDepth");
	Ogre::CompositorManager::getSingleton().setCompositorEnabled(GetViewport(), "RenderDepth", true);
	Ogre::CompositorInstance *hdrinstance = Ogre::CompositorManager::getSingleton().addCompositor(Main::Instance().GetViewport(), "HDRWorking");
	HDRListener *hdrListener = new HDRListener(); 
	hdrinstance->addListener(hdrListener);
	hdrListener->notifyViewportSize(mViewport->getActualWidth(), mViewport->getActualHeight());
	hdrListener->notifyCompositor(hdrinstance);
	Ogre::CompositorManager::getSingleton().setCompositorEnabled(GetViewport(), "HDRWorking", true);*/


	mSceneRenderCompositor = Ogre::CompositorManager::getSingleton().addCompositor(GetViewport(), "RenderHDRScene");
	mSceneRenderCompositor->addListener(new VolumetricLightListener());
	Ogre::CompositorManager::getSingleton().setCompositorEnabled(GetViewport(), "RenderHDRScene", true);
	Ogre::CompositorInstance *hdrinstance = Ogre::CompositorManager::getSingleton().addCompositor(Main::Instance().GetViewport(), "DownsampleHDR1");
	HDRListener *hdrListener = new HDRListener(); 
	hdrinstance->addListener(hdrListener);
	hdrListener->notifyViewportSize(mViewport->getActualWidth(), mViewport->getActualHeight());
	hdrListener->notifyCompositor(hdrinstance);
	Ogre::CompositorManager::getSingleton().setCompositorEnabled(GetViewport(), "DownsampleHDR1", true);

	Ogre::MovableObject::setDefaultVisibilityFlags(~3);
	
	mPhysXScene->getNxScene()->setUserContactReport(ICE_NEW ActorContactReport());

	/*mCollisionCallback = ICE_NEW ScriptedCollisionCallback();
	NxOgre::ActorGroup *dynamicbodies = mScene->createActorGroup("DynamicBody");
	mScene->addMaterialPair(
	dynamicbodies->((NxOgre::GroupCallback::InheritedCallback*)(mCollisionCallback));
	dynamicbodies->setCollisionCallback(dynamicbodies, NX_NOTIFY_ALL, false);*/

	mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED);

	mSceneMgr->setShadowTextureCountPerLightType(Ogre::Light::LT_DIRECTIONAL, 3);
	mSceneMgr->setShadowTextureCountPerLightType(Ogre::Light::LT_SPOTLIGHT, 1);
	mSceneMgr->setShadowTextureCount(3);
	mSceneMgr->setShadowTextureConfig(0, 2048, 2048, Ogre::PF_FLOAT32_R);
	mSceneMgr->setShadowTextureConfig(1, 2048, 2048, Ogre::PF_FLOAT32_R);
	mSceneMgr->setShadowTextureConfig(2, 2048, 2048, Ogre::PF_FLOAT32_R);
	/*mSceneMgr->setShadowTextureConfig(3, 1024, 1024, Ogre::PF_FLOAT32_R);
	mSceneMgr->setShadowTextureConfig(4, 1024, 1024, Ogre::PF_FLOAT32_R);
	mSceneMgr->setShadowTextureConfig(5, 1024, 1024, Ogre::PF_FLOAT32_R);
	mSceneMgr->setShadowTextureConfig(6, 1024, 1024, Ogre::PF_FLOAT32_R);
	mSceneMgr->setShadowTextureConfig(7, 1024, 1024, Ogre::PF_FLOAT32_R);*/
	/*mSceneMgr->setShadowTextureSelfShadow(true);*/
	mSceneMgr->setShadowTextureCasterMaterial("shadow_caster");

	mSceneMgr->getShadowTexture(0)->getBuffer()->getRenderTarget()->getViewport(0)->setBackgroundColour(Ogre::ColourValue::White);
	mSceneMgr->getShadowTexture(0)->getBuffer()->getRenderTarget()->getViewport(0)->setClearEveryFrame(true);
	mSceneMgr->getShadowTexture(1)->getBuffer()->getRenderTarget()->getViewport(0)->setBackgroundColour(Ogre::ColourValue::White);
	mSceneMgr->getShadowTexture(1)->getBuffer()->getRenderTarget()->getViewport(0)->setClearEveryFrame(true);
	mSceneMgr->getShadowTexture(2)->getBuffer()->getRenderTarget()->getViewport(0)->setBackgroundColour(Ogre::ColourValue::White);
	mSceneMgr->getShadowTexture(2)->getBuffer()->getRenderTarget()->getViewport(0)->setClearEveryFrame(true);

	mSceneMgr->setShadowCasterRenderBackFaces(false);

	// shadow camera setup
	Ogre::FocusedShadowCameraSetup* spotSetup = ICE_NEW Ogre::FocusedShadowCameraSetup();
	mSpotShadowCameraSetup = Ogre::ShadowCameraSetupPtr(spotSetup);

	Ogre::PSSMShadowCameraSetup* pssmSetup = ICE_NEW Ogre::PSSMShadowCameraSetup();
	pssmSetup->calculateSplitPoints(3, 0.4f, 150, 0.95f);
	pssmSetup->setSplitPadding(1);
	pssmSetup->setOptimalAdjustFactor(0, 2);
	pssmSetup->setOptimalAdjustFactor(1, 0.6f);
	pssmSetup->setOptimalAdjustFactor(2, 0.3f);//0.4f);//2, 0.5);
	mDirectionalShadowCameraSetup = Ogre::ShadowCameraSetupPtr(pssmSetup);
	mSceneMgr->setShadowCameraSetup(mDirectionalShadowCameraSetup);
	const Ogre::PSSMShadowCameraSetup::SplitPointList& splitPointList = pssmSetup->getSplitPoints();
	Ogre::Vector4 PSSMSplitPoints;
	for (int i = 0; i < 3; ++i)
	{
		PSSMSplitPoints[i] = splitPointList[i];
	}

	IceNote("PSSMSplitPoints: " + Ogre::StringConverter::toString(PSSMSplitPoints));

	SceneManager::Instance().Init();

	//Load Plugins
	LoadPlugins();
	//initialize scripts
	ScriptSystem::GetInstance().CreateInstance("InitEngine.lua");

	SceneManager::Instance().PostInit();

	/*Ogre::TexturePtr t_depth = Ogre::TextureManager::getSingleton().createManual("rt_SceneDepth",
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, 512, 512, 0, Ogre::PF_FLOAT16_RGB, Ogre::TU_RENDERTARGET);
	Ogre::RenderTarget* depth_rtt = t_depth->getBuffer()->getRenderTarget();
	depth_rtt->addViewport(mCamera)->setOverlaysEnabled(false);
	depth_rtt->getViewport(0)->setShadowsEnabled(false);
	depth_rtt->getViewport(0)->setAutoUpdated(true);
	depth_rtt->getViewport(0)->setBackgroundColour(Ogre::ColourValue::Black);
	depth_rtt->getViewport(0)->setClearEveryFrame(true);
	depth_rtt->getViewport(0)->setVisibilityMask(4294967281);
	depth_rtt->getViewport(0)->setMaterialScheme("depth");

	Ogre::TexturePtr t_volumetrics = Ogre::TextureManager::getSingleton().createManual("rt_VolumetricObjects",
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, 512, 512, 0, Ogre::PF_FLOAT16_RGB, Ogre::TU_RENDERTARGET);
	Ogre::RenderTarget* volumetrics_rtt = t_volumetrics->getBuffer()->getRenderTarget();
	volumetrics_rtt->addViewport(mCamera)->setOverlaysEnabled(false);
	volumetrics_rtt->getViewport(0)->setAutoUpdated(true);
	volumetrics_rtt->getViewport(0)->setShadowsEnabled(false);
	volumetrics_rtt->getViewport(0)->setClearEveryFrame(true);
	volumetrics_rtt->getViewport(0)->setVisibilityMask(2);
	volumetrics_rtt->getViewport(0)->setMaterialScheme("Volumetrics");*/


};

void Main::LoadOgrePlugins()
{
	for (std::vector<KeyVal>::iterator i = mSettings["OgrePlugins"].begin(); i != mSettings["OgrePlugins"].end(); i++)
	{
		if (i->Key == "Plugin") mRoot->loadPlugin(i->Val);
	}
}

void Main::setupRenderSystem()
{
	Ogre::String renderer = "";
	Ogre::String vsync = "";
	Ogre::String aa = "";
	Ogre::String fullscreen = "";
	Ogre::String width = "";
	Ogre::String height = "";
	Ogre::String perfhud = "";

	for (std::vector<KeyVal>::iterator i = mSettings["Graphics"].begin(); i != mSettings["Graphics"].end(); i++)
	{
		if (i->Key == "Renderer") renderer = i->Val;
		if (i->Key == "AA") aa = i->Val;
		if (i->Key == "VSync") vsync = i->Val;
		if (i->Key == "Fullscreen") fullscreen = i->Val;
		if (i->Key == "ResolutionWidth") width = i->Val;
		if (i->Key == "ResolutionHeight") height = i->Val;
		if (i->Key == "NVPerfHUD") perfhud = i->Val;
	}

#if _DEBUG
	mRoot->loadPlugin("RenderSystem_Direct3D9_d");
	mRoot->loadPlugin("RenderSystem_GL_d");
#else
	mRoot->loadPlugin("RenderSystem_Direct3D9");
	mRoot->loadPlugin("RenderSystem_GL");
#endif

	LoadOgrePlugins();
	/*for (Ogre::Root::PluginInstanceList::const_iterator i = mRoot->getInstalledPlugins().begin(); i != mRoot->getInstalledPlugins().end(); i++)
	{
		if ((*i)->getName() == "AviSaver")
		{
		}
	}*/

	auto pRenderSystem = mRoot->getAvailableRenderers().begin(); 
	Ogre::RenderSystem *pSelectedRenderSystem; 
	pSelectedRenderSystem = *pRenderSystem; 
	while (pRenderSystem != mRoot->getAvailableRenderers().end())
	{
		if ((*pRenderSystem)->getName() == renderer)
		{
			mRenderSystem = *pRenderSystem;
			break;
		}
				
		pRenderSystem++;
	}

	mRoot->setRenderSystem(mRenderSystem);

	mRenderSystem->setConfigOption("Full Screen", fullscreen);

	//mRenderSystem->setConfigOption("Capture frames to AVI file (capture.avi)", "Yes");

	mRenderSystem->setConfigOption("VSync", vsync);
	mRenderSystem->setConfigOption("FSAA", aa);

	if (renderer == "Direct3D9 Rendering Subsystem")
	{
		mRenderSystem->setConfigOption("Allow NVPerfHUD", perfhud);

		Ogre::LogManager::getSingleton().logMessage("setup Rendersystem: " + width + " " + height);
		mRenderSystem->setConfigOption("Video Mode", 
										width
										+ " x " +  height
										+ " @ " +  "32" + "-bit colour"  
										);
				
	}
	else
	{
		mRenderSystem->setConfigOption("Video Mode", width   + " x " + height);

		mRenderSystem->setConfigOption("Colour Depth", "32");

	}

}

void Main::ResetConfig()
{

	mSettings.clear();
	mSettings["Graphics"] = std::vector<KeyVal>();
	mSettings["Graphics"].push_back(KeyVal("Renderer", "Direct3D9 Rendering Subsystem"));
	mSettings["Graphics"].push_back(KeyVal("ResolutionWidth", "1280"));
	mSettings["Graphics"].push_back(KeyVal("ResolutionHeight", "1024"));
	mSettings["Graphics"].push_back(KeyVal("AA", "0"));
	mSettings["Graphics"].push_back(KeyVal("Fullscreen", "No"));
	mSettings["Graphics"].push_back(KeyVal("VSync", "Yes"));
	mSettings["Graphics"].push_back(KeyVal("NVPerfHUD", "Yes"));
}

void Main::GetConfig()
{
	Ogre::ConfigFile cf;
#if		_DEBUG
		cf.load("BlackstarFramework_d.cfg");
#else
		cf.load("BlackstarFramework.cfg");
#endif

	Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
	Ogre::String secName, typeName, archName;
			
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap *Settings = seci.getNext();
		Ogre::ConfigFile::SettingsMultiMap::iterator i;
		if (mSettings.find(secName) == mSettings.end()) mSettings[secName] = std::vector<KeyVal>();

		for (i = Settings->begin(); i != Settings->end(); i++)
		{
			bool added = false;
			if (secName == "Graphics")
			{
				for (auto x = mSettings[secName].begin(); x != mSettings[secName].end(); x++)
				{
					if (x->Key == i->first)
					{
						x->Val = i->second;
						added = true;
						break;
					}
				}
			}
			else mSettings[secName].push_back(KeyVal(i->first, i->second));
		}
	}
}
	
void Main::createInputSystem(size_t windowHnd, bool freeCursor)
{
	Ogre::String width = "";
	Ogre::String height = "";

	for (std::vector<KeyVal>::iterator i = mSettings["Graphics"].begin(); i != mSettings["Graphics"].end(); i++)
	{
		if (i->Key == "ResolutionWidth") width = i->Val;
		if (i->Key == "ResolutionHeight") height = i->Val;
	}
	mInputSystem = ICE_NEW Input(windowHnd, Ogre::StringConverter::parseInt(width), Ogre::StringConverter::parseInt(height), freeCursor);
}

void Main::Shutdown()
{
	if (mRoot)
	{
		mDirectionalShadowCameraSetup.setNull();
		mSpotShadowCameraSetup.setNull();
		mPointShadowCameraSetup.setNull();
		Ogre::LogManager::getSingleton().logMessage("Ogre shutdown!");
		//Console::Instance().Shutdown();
		SceneManager::Instance().Shutdown();
		ClearPlugins();
		ICE_DELETE mCameraController;
		OgrePhysX::World::getSingleton().shutdown();
		auto report = LeakManager::getInstance().reportLeaks();
		Ogre::LogManager::getSingleton().logMessage("Memory leaks: " + Ogre::StringConverter::toString(report.size()));
		for (auto i = report.begin(); i != report.end(); i++)
			Ogre::LogManager::getSingleton().logMessage(*i);
		delete mRoot;
		mRoot = nullptr;
		MainLoop::Instance().quitLoop();
	}
}

Ogre::SceneManager* Main::GetOgreSceneMgr()
{
	return mMainSceneMgr ? mSceneMgr : mPreviewSceneMgr;
};


typedef void (*DLL_START_PLUGIN)(void);
typedef void (*DLL_STOP_PLUGIN)(void);

void Main::LoadPlugins()
{
	for (std::vector<KeyVal>::iterator i = mSettings["BlackstarFrameworkPlugins"].begin(); i != mSettings["BlackstarFrameworkPlugins"].end(); i++)
	{
		if (i->Key == "Plugin") LoadPlugin(i->Val);
	}
}

void Main::InstallPlugin(Ogre::Plugin* plugin)
{
	Ogre::LogManager::getSingleton().logMessage("Installing  plugin: " + plugin->getName());

	mPlugins.push_back(plugin);
	plugin->install();

	plugin->initialise();

	Ogre::LogManager::getSingleton().logMessage(" Plugin successfully installed");
}
//---------------------------------------------------------------------
void Main::UninstallPlugin(Ogre::Plugin* plugin)
{
	Ogre::LogManager::getSingleton().logMessage("Uninstalling  plugin: " + plugin->getName());
	std::vector<Ogre::Plugin*>::iterator i = std::find(mPlugins.begin(), mPlugins.end(), plugin);
	if (i != mPlugins.end())
	{
		plugin->uninstall();
		mPlugins.erase(i);
	}

	Ogre::LogManager::getSingleton().logMessage(" Plugin successfully uninstalled");

}
//-----------------------------------------------------------------------
void Main::LoadPlugin(const Ogre::String& pluginName)
{
	// Load plugin library
    Ogre::DynLib* lib = Ogre::DynLibManager::getSingleton().load(pluginName);
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
void Main::UnloadPlugin(const Ogre::String& pluginName)
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

void Main::ClearPlugins()
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

void Main::StartMainLoop()
{
	MainLoop::Instance().startLoop();
}


Main& Main::Instance()
{
	static Main TheOneAndOnly;
	return TheOneAndOnly;
};

};