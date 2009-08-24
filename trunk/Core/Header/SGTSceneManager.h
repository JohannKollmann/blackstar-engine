
#ifndef __SGTSceneManager_H__
#define __SGTSceneManager_H__

#include "SGTIncludes.h"
#include <list>
#include <map>
#include "Ogre.h"
#include "SGTLoadSave.h"
#include "SGTLevelMesh.h"
#include "SGTScriptSystem.h"
#include "SGTGameObject.h"
#include "SGTGOCEditorInterface.h"
#include "SGTMessageSystem.h"

typedef SGTGOCEditorInterface* (*EDTCreatorFn)();
typedef void (*GOCDefaultParametersFn)(SGTDataMap*);

class SGTDllExport SGTSceneManager : public SGTGOCEditorInterface, public SGTMessageListener
{
private:
	unsigned int mNextID;
	float mDayTime;
	float mMaxDayTime;
	float mTimeScale;

	SGTWeatherController *mWeatherController;

	bool mIndoorRendering;

	SGTLevelMesh *mLevelMesh;

	std::map<Ogre::String, EDTCreatorFn> mEditorInterfaces;

	Ogre::String ScanPath(Ogre::String path, Ogre::String filename);

public:

	std::list<SGTGameObject*> mGameObjects;

	std::map<Ogre::String, std::map<Ogre::String, SGTDataMap*> > mGOCDefaultParameters; //For Editors

	SGTSceneManager(void);
	~SGTSceneManager(void);

	void UpdateGameObjects();

	void CreatePlayer();

	/*
	Liefert eine neue, individuelle ID zur�ck.
	*/
	unsigned int RequestID();
	Ogre::String RequestIDStr();

	SGTGameObject* GetObjectByInternID(int id);

	SGTWeatherController* GetWeatherController();

	void SetToIndoor();

	void SetToOutdoor();

	bool HasLevelMesh();
	SGTLevelMesh* GetLevelMesh();

	void LoadLevelMesh(Ogre::String meshname);

	void LoadLevel(Ogre::String levelfile);

	void SaveLevel(Ogre::String levelfile);

	void BakeStaticMeshShape(Ogre::String meshname);

	void Init();
	void Reset();

	void Shutdown();

	void RegisterEditorInterface(Ogre::String family, Ogre::String type, EDTCreatorFn RegisterFn, GOCDefaultParametersFn DefaulParametersFn);
	void RegisterComponentDefaultParameters(Ogre::String family, Ogre::String type, GOCDefaultParametersFn RegisterFn);

	void ShowEditorMeshes(bool show);

	SGTGOCEditorInterface* CreateComponent(Ogre::String type, SGTDataMap *parameters);

	SGTGameObject* CreateWaypoint();

	//Script
	static std::vector<SGTScriptParam> Lua_LogMessage(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_LoadLevel(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_InsertNpc(SGTScript& caller, std::vector<SGTScriptParam> vParams);

	/*
	Alle Methoden im Format Npc_* erwarten als ersten Parameter die eindeutige ID des AI-Objekts.
	*/

	/*
	Erwartet den anzunehmenden Statename als String
	*/
	static std::vector<SGTScriptParam> Lua_Npc_AddState(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	/*
	Erwartet den Dateinamen des Script-TAs als String (z. B. TA_Sleep.lua)
	*/
	static std::vector<SGTScriptParam> Lua_Npc_AddTA(SGTScript& caller, std::vector<SGTScriptParam> vParams);

	static std::vector<SGTScriptParam> Lua_InsertMesh(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_SetObjectPosition(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_SetObjectOrientation(SGTScript& caller, std::vector<SGTScriptParam> vParams);
	static std::vector<SGTScriptParam> Lua_SetObjectScale(SGTScript& caller, std::vector<SGTScriptParam> vParams);

	//Editor
	void CreateFromDataMap(SGTDataMap *parameters);
	void GetParameters(SGTDataMap *parameters);
	bool IsViewComponent() { return false; }

	//Game clock
	void SetTimeScale(float scale);
	void SetTime(int hours, int minutes);
	int GetHour();
	int GetMinutes();
	void ReceiveMessage(SGTMsg &msg);

	//Singleton
	static SGTSceneManager& Instance();

};

#endif