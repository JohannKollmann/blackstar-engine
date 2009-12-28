
#ifndef __SGTSceneManager_H__
#define __SGTSceneManager_H__

#include "SGTIncludes.h"
#include <list>
#include <map>
#include "Ogre.h"
#include "SGTGameObject.h"
#include "SGTGOCEditorInterface.h"
#include "SGTMessageSystem.h"
#include "SGTScriptSystem.h"

typedef SGTGOCEditorInterface* (*EDTCreatorFn)();
typedef void (*GOCDefaultParametersFn)(SGTDataMap*);

class SGTDllExport SGTSceneManager : public SGTGOCEditorInterface, public SGTMessageListener
{
private:
	unsigned int mNextID;
	float mDayTime;
	float mMaxDayTime;
	float mTimeScale;
	bool mClockEnabled;
	Ogre::Vector3 mNextPreviewPos;

	SGTWeatherController *mWeatherController;

	bool mIndoorRendering;

	SGTLevelMesh *mLevelMesh;

	std::map<Ogre::String, EDTCreatorFn> mEditorInterfaces;

public:

	std::list<SGTGameObject*> mGameObjects;

	std::map<Ogre::String, std::map<Ogre::String, SGTDataMap*> > mGOCDefaultParameters; //For Editors

	SGTSceneManager(void);
	~SGTSceneManager(void);

	void UpdateGameObjects();

	/*
	Liefert eine neue, individuelle ID zurück.
	*/
	unsigned int RequestID();
	Ogre::String RequestIDStr();

	SGTGameObject* GetObjectByInternID(int id);
	void ClearGameObjects();

	SGTWeatherController* GetWeatherController();

	void SetToIndoor();
	void SetToOutdoor();

	bool HasLevelMesh();
	SGTLevelMesh* GetLevelMesh();

	void LoadLevelMesh(Ogre::String meshname);

	void LoadLevel(Ogre::String levelfile, bool load_dynamic = true);
	void SaveLevel(Ogre::String levelfile);

	void Init();
	void Reset();
	void Shutdown();

	//Script

	static std::vector<SGTScriptParam> Lua_LogMessage(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_LoadLevel(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_CreateNpc(SGTScript& caller, std::vector<SGTScriptParam> vParams);

	static std::vector<SGTScriptParam> Lua_CreatePlayer(SGTScript& caller, std::vector<SGTScriptParam> vParams);

	/*
	Alle Methoden im Format Npc_* erwarten als ersten Parameter die eindeutige ID des AI-Objekts.
	*/

	/*
	Getter und Setter für Npc-Properties.
	Erwartet den Namen der Property als String und einen beliebigen Wert.
	*/
	static std::vector<SGTScriptParam> Lua_Npc_SetProperty(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_Npc_GetProperty(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	/*
	Erwartet den anzunehmenden Statename als String
	*/
	static std::vector<SGTScriptParam> Lua_Npc_AddState(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	/*
	Erwartet den Dateinamen des Script-TAs als String (z. B. TA_Sleep.lua)
	*/
	static std::vector<SGTScriptParam> Lua_Npc_AddTA(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	/*
	Erwartet den Ziel-WP
	*/
	static std::vector<SGTScriptParam> Lua_Npc_GotoWP(SGTScript& caller, std::vector<SGTScriptParam> vParams);

	static std::vector<SGTScriptParam> Lua_InsertMesh(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_SetObjectPosition(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_SetObjectOrientation(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_SetObjectScale(SGTScript& caller, std::vector<SGTScriptParam> vParams);

	//Editor
	void CreateFromDataMap(SGTDataMap *parameters);
	void GetParameters(SGTDataMap *parameters);
	void AttachToGO(SGTGameObject *go) {};
	Ogre::String GetLabel() { return ""; }

	void RegisterEditorInterface(Ogre::String family, Ogre::String type, EDTCreatorFn RegisterFn, GOCDefaultParametersFn DefaulParametersFn);
	void RegisterComponentDefaultParameters(Ogre::String family, Ogre::String type, GOCDefaultParametersFn RegisterFn);

	void ShowEditorMeshes(bool show);

	SGTGOCEditorInterface* CreateComponent(Ogre::String type, SGTDataMap *parameters);

	//Game clock
	void EnableClock(bool enable);
	void SetTimeScale(float scale);
	void SetTime(int hours, int minutes);
	int GetHour();
	int GetMinutes();
	void ReceiveMessage(SGTMsg &msg);

	//Preview render helper functions (for items, editor object preview etc.) 
	Ogre::TexturePtr CreatePreviewRender(Ogre::SceneNode *node, Ogre::String name, float width = 512, float height = 512);
	void DestroyPreviewRender(Ogre::String name);

	//Singleton
	static SGTSceneManager& Instance();

};

#endif