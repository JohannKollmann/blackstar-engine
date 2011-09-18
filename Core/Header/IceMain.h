
#pragma once

/*
Main
Initialisiert alle Subsysteme.
*/

#include "IceIncludes.h"

#define NOMINMAX
#include "windows.h"

#include "Ogre.h"

#include "boost/thread.hpp"

namespace Ice
{

	class DllExport Main
	{
	public:
		class KeyVal
		{
		public:
			KeyVal(Ogre::String k, Ogre::String v)
				: Key(k), Val(v) {}
			Ogre::String Key;
			Ogre::String Val;
		};

		~Main();

	protected:

		Main();

		Ogre::Root* mRoot;
		Ogre::RenderSystem* mRenderSystem;
		Ogre::Camera* mCamera;
		Ogre::RenderWindow* mWindow;
		Ogre::Viewport* mViewport;
		Ogre::SceneManager* mSceneMgr;
		Ogre::SceneManager* mPreviewSceneMgr;
		bool mMainSceneMgr;
		PhysXSimulationEventCallback *mPhysXSimulationCallback;

		Ogre::ShadowCameraSetupPtr mDirectionalShadowCameraSetup;
		Ogre::ShadowCameraSetupPtr mSpotShadowCameraSetup;
		Ogre::ShadowCameraSetupPtr mPointShadowCameraSetup;

		Ogre::CompositorInstance *mSceneRenderCompositor;

		FreeFlightCameraController* mCameraController;

		OgrePhysX::Scene		*mPhysXScene;
		Input *mInputSystem;

		OgreOggSound::OgreOggSoundManager *mSoundManager;

		// List of plugin DLLs loaded
		std::vector<Ogre::DynLib*> mPluginLibs;
		// List of Plugin instances registered
		std::vector<Ogre::Plugin*> mPlugins;

		int winHeight;
		int winWidth;

		struct MainLoopItem
		{
			boost::thread *thread;
			MainLoopThread *mainLoopThread;
		};

		std::map<Ogre::String, MainLoopItem> mMainLoopThreads;

	public:

		void InitCompositor();
		Ogre::CompositorInstance* GetSceneRenderCompositor() { return mSceneRenderCompositor; }

		Ogre::Entity *mWaterTestEntity;

		// Settings
		std::map<Ogre::String, std::vector<KeyVal> > mSettings;

		void CreateMainLoopThreads();
		void AddMainLoopThread(Ogre::String name, MainLoopThread *thread, bool createThread = true);
		void PauseAllMainLoopThreads(bool paused);
		MainLoopThread* GetMainLoopThread(Ogre::String name);

		bool Run();	//Eigenes Fenster erstellen
		bool Run(Ogre::RenderWindow *window, size_t OISInputWindow);
		void ExternInit();

		void initScene();

		void InitOgreResources();

		void LoadPlugins();

		void LoadOgrePlugins();
		void setupRenderSystem();

		void ResetConfig();
		void GetConfig();

		void createInputSystem(size_t windowHnd, bool freeCursor = false);

		void Shutdown();
		
		Ogre::RenderWindow* GetWindow() { return mWindow; };
		OgrePhysX::Scene* GetPhysXScene() { return mPhysXScene; };
		Ogre::SceneManager* GetOgreSceneMgr();// { return mSceneMgr; };
		Ogre::SceneManager* GetPreviewSceneMgr() { return mPreviewSceneMgr; };
		Ogre::Viewport* GetViewport() { return mViewport; };
		Ogre::Camera* GetCamera() { return mCamera; };
		OgreOggSound::OgreOggSoundManager* GetSoundManager() { return mSoundManager; };
		Input* GetInputManager() { return mInputSystem; };

		FreeFlightCameraController* GetCameraController() { return mCameraController; };

		void SetSceneMgr(bool main = true) { mMainSceneMgr = main; }

		Ogre::ShadowCameraSetupPtr GetDirectionalShadowCameraSetup() { return mDirectionalShadowCameraSetup; };
		Ogre::ShadowCameraSetupPtr GetSpotShadowCameraSetup() { return mSpotShadowCameraSetup; };
		Ogre::ShadowCameraSetupPtr GetPointShadowCameraSetup() { return mPointShadowCameraSetup; };
		void SetDirectionalShadowCameraSetup(Ogre::ShadowCameraSetupPtr setup) { mDirectionalShadowCameraSetup = setup; };
		void SetSpotShadowCameraSetup(Ogre::ShadowCameraSetupPtr setup) { mSpotShadowCameraSetup = setup; };
		void SetPointShadowCameraSetup(Ogre::ShadowCameraSetupPtr setup) { mPointShadowCameraSetup = setup; };

		void AddOgreResourcePath(Ogre::String dir, Ogre::String resourceGroup = "General");

		//Muss vom Plugin aus in dllStartPlugin aufgerufen werden!
		void InstallPlugin(Ogre::Plugin* plugin);
		//Muss vom Plugin aus dllStopPlugin aufgerufen werden!
		void UninstallPlugin(Ogre::Plugin* plugin);
		void LoadPlugin(const Ogre::String& pluginName);
		void UnloadPlugin(const Ogre::String& pluginName);
		void ClearPlugins();

		//Singleton
		static Main& Instance();
	};


};