
#pragma once

#include "IceIncludes.h"
#include <list>
#include <map>
#include "Ogre.h"
#include "IceGameObject.h"
#include "IceGOCEditorInterface.h"
#include "IceMessageSystem.h"
#include "IceScriptSystem.h"
#include "IceSoundMaterialTable.h"

namespace OgreOggSound
{
	class OgreOggISound;	//Forward declaration
}

namespace Ice
{

	typedef GOCEditorInterface* (*EDTCreatorFn)();
	typedef void (*GOCDefaultParametersFn)(DataMap*);

	class DllExport SceneManager : public EditorInterface, public MessageListener
	{
	private:
		unsigned int mNextID;
		float mDayTime;
		float mMaxDayTime;
		float mTimeScale;
		bool mClockEnabled;
		Ogre::Vector3 mNextPreviewPos;

		WeatherController *mWeatherController;

		GameObject *mPlayer;

		bool mIndoorRendering;

		LevelMesh *mLevelMesh;

		std::vector<GameObject*> mObjectMessageQueue;
		std::map<int, ManagedGameObject*> mGameObjects;

		std::map<Ogre::String, GOCEditorInterface*> mGOCPrototypes;

		std::stack<GOCSimpleCameraController*> mCameraStack;

		std::vector<OgreOggSound::OgreOggISound*> mPlayingSounds;
		void DestroyStoppedSounds();
		float mDestroyStoppedSoundsDelay; //Hack because of OgreOggSound threading!
		float mDestroyStoppedSoundsLast;

	public:

		SoundMaterialTable mSoundMaterialTable;

		void RegisterPlayer(GameObject *player);
		GameObject* GetPlayer() { return mPlayer; }

		void AcquireCamera(GOCSimpleCameraController *cam);
		void FreeCamera(GOCSimpleCameraController *cam);

		void RegisterSound(OgreOggSound::OgreOggISound* sound);

		void AddToMessageQueue(GameObject *object);

		std::map<Ogre::String, std::map<Ogre::String, DataMap> > mGOCDefaultParameters; //For Editors

		SceneManager(void);
		~SceneManager(void);

		void UpdateGameObjects();

		/*
		Liefert eine neue, individuelle ID zur�ck.
		*/
		unsigned int RequestID();
		Ogre::String RequestIDStr();

		ManagedGameObject* GetObjectByInternID(int id);
		void ClearGameObjects();

		void NotifyGODelete(ManagedGameObject *object);
		int RegisterObject(ManagedGameObject *object);

		std::map<int, ManagedGameObject*>& GetGameObjects();

		void RemoveGameObject(ManagedGameObject *object);
		ManagedGameObject* CreateGameObject();

		WeatherController* GetWeatherController();

		void SetToIndoor();
		void SetToOutdoor();

		bool HasLevelMesh();
		LevelMesh* GetLevelMesh();

		void LoadLevelMesh(Ogre::String meshname);

		void LoadLevel(Ogre::String levelfile, bool load_dynamic = true);
		void SaveLevel(Ogre::String levelfile);

		void Init();
		void Reset();
		void Shutdown();

		//Editor
		void SetParameters(DataMap *parameters);
		void GetParameters(DataMap *parameters);

		void RegisterComponentDefaultParams(Ogre::String editFamily, Ogre::String type, DataMap &params);

		void RegisterGOCPrototype(GOCEditorInterface *prototype);
		void RegisterGOCPrototype(Ogre::String editFamily, GOCEditorInterface *prototype);

		void ShowEditorMeshes(bool show);

		GOCEditorInterface* GetGOCPrototype(Ogre::String type);
		GOCEditorInterface* CreateGOCEditorInterface(Ogre::String type, DataMap *parameters);

		//Game clock
		void EnableClock(bool enable);
		void SetTimeScale(float scale);
		void SetTime(int hours, int minutes);
		int GetHour();
		int GetMinutes();
		void ReceiveMessage(Msg &msg);

		//Preview render helper functions (for items, editor object preview etc.) 
		Ogre::TexturePtr CreatePreviewRender(Ogre::SceneNode *node, Ogre::String name, float width = 512, float height = 512);
		void DestroyPreviewRender(Ogre::String name);

		//Singleton
		static SceneManager& Instance();



		//Script

		static void LogMessage(std::string strError);
		static std::vector<ScriptParam> Lua_LogMessage(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_LoadLevel(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_CreateNpc(Script& caller, std::vector<ScriptParam> vParams);

		static std::vector<ScriptParam> Lua_CreatePlayer(Script& caller, std::vector<ScriptParam> vParams);

		//Sound system
		static std::vector<ScriptParam> Lua_CreateMaterialProfile(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_Play3DSound(Script& caller, std::vector<ScriptParam> vParams);

		/*
		Alle Methoden im Format Npc_* erwarten als ersten Parameter die eindeutige ID des AI-Objekts.
		*/
		static GOCAI* Lua_Npc_ExtractAI(std::vector<ScriptParam> vParams);
		/*
		Liefert die ID des mit dem caller script assozierten AI Objekts zur�ck.
		*/
		static std::vector<ScriptParam> Lua_Npc_GetThis(Script& caller, std::vector<ScriptParam> vParams);
		/*
		Getter und Setter f�r Npc-Properties.
		Erwartet den Namen der Property als String und einen beliebigen Wert.
		*/
		static std::vector<ScriptParam> Lua_Npc_SetProperty(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_Npc_GetProperty(Script& caller, std::vector<ScriptParam> vParams);
		/*
		Erwartet den anzunehmenden Statename als String
		*/
		static std::vector<ScriptParam> Lua_Npc_AddState(Script& caller, std::vector<ScriptParam> vParams);
		/*
		Killt den momentan akiven Action-State (wenn vorhanden)
		*/
		static std::vector<ScriptParam> Lua_Npc_KillActiveState(Script& caller, std::vector<ScriptParam> vParams);
		/*
		Leert die gesamte Action State Queue.
		*/
		static std::vector<ScriptParam> Lua_Npc_ClearQueue(Script& caller, std::vector<ScriptParam> vParams);
		/*
		Erwartet den Dateinamen des Script-TAs als String (z. B. TA_Sleep.lua)
		*/
		static std::vector<ScriptParam> Lua_Npc_AddTA(Script& caller, std::vector<ScriptParam> vParams);
		/*
		Erwartet den Ziel-WP
		*/
		static std::vector<ScriptParam> Lua_Npc_GotoWP(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_Npc_GetDistToWP(Script& caller, std::vector<ScriptParam> vParams);

		static std::vector<ScriptParam> Lua_InsertMesh(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_SetObjectPosition(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_SetObjectOrientation(Script& caller, std::vector<ScriptParam> vParams);
		static std::vector<ScriptParam> Lua_SetObjectScale(Script& caller, std::vector<ScriptParam> vParams);

		/*
		Liefert die ID des aktuell fokussierten Objekts zur�ck.
		Liefert -1, falls kein Objekt fokussiert ist.
		*/
		static std::vector<ScriptParam> Lua_GetFocusObject(Script& caller, std::vector<ScriptParam> params);

		static std::vector<ScriptParam> Lua_NPCOpenDialog(Script& caller, std::vector<ScriptParam> params);
		static std::vector<ScriptParam> Lua_SetObjectVisible(Script& caller, std::vector<ScriptParam> params);
		static std::vector<ScriptParam> Lua_GetObjectName(Script& caller, std::vector<ScriptParam> params);
		

		/*
		Liefert true, wenn Objekt ein Npc ist, sonst false.
		*/
		static std::vector<ScriptParam> Lua_ObjectIsNpc(Script& caller, std::vector<ScriptParam> params);

	};

}