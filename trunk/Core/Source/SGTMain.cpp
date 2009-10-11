
#include "SGTMain.h"
#include "SGTGameState.h"
#include "SGTInput.h"
#include "SGTMessageSystem.h"
#include "SGTCameraController.h"
#include "SGTConsole.h"
#include "SGTSceneManager.h"
//#include "standard_atoms.h"		//Load Save
#include "SGTCompositorLoader.h"
#include "SGTSceneListener.h"
#include "SGTOcean.h"
#include "HDRListener.h"
#include "SGTWeatherController.h"
#include "SGTCollisionCallback.h"
#include "..\GUISystem.h"
#include "SSAOListener.h"

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"

#define USE_REMOTEDEBUGGER 1

SGTMain::SGTMain()
{
	mNxWorld = NULL;
}

SGTMain::~SGTMain()
{
	std::cout << "Bye bye." << std::endl;
}

bool SGTMain::Run()
{
	ResetConfig();
	GetConfig();

#if		_DEBUG
		mRoot = new Ogre::Root("","","ogre.graphics.log");
#else
		mRoot = new Ogre::Root("","","ogre.graphics.log");
#endif

	setupRenderSystem();

	SGTKernel::Instance();

	Ogre::LogManager::getSingleton().logMessage("SGTMain Run");

	mWindow = mRoot->initialise(true, "Blackstar Engine");

	size_t window_hwnd;
	mWindow->getCustomAttribute("WINDOW", &window_hwnd);
	createInputSystem(window_hwnd);

	initScene();

	SGTKernel::Instance().SetState("Game");

	return true;
}

void SGTMain::ExternInit()
{
	mRoot = Ogre::Root::getSingletonPtr();

	ResetConfig();
	GetConfig();

	setupRenderSystem();
	mRoot->initialise(false);
}

bool SGTMain::Run(Ogre::RenderWindow *window, size_t OISInputWindow)
{
	Ogre::LogManager::getSingleton().logMessage("SGTMain (Embedded) Run");

	mWindow = window;

	createInputSystem(OISInputWindow, true);

	initScene();

	SGTKernel::Instance().SetState("Editor");

	return true;
};

void SGTMain::AddOgreResourcePath(Ogre::String dir)
{
	boost::filesystem::path path(dir.c_str());
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(dir, "FileSystem");
	for (boost::filesystem::directory_iterator i(path); i != boost::filesystem::directory_iterator(); i++)
	{
		if (boost::filesystem::is_directory((*i))) AddOgreResourcePath((*i).path().directory_string().c_str());
	}
}

void SGTMain::initScene()
{
	Ogre::LogManager::getSingleton().logMessage("SGTMain initScene");

	//Init Ogre Resources
	for (std::vector<KeyVal>::iterator i = mSettings["Resources"].begin(); i != mSettings["Resources"].end(); i++)
	{
		if (i->Key == "OgreZip") Ogre::ResourceGroupManager::getSingleton().addResourceLocation(i->Val, "Zip");
		if (i->Key == "OgreMedia") AddOgreResourcePath(i->Val);
		if (i->Key == "LuaScripts") SGTSceneManager::Instance().AddScriptLocation(i->Val);
	}

	mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC, "Esgaroth");
	mCamera = mSceneMgr->createCamera("MainCamera");
	mCamera->lookAt(Ogre::Vector3(0,0,0));
	mCamera->setNearClipDistance(1);
	mCamera->setFarClipDistance(99999*6);

	mViewport = mWindow->addViewport(mCamera);
	mViewport->setBackgroundColour(Ogre::ColourValue::Black);
	mCamera->setAspectRatio(Ogre::Real(mViewport->getActualWidth()) / Ogre::Real(mViewport->getActualHeight()));

	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
	Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(Ogre::TFO_ANISOTROPIC);
	Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(8); 
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.3,0.3,0.3));
	//mSceneMgr->setFog(Ogre::FOG_LINEAR, Ogre::ColourValue::Blue, 0.015);

	//Start up NxOgre and pass on Ogre Root and the SceneManager as arguments.
	NxOgre::PhysXParams pp;
	pp.setToDefault();
	//pp.mFlags.mNoHardware = true;
	mNxWorld = new NxOgre::World(pp);//"time-controller: ogre");
	Ogre::LogManager::getSingleton().logMessage("Initialising NxOgre...");
	NxOgre::SceneParams sp;
	sp.setToDefault();
	sp.mRenderer = NxOgre::SceneParams::RN_OGRE;
	sp.mController = NxOgre::SceneParams::CN_ACCUMULATOR;
	sp.mGravity = NxVec3(0, -9.81, 0);
	//sp.mThreadMask = 0xffffffff;
	//sp.mInternalThreadCount = 3;
	sp.mSceneFlags.toDefault();
	sp.mSceneFlags.mEnableMultithread = true;
	sp.mSceneFlags.mSimulateSeperateThread = true;
	mScene = mNxWorld->createScene("Esgaroth", mSceneMgr, sp);//"renderer: ogre, controller: accumulator, gravity: yes, flags: multithread");
#if USE_REMOTEDEBUGGER
	mNxWorld->getPhysXDriver()->createDebuggerConnection();
#endif
	Ogre::LogManager::getSingleton().logMessage("Done");

	mScene->createShapeGroup("Collidable");
	mScene->createShapeGroup("BoneActor");

	mCharacterControllerManager = NxCreateControllerManager(mNxWorld->getPhysXDriver()->getUserAllocator());

	mScene->createActor("FakeFloor", new NxOgre::Cube(500,0.1f,500), Ogre::Vector3(0,-500,0), "static: yes");

	mCameraController = new SGTCameraController();
	SGTGUISystem::GetInstance();

	mSoundManager = new OgreOggSound::OgreOggSoundManager();
	mSoundManager->init("");
	mSoundManager->setDistanceModel(AL_LINEAR_DISTANCE);
	//mCamera->getParentSceneNode()->attachObject(mSoundManager->getListener());

	SGTConsole::Instance().Init();
	SGTConsole::Instance().Show(false);

	/*SSAOListener *ssaoParamUpdater = new SSAOListener();
	SGTCompositorLoader::Instance().AddListener("ssao", ssaoParamUpdater);
	SGTCompositorLoader::Instance().EnableCompositor("ssao");*/
	//SGTCompositorLoader::Instance().EnableCompositor("Bloom2");
	HDRListener *hdrListener = new HDRListener(); 
	Ogre::CompositorInstance* hdrinstance = SGTCompositorLoader::Instance().AddListener("HDR", hdrListener);
	hdrListener->notifyViewportSize(mViewport->getActualWidth(), mViewport->getActualHeight());
	hdrListener->notifyCompositor(hdrinstance);
	SGTCompositorLoader::Instance().EnableCompositor("HDR");


	SGTSceneManager::Instance().Init();

	mCollisionCallback = new SGTCollisionCallback();
	/*NxOgre::ActorGroup *dynamicbodies = mScene->createActorGroup("DynamicBody");
	dynamicbodies->setCallback((NxOgre::GroupCallback::InheritedCallback*)(mCollisionCallback));
	dynamicbodies->setCollisionCallback(dynamicbodies, NX_NOTIFY_ALL, false);*/

	mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED);

	mSceneMgr->setShadowTextureCountPerLightType(Ogre::Light::LT_DIRECTIONAL, 3);
	mSceneMgr->setShadowTextureCountPerLightType(Ogre::Light::LT_SPOTLIGHT, 1);
	mSceneMgr->setShadowTextureCount(8);
	mSceneMgr->setShadowTextureConfig(0, 1024, 1024, Ogre::PF_FLOAT32_R);
	mSceneMgr->setShadowTextureConfig(1, 1024, 1024, Ogre::PF_FLOAT32_R);
	mSceneMgr->setShadowTextureConfig(2, 1024, 1024, Ogre::PF_FLOAT32_R);
	mSceneMgr->setShadowTextureConfig(3, 1024, 1024, Ogre::PF_FLOAT32_R);
	mSceneMgr->setShadowTextureConfig(4, 1024, 1024, Ogre::PF_FLOAT32_R);
	mSceneMgr->setShadowTextureConfig(5, 1024, 1024, Ogre::PF_FLOAT32_R);
	mSceneMgr->setShadowTextureConfig(6, 1024, 1024, Ogre::PF_FLOAT32_R);
	mSceneMgr->setShadowTextureConfig(7, 1024, 1024, Ogre::PF_FLOAT32_R);
	mSceneMgr->setShadowTextureSelfShadow(true);
	mSceneMgr->setShadowTextureCasterMaterial("shadow_caster");

	mSceneMgr->setShadowCasterRenderBackFaces(false);

	// shadow camera setup
	Ogre::FocusedShadowCameraSetup* spotSetup = new Ogre::FocusedShadowCameraSetup();
	mSpotShadowCameraSetup = Ogre::ShadowCameraSetupPtr(spotSetup);

	Ogre::PSSMShadowCameraSetup* pssmSetup = new Ogre::PSSMShadowCameraSetup();
	pssmSetup->calculateSplitPoints(3, mCamera->getNearClipDistance(), 250, 0.95);
	pssmSetup->setSplitPadding(1);
	pssmSetup->setOptimalAdjustFactor(0, 5);
	pssmSetup->setOptimalAdjustFactor(1, 1);
	pssmSetup->setOptimalAdjustFactor(2, 0.5);
	mDirectionalShadowCameraSetup = Ogre::ShadowCameraSetupPtr(pssmSetup);
	mSceneMgr->setShadowCameraSetup(mDirectionalShadowCameraSetup);
	const Ogre::PSSMShadowCameraSetup::SplitPointList& splitPointList = pssmSetup->getSplitPoints();
	Ogre::Vector4 PSSMSplitPoints;
	for (int i = 0; i < 3; ++i)
	{
		PSSMSplitPoints[i] = splitPointList[i];
	}

	Ogre::LogManager::getSingleton().logMessage("PSSMSplitPoints: " + Ogre::StringConverter::toString(PSSMSplitPoints));

	/*RefractionTextureListener *TestRefractionListener = new RefractionTextureListener();
	ReflectionTextureListener *TestReflectionListener = new ReflectionTextureListener(mCamera);*/

	mWaterTestEntity = 0;

	/*Ogre::ResourceManager::ResourceMapIterator RI = Ogre::MaterialManager::getSingleton().getResourceIterator();
	while (RI.hasMoreElements())
	{
		Ogre::MaterialPtr mat = RI.getNext();
		if (mat->getTechnique(0)->getPass("SkyLight") != NULL) mat->getTechnique(0)->getPass("SkyLight")->getFragmentProgramParameters()->setNamedConstant("pssmSplitPoints", mPSSMSplitPoints);

		/*if (mat->getTechnique(0)->getPass("ReflactionRefraction") != NULL)
		{

			Ogre::TexturePtr Texture = Ogre::TextureManager::getSingleton().createManual( "Refraction", 
				Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, 
			512, 512, 0, Ogre::PF_R8G8B8, Ogre::TU_RENDERTARGET );
        //RenderTexture* rttTex = mRoot->getRenderSystem()->createRenderTexture( "Refraction", 512, 512 );
        Ogre::RenderTarget *rttTex = Texture->getBuffer()->getRenderTarget();
		
        {
            Ogre::Viewport *v = rttTex->addViewport( mCamera );
            mat->getTechnique(0)->getPass(0)->getTextureUnitState(2)->setTextureName("Refraction");
            v->setOverlaysEnabled(false);
            rttTex->addListener(TestRefractionListener);
        }
        
		Texture = Ogre::TextureManager::getSingleton().createManual( "Reflection", 
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, 
			512, 512, 0, Ogre::PF_R8G8B8, Ogre::TU_RENDERTARGET );
        //rttTex = mRoot->getRenderSystem()->createRenderTexture( "Reflection", 512, 512 );
		rttTex = Texture->getBuffer()->getRenderTarget();
        {
            Ogre::Viewport *v = rttTex->addViewport( mCamera );
            mat->getTechnique(0)->getPass(0)->getTextureUnitState(1)->setTextureName("Reflection");
            v->setOverlaysEnabled(false);
            rttTex->addListener(TestReflectionListener);
        }
		}
	}*/

	/*Ogre::ShadowCameraSetupPtr shadowsetup = Ogre::ShadowCameraSetupPtr(new Ogre::FocusedShadowCameraSetup());
	mSceneMgr->setShadowCameraSetup(shadowsetup);*/

	/*mSceneMgr->addListener(new SGTSceneListener());*/

};

void SGTMain::LoadOgrePlugins()
{
	for (std::vector<KeyVal>::iterator i = mSettings["OgrePlugins"].begin(); i != mSettings["OgrePlugins"].end(); i++)
	{
		if (i->Key == "Plugin") mRoot->loadPlugin(i->Val);
	}
}

void SGTMain::setupRenderSystem()
{
	Ogre::String renderer = "";
	Ogre::String vsync = "";
	Ogre::String aa = "";
	Ogre::String fullscreen = "";
	Ogre::String width = "";
	Ogre::String height = "";

	for (std::vector<KeyVal>::iterator i = mSettings["Graphics"].begin(); i != mSettings["Graphics"].end(); i++)
	{
		if (i->Key == "Renderer") renderer = i->Val;
		if (i->Key == "AA") aa = i->Val;
		if (i->Key == "VSync") vsync = i->Val;
		if (i->Key == "Fullscreen") fullscreen = i->Val;
		if (i->Key == "ResolutionWidth") width = i->Val;
		if (i->Key == "ResolutionHeight") height = i->Val;
	}

	LoadOgrePlugins();

	Ogre::RenderSystemList *pRenderSystemList; 
	pRenderSystemList = mRoot->getAvailableRenderers(); 
	Ogre::RenderSystemList::iterator pRenderSystem; 
	pRenderSystem = pRenderSystemList->begin(); 
	Ogre::RenderSystem *pSelectedRenderSystem; 
	pSelectedRenderSystem = *pRenderSystem; 
		
	while (pRenderSystem != pRenderSystemList->end())
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

	mRenderSystem->setConfigOption("VSync", vsync);
	mRenderSystem->setConfigOption("Anti aliasing", "Level " + aa);

	if (renderer == "Direct3D9 Rendering Subsystem")
	{
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

void SGTMain::ResetConfig()
{

	mSettings.clear();
	mSettings["Graphics"] = std::vector<KeyVal>();
	mSettings["Graphics"].push_back(KeyVal("Renderer", "Direct3D9 Rendering Subsystem"));
	mSettings["Graphics"].push_back(KeyVal("ResolutionWidth", "1280"));
	mSettings["Graphics"].push_back(KeyVal("ResolutionHeight", "1024"));
	mSettings["Graphics"].push_back(KeyVal("AA", "0"));
	mSettings["Graphics"].push_back(KeyVal("Fullscreen", "No"));
	mSettings["Graphics"].push_back(KeyVal("VSync", "Yes"));
}

void SGTMain::GetConfig()
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
		if (mSettings.find("secName") == mSettings.end()) mSettings[secName] = std::vector<KeyVal>();

		for (i = Settings->begin(); i != Settings->end(); i++)
		{
			mSettings[secName].push_back(KeyVal(i->first, i->second));
		}
	}
}
	
void SGTMain::createInputSystem(size_t windowHnd, bool freeCursor)
{
	Ogre::String width = "";
	Ogre::String height = "";

	for (std::vector<KeyVal>::iterator i = mSettings["Graphics"].begin(); i != mSettings["Graphics"].end(); i++)
	{
		if (i->Key == "ResolutionWidth") width = i->Val;
		if (i->Key == "ResolutionHeight") height = i->Val;
	}
	mInputSystem = new SGTInput(windowHnd, Ogre::StringConverter::parseInt(width), Ogre::StringConverter::parseInt(height), freeCursor);
}

void SGTMain::Shutdown()
{
	if (mRoot)
	{
		mDirectionalShadowCameraSetup.setNull();
		mSpotShadowCameraSetup.setNull();
		mPointShadowCameraSetup.setNull();
		Ogre::LogManager::getSingleton().logMessage("Ogre shutdown!");
		SGTConsole::Instance().Shutdown();
		SGTSceneManager::Instance().Shutdown();
		SGTKernel::Instance().ClearPlugins();
		delete mSoundManager;
		delete mCameraController;
		delete mRoot;
		mRoot = 0;
	}
}

Ogre::SceneManager* SGTMain::GetOgreSceneMgr()
{
	return mSceneMgr;
};

SGTMain& SGTMain::Instance()
{
	static SGTMain TheOneAndOnly;
	return TheOneAndOnly;
};