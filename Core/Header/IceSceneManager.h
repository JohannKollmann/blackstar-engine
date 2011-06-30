
#pragma once

#include "IceIncludes.h"
#include <list>
#include <map>
#include "Ogre.h"
#include "IceGOCEditorInterface.h"
#include "IceMessageSystem.h"
#include "IceScriptSystem.h"
#include "IceSoundMaterialTable.h"
#include "IceGameObject.h"
#include "OgreOggSound.h"

namespace OgreOggSound
{
	class OgreOggISound;	//Forward declaration
}

namespace Ice
{

	typedef GOCEditorInterface* (*EDTCreatorFn)();
	typedef void (*GOCDefaultParametersFn)(DataMap*);

	class DllExport SceneManager : public EditorInterface, public ViewMessageListener
	{
	public:

		class TimeListener
		{
		public:
			TimeListener() { SceneManager::Instance().mTimeListeners.push_back(this); }
			virtual ~TimeListener() { SceneManager::Instance().mTimeListeners.remove(this); }

			virtual void UpdateScene(float time) = 0;
		};

	private:
		unsigned int mNextID;
		float mDayTime;
		float mMaxDayTime;
		float mTimeScale;
		bool mClockEnabled;
		Ogre::Vector3 mNextPreviewPos;

		//created when SceneBlendFactor is loaded
		Ogre::String mStartupScriptName;

		WeatherController *mWeatherController;

		GameObjectPtr mPlayer;

		bool mIndoorRendering;

		std::map<int, GameObjectPtr> mGameObjects;

		std::list<TimeListener*> mTimeListeners;

		std::map<Ogre::String, GOCEditorInterfacePtr> mGOCPrototypes;

		std::stack<CameraController*> mCameraStack;

		bool mShowEditorVisuals;

		bool mClearingScene;

		SoundMaterialTable mSoundMaterialTable;

		class DllExport OggListener : public OgreOggSound::OgreOggISound::SoundListener
		{
			void soundStopped(OgreOggSound::OgreOggISound* sound);
		};
		OggListener mOggListener;

		class DllExport OggCamSync : public ViewMessageListener
		{
		public:
			OggCamSync();
			~OggCamSync() {}
			void ReceiveMessage(Msg &msg);
		};
		OggCamSync mOggCamSync;

		SceneManager();

	public:

		~SceneManager();

		bool GetClearingScene() { return mClearingScene; }

		SoundMaterialTable& GetSoundMaterialTable() { return mSoundMaterialTable; }

		void RegisterPlayer(GameObjectPtr player);
		GameObjectPtr GetPlayer() { return mPlayer; }

		/** Acquires control over the main camera. GOCSimpleCameraController::AttachCamera will be called.
		The old active camera controller gets pushed on a stack an will be informed when this camera controller calls FreeCamera.
		@param cam The new camera controller.
		*/
		void AcquireCamera(CameraController *cam);

		///Tells the system that the current camera controller has done its job.
		void TerminateCurrentCameraController();

		std::map<Ogre::String, std::map<Ogre::String, DataMap> > mGOCDefaultParameters; //For Editors

		/*
		Liefert eine neue, individuelle ID zurück.
		*/
		unsigned int RequestID();
		Ogre::String RequestIDStr();

		GameObjectPtr GetObjectByInternID(int id);
		void ClearGameObjects();

		std::map<int, GameObjectPtr>& GetGameObjects();

		void RegisterGameObject(GameObjectPtr object);

		void RemoveGameObject(int objectID);
		GameObjectPtr CreateGameObject();

		WeatherController* GetWeatherController();

		void SetToIndoor();
		void SetToOutdoor();

		GameObjectPtr AddLevelMesh(Ogre::String meshname);

		///Setups the navigation mesh by adding all static actors to it.
		void CreateNavigationMesh();

		void LoadLevel(Ogre::String levelfile, bool load_dynamic = true);
		void SaveLevel(Ogre::String levelfile);

		void Init();
		void PostInit(); //Called after plugins are installed and init script is called
		void Reset();
		void Shutdown();

		//Editor
		void SetParameters(DataMap *parameters);
		void GetParameters(DataMap *parameters);

		void SetWeatherParameters(DataMap *parameters);
		void GetWeatherParameters(DataMap *parameters);

		void RegisterComponentDefaultParams(Ogre::String editFamily, Ogre::String type, DataMap &params);

		void RegisterGOCPrototype(GOCEditorInterfacePtr prototype);
		void RegisterGOCPrototype(Ogre::String editFamily, GOCEditorInterfacePtr prototype);

		void ShowEditorMeshes(bool show);
		bool GetShowEditorVisuals() { return mShowEditorVisuals; }

		GOCEditorInterface* GetGOCPrototype(Ogre::String type);
		GOCEditorInterface* NewGOC(Ogre::String type);

		//Game clock
		void EnableClock(bool enable);
		void SetTimeScale(float scale);
		void SetTime(int hours, int minutes);
		int GetHour();
		int GetMinutes();
		void ReceiveMessage(Msg &msg);

		bool IsClockEnabled() { return mClockEnabled; }

		//Preview render helper functions (for items, editor object preview etc.) 
		Ogre::TexturePtr CreatePreviewRender(Ogre::SceneNode *node, Ogre::String name, float width = 512, float height = 512);
		void DestroyPreviewRender(Ogre::String name);

		//Singleton
		static SceneManager& Instance();


		//Script
		static void LogMessage(std::string strError);
		static std::vector<ScriptParam> Lua_LogMessage(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_LoadLevel(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_SaveLevel(Script& caller, std::vector<ScriptParam> vParams);

		//Object creation - retrieves the object id
		static std::vector<ScriptParam> Lua_CreateGameObject(Script& caller, std::vector<ScriptParam> vParams); 

		//Sound system
		static std::vector<ScriptParam> Lua_CreateMaterialProfile(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_Play3DSound(Script& caller, std::vector<ScriptParam> vParams);

		/*
		Liefert die ID des mit dem caller script assozierten Script Objekts zurück.
		*/
		static std::vector<ScriptParam> Lua_GetThis(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_GetObjectByName(Script& caller, std::vector<ScriptParam> vParams);

		static std::vector<ScriptParam> Lua_InsertMesh(Script& caller, std::vector<ScriptParam> vParams);

		//Game clock methods
		static std::vector<ScriptParam> Lua_GetGameTimeHour(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_GetGameTimeMinutes(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_SetGameTime(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_SetGameTimeScale(Script& caller, std::vector<ScriptParam> vParams);

		static std::vector<ScriptParam> Lua_ConcatToString(Script& caller, std::vector<ScriptParam> vParams);

		static std::vector<ScriptParam> Lua_GetRandomNumber(Script& caller, std::vector<ScriptParam> vParams);

		/*
		Liefert die ID des aktuell fokussierten Objekts zurück.
		Liefert -1, falls kein Objekt fokussiert ist.
		*/
		static std::vector<ScriptParam> Lua_GetFocusObject(Script& caller, std::vector<ScriptParam> params);

		//Retrieves the player object id
		static std::vector<ScriptParam> Lua_GetPlayer(Script& caller, std::vector<ScriptParam> params);

	};

}