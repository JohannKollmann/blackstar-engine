
#include "SGTSceneManager.h"
#include "SGTMain.h"
#include "SGTWeatherController.h"
#include "SGTSaveable.h"
#include "SGTLevelMesh.h"
#include "SGTOcean.h"
#include "shellapi.h"
#include "SGTScriptSystem.h"
#include "standard_atoms.h"
#include "SGTRagdoll.h"
#include "SGTGOCPhysics.h"
#include "SGTGOCView.h"
#include "SGTGOCPlayerInput.h"
#include "SGTGOCAnimatedCharacter.h"
#include "SGTGOCIntern.h"
#include "SGTGOCAI.h"
#include "SGTAIManager.h"
#include "SGTFollowPathway.h"

SGTSceneManager::SGTSceneManager(void)
{
	mWeatherController = NULL;
	mIndoorRendering = false;
	mLevelMesh = NULL;
	mNextID = 0;
	mDayTime = 0.0f;
	mMaxDayTime = 86400.0f;
	mTimeScale = 512.0f;
	SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
}

SGTSceneManager::~SGTSceneManager(void)
{
	SGTMessageSystem::Instance().QuitNewsgroup(this, "UPDATE_PER_FRAME");
}

unsigned int SGTSceneManager::RequestID()
{
	mNextID++;
	return (mNextID-1);
}

Ogre::String SGTSceneManager::RequestIDStr()
{
	return Ogre::StringConverter::toString(RequestID());
}

void SGTSceneManager::UpdateGameObjects()
{
	for (std::list<SGTGameObject*>::iterator i = mGameObjects.begin(); i != mGameObjects.end(); i++)
	{
		(*i)->ProcessMessages();
	}
}

void SGTSceneManager::RegisterEditorInterface(Ogre::String family, Ogre::String type, EDTCreatorFn RegisterFn, GOCDefaultParametersFn DefaulParametersFn)
{
	mEditorInterfaces.insert(std::make_pair<Ogre::String, EDTCreatorFn>(type, RegisterFn));
	if (family != "") RegisterComponentDefaultParameters(family, type, DefaulParametersFn);
}

void SGTSceneManager::RegisterComponentDefaultParameters(Ogre::String family, Ogre::String type, GOCDefaultParametersFn RegisterFn)
{
	SGTDataMap *parameters = new SGTDataMap();
	RegisterFn(parameters);
	std::map<Ogre::String, std::map<Ogre::String, SGTDataMap*> >::iterator i = mGOCDefaultParameters.find(family);
	if (i == mGOCDefaultParameters.end())
	{
		std::map<Ogre::String, SGTDataMap*> map;
		map.insert(std::make_pair<Ogre::String, SGTDataMap*>(type, parameters));
		mGOCDefaultParameters.insert(std::make_pair<Ogre::String, std::map<Ogre::String, SGTDataMap*> >(family, map));
	}
	else
	{
		(*i).second.insert(std::make_pair<Ogre::String, SGTDataMap*>(type, parameters));
	}
}

void SGTSceneManager::ShowEditorMeshes(bool show)
{
	for (std::list<SGTGameObject*>::iterator i = mGameObjects.begin(); i != mGameObjects.end(); i++)
	{
		if (show)
		{
			if ((*i)->GetComponent("Waypoint") && !((*i)->GetComponent("MeshDebugRenderable")))
			{
				(*i)->AddComponent(new SGTMeshDebugRenderable("Editor_Waypoint.mesh"));
			}
		}
		else
		{
			if ((*i)->GetComponent("MeshDebugRenderable"))
			{
				(*i)->RemoveComponent("MeshDebugRenderable");
			}
		}
		(*i)->ShowEditorVisuals(show);
	}
}

SGTGOCEditorInterface* SGTSceneManager::CreateComponent(Ogre::String type, SGTDataMap *parameters)
{
	std::map<Ogre::String, EDTCreatorFn>::iterator i = mEditorInterfaces.find(type);
	SGTGOCEditorInterface *goc = 0;
	if (i != mEditorInterfaces.end())
	{
		goc = (*i).second();
		goc->CreateFromDataMap(parameters);
		return goc;
	}
	Ogre::LogManager::getSingleton().logMessage("WARNING: SGTSceneManager::CreateComponent - Can't find \"" + type + "\". Returning NULL.");
	return goc;
}

SGTWeatherController* SGTSceneManager::GetWeatherController()
{
	return mWeatherController;
}

void SGTSceneManager::Init()
{
	Reset();

	RegisterStandardAtoms();

	SGTDataMap params;
	SGTLoadSave::Instance().RegisterObject(&SGTDataMap::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTGenericProperty::Register);
	SGTLoadSave::Instance().RegisterObject(&ComponentSection::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTGameObject::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTSaveableDummy::Register);

	SGTLoadSave::Instance().RegisterObject(&SGTMeshRenderable::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTPfxRenderable::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTLocalLightRenderable::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTSound3D::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTGOCRigidBody::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTGOCStaticBody::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTGOCViewContainer::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTGOCAnimatedCharacter::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTGOCAnimatedCharacterBone::Register);
	RegisterEditorInterface("View", "MeshRenderable", (EDTCreatorFn)&SGTMeshRenderable::NewEditorInterfaceInstance, SGTMeshRenderable::GetDefaultParameters);
	RegisterEditorInterface("View", "ParticleSystem", (EDTCreatorFn)&SGTPfxRenderable::NewEditorInterfaceInstance, SGTPfxRenderable::GetDefaultParameters);
	RegisterEditorInterface("View", "LocalLight", (EDTCreatorFn)&SGTLocalLightRenderable::NewEditorInterfaceInstance, SGTLocalLightRenderable::GetDefaultParameters);
	RegisterEditorInterface("View", "Sound3D", (EDTCreatorFn)&SGTSound3D::NewEditorInterfaceInstance, SGTSound3D::GetDefaultParameters);
	RegisterEditorInterface("Physics", "RigidBody", (EDTCreatorFn)&SGTGOCRigidBody::NewEditorInterfaceInstance, SGTGOCRigidBody::GetDefaultParameters);
	RegisterEditorInterface("Physics", "StaticBody", (EDTCreatorFn)&SGTGOCRigidBody::NewEditorInterfaceInstance, SGTGOCRigidBody::GetDefaultParameters);
	RegisterEditorInterface("", "AnimatedCharacter", (EDTCreatorFn)&SGTGOCAnimatedCharacter::NewEditorInterfaceInstance, SGTGOCAnimatedCharacter::GetDefaultParameters);
	RegisterEditorInterface("", "AnimatedCharacterBone", (EDTCreatorFn)&SGTGOCAnimatedCharacterBone::NewEditorInterfaceInstance, SGTGOCAnimatedCharacterBone::GetDefaultParameters);

	//Init NxOgre Resource System
	HANDLE fHandle;
	WIN32_FIND_DATA wfd;
	fHandle=FindFirstFile("Data/Media/Meshes/NXS/*.nxs",&wfd);
	do
	{
		NxOgre::Resources::ResourceSystem::getSingleton()->addMesh("file://Data/Media/Meshes/NXS/" + Ogre::String(wfd.cFileName));
	}
	while (FindNextFile(fHandle,&wfd) && fHandle != (HANDLE)ERROR_FILE_NOT_FOUND);
	FindClose(fHandle);


	//Setup Lua Callback
	SGTScriptSystem::GetInstance().ShareCFunction("LogMessage", &SGTSceneManager::Lua_LogMessage);
	SGTScriptSystem::GetInstance().ShareCFunction("LoadLevel", &SGTSceneManager::Lua_LoadLevel);
	SGTScriptSystem::GetInstance().ShareCFunction("CreateNpc", &SGTSceneManager::Lua_CreateNpc);
	SGTScriptSystem::GetInstance().ShareCFunction("Npc_AddState", &SGTSceneManager::Lua_Npc_AddState);
	SGTScriptSystem::GetInstance().ShareCFunction("Npc_AddTA", &SGTSceneManager::Lua_Npc_AddTA);
	SGTScriptSystem::GetInstance().ShareCFunction("Npc_GotoWP", &SGTSceneManager::Lua_Npc_GotoWP);

	SGTScriptSystem::GetInstance().ShareCFunction("InsertMesh", &SGTSceneManager::Lua_InsertMesh);
	SGTScriptSystem::GetInstance().ShareCFunction("SetObjectPosition", &SGTSceneManager::Lua_SetObjectPosition);
	SGTScriptSystem::GetInstance().ShareCFunction("SetObjectOrientation", &SGTSceneManager::Lua_SetObjectOrientation);
	SGTScriptSystem::GetInstance().ShareCFunction("SetObjectScale", &SGTSceneManager::Lua_SetObjectScale);

}

void SGTSceneManager::Reset()
{
	SetToOutdoor();
}

void SGTSceneManager::Shutdown()
{
	SetToIndoor();
}


void SGTSceneManager::SetToIndoor()
{
	if (mWeatherController)
	{
		delete mWeatherController;
		mWeatherController = NULL;
	}
	mIndoorRendering = true;
}

void SGTSceneManager::SetToOutdoor()
{
	/*SGTMain::Instance().GetOgreSceneMgr()->setSkyBox(true,"Sky/ClubTropicana", 2000);
	Ogre::Light *Light = SGTMain::Instance().GetOgreSceneMgr()->createLight("Light0");
	Light->setType(Ogre::Light::LT_DIRECTIONAL);
	Light->setDirection(0, -1, 0.9);
	Light->setDiffuseColour(1, 1, 1);
	Light->setSpecularColour(Ogre::ColourValue(1, 0.9, 0.6)/5);*/
	
	if (!mWeatherController) mWeatherController = new SGTWeatherController();
	mIndoorRendering = false;
}

bool SGTSceneManager::HasLevelMesh()
{
	return (mLevelMesh == NULL ? false : true);
}

SGTLevelMesh* SGTSceneManager::GetLevelMesh()
{
	return mLevelMesh;
}


void SGTSceneManager::LoadLevelMesh(Ogre::String meshname)
{
	if (mIndoorRendering)
	{
	}
	else
	{
		if (mLevelMesh) delete mLevelMesh;
		mLevelMesh = new SGTLevelMesh(meshname);
	}
}

void SGTSceneManager::LoadLevel(Ogre::String levelfile)
{
	SGTLoadSystem *ls=SGTLoadSave::Instance().LoadFile(levelfile);

	SGTDataMap *levelparams = (SGTDataMap*)ls->LoadObject();
	CreateFromDataMap(levelparams);

	ls->LoadAtom("std::list<SGTSaveable*>", &mGameObjects);
	mNextID = 0;
}

void SGTSceneManager::SaveLevel(Ogre::String levelfile)
{
	SGTSaveSystem *ss=SGTLoadSave::Instance().CreateSaveFile(levelfile, levelfile + ".xml");
	SGTDataMap map;
	GetParameters(&map);
	ss->SaveObject(&map, "LevelParams");
	ss->SaveAtom("std::list<SGTSaveable*>", &mGameObjects, "GameObjects");
	ss->CloseFiles();
	delete ss;
}

void SGTSceneManager::CreateFromDataMap(SGTDataMap *parameters)
{
	Ogre::String levelmesh = parameters->GetOgreString("LevelMesh");
	if (levelmesh != "") LoadLevelMesh(levelmesh.c_str());
	bool indoor = parameters->GetBool("Indoor");
	if (indoor) SetToIndoor();
	else SetToOutdoor();
}

void SGTSceneManager::GetParameters(SGTDataMap *parameters)
{
	if (mLevelMesh) parameters->AddOgreString("LevelMesh", mLevelMesh->GetMeshFileName());
	else parameters->AddOgreString("LevelMesh", "");
	parameters->AddBool("Indoor", mIndoorRendering);
}

Ogre::String SGTSceneManager::ScanPath(Ogre::String path, Ogre::String filename)
{
	HANDLE fHandle;
	WIN32_FIND_DATA wfd;
	fHandle=FindFirstFile((path + "/*").c_str(),&wfd);
	do
	{
		if((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{
			Ogre::String dir = wfd.cFileName;
			if (dir == "." || dir == "..") continue;
			Ogre::String result = "";
			result = ScanPath(path + "/" + Ogre::String(wfd.cFileName),  filename);
			if (result != "") return result;
		}
		if (Ogre::String(wfd.cFileName) == filename)
		{
			return path + "/" + Ogre::String(wfd.cFileName);
		}
	}
	while (FindNextFile(fHandle,&wfd));
	FindClose(fHandle);
	return "";
}

void SGTSceneManager::BakeStaticMeshShape(Ogre::String meshname)
{
	if (!NxOgre::Resources::ResourceSystem::getSingleton()->hasMesh("Data/Media/Meshes/NXS/" + meshname + ".nxs"))
	{
		Ogre::String path = ScanPath("Data/Media/Meshes", meshname);
		if (path == "")
		{
			Ogre::LogManager::getSingleton().logMessage("SGTSceneManager::BakeStaticMeshShape: Could not find " + meshname);
			return;
		}
		Ogre::String command = "flour convert in: " + path + ", into: triangle, out: Data/Media/Meshes/NXS/" + meshname + ".nxs";
		Ogre::LogManager::getSingleton().logMessage(command);
		//spawnl(_P_WAIT, "flour.exe", command.c_str(), NULL);
		//ShellExecute(0, "open", "flour.exe", command.c_str(), 0, SW_SHOWNORMAL); 
		system(command.c_str());
		Ogre::LogManager::getSingleton().logMessage("... Done");
		NxOgre::Resources::ResourceSystem::getSingleton()->addMesh("file://Data/Media/Meshes/NXS/" + meshname + ".nxs");
	}
}

SGTGameObject* SGTSceneManager::GetObjectByInternID(int id)
{
	for (std::list<SGTGameObject*>::iterator i = mGameObjects.begin(); i != mGameObjects.end(); i++)
	{
		if ((*i)->GetID() == id) return (*i);
	}
	return 0;
}

std::vector<SGTScriptParam>
SGTSceneManager::Lua_LoadLevel(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> out;
	if (vParams.size() == 1)
	{
		if (vParams[0].getType() == SGTScriptParam::PARM_TYPE_STRING)
		{
			SGTSceneManager::Instance().LoadLevel(vParams[0].getString());
		}
	}
	return out;
}

std::vector<SGTScriptParam>
SGTSceneManager::Lua_LogMessage(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	Ogre::String msg = "";
	for(unsigned int iArg=0; iArg<vParams.size(); iArg++)
	{
		if(vParams[iArg].getType()==SGTScriptParam::PARM_TYPE_STRING)
			msg = msg + vParams[iArg].getString().c_str();
		else if(vParams[iArg].getType()==SGTScriptParam::PARM_TYPE_BOOL)
			msg = msg + Ogre::StringConverter::toString(vParams[iArg].getBool());
		else if(vParams[iArg].getType()==SGTScriptParam::PARM_TYPE_FLOAT)
		{
			float val = static_cast<float>(vParams[iArg].getFloat());
			msg = msg + Ogre::StringConverter::toString(val);
		}
		else if(vParams[iArg].getType()==SGTScriptParam::PARM_TYPE_INT)
			msg = msg + Ogre::StringConverter::toString(vParams[iArg].getInt());
	}
	Ogre::LogManager::getSingleton().logMessage(msg);
	return std::vector<SGTScriptParam>();
}


std::vector<SGTScriptParam> SGTSceneManager::Lua_Npc_AddState(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> out;
	return out;
}

std::vector<SGTScriptParam> SGTSceneManager::Lua_Npc_AddTA(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> out;
	if (vParams.size() < 3) return out;
	if (!vParams[0].hasInt()) return out;
	if (vParams[1].getType() != SGTScriptParam::PARM_TYPE_STRING) return out;
	if (vParams[2].getType() != SGTScriptParam::PARM_TYPE_STRING) return out;
	int id = vParams[0].getInt();
	std::string ta_script = vParams[1].getString();
	std::string wp = vParams[2].getString();
	int end_timeH = 25;
	int end_timeM = 61;
	bool time_abs = true;
	if (vParams.size() >= 5)
	{
		if (vParams[3].hasInt()) end_timeH = vParams[3].getInt();
		if (vParams[4].hasInt()) end_timeH = vParams[4].getInt();
	}
	if (vParams.size() == 6)
	{
		if (vParams[5].getType() == SGTScriptParam::PARM_TYPE_BOOL) time_abs = vParams[5].getBool();
	}

	SGTGOCAI *ai = SGTAIManager::Instance().GetAIByID(id);
	if (ai) ai->AddScriptedState(new SGTDayCycle(ai, ta_script, wp, end_timeH, end_timeM, time_abs));
	return out;
}
std::vector<SGTScriptParam> SGTSceneManager::Lua_Npc_GotoWP(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> out;
	if (vParams.size() < 2) return out;
	if (!vParams[0].hasInt()) return out;
	if (vParams[1].getType() != SGTScriptParam::PARM_TYPE_STRING) return out;
	int id = vParams[0].getInt();
	std::string wp = vParams[1].getString();
	SGTGOCAI *ai = SGTAIManager::Instance().GetAIByID(id);
	if (ai) ai->AddState(new SGTFollowPathway(wp));
	return out;
}

std::vector<SGTScriptParam> SGTSceneManager::Lua_InsertMesh(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> out;
	if (vParams.size() != 3) return out;
	if (vParams[0].getType() != SGTScriptParam::PARM_TYPE_STRING) return out;
	Ogre::String mesh = vParams[0].getString().c_str();
	if (vParams[1].getType() != SGTScriptParam::PARM_TYPE_BOOL) return out;
	bool shadows = vParams[1].getBool();
	if (vParams[2].getType() != SGTScriptParam::PARM_TYPE_FLOAT) return out;
	int collision = (int)vParams[2].getFloat();

	SGTGameObject *object = new SGTGameObject();
	SGTGOCViewContainer *container = new SGTGOCViewContainer();
	container->AddItem(new SGTMeshRenderable(mesh, shadows));
	object->AddComponent(container);
	if (collision == -1)
	{
		object->AddComponent(new SGTGOCStaticBody(mesh));
	}
	else if (collision >= 0 && collision <= 3)
	{
		object->AddComponent(new SGTGOCRigidBody(mesh, 10, collision));
	}
	out.push_back(SGTScriptParam(object->GetID()));
	return out;
}

std::vector<SGTScriptParam> SGTSceneManager::Lua_SetObjectPosition(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> out;
	if (vParams.size() != 4) return out;
	if (vParams[0].getType() != SGTScriptParam::PARM_TYPE_FLOAT || vParams[1].getType() != SGTScriptParam::PARM_TYPE_FLOAT || vParams[2].getType() != SGTScriptParam::PARM_TYPE_FLOAT || vParams[3].getType() != SGTScriptParam::PARM_TYPE_FLOAT) return out;
	int id = (int)vParams[0].getFloat();
	SGTGameObject *object = SGTSceneManager::Instance().GetObjectByInternID(id);
	if (object)
	{
		float x = vParams[1].getFloat();
		float y = vParams[2].getFloat();
		float z = vParams[3].getFloat();
		object->SetGlobalPosition(Ogre::Vector3(x,y,z));
	}
	return out;
}
std::vector<SGTScriptParam>SGTSceneManager::Lua_SetObjectOrientation(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> out;
	if (vParams.size() != 4) return out;
	if (vParams[0].getType() != SGTScriptParam::PARM_TYPE_FLOAT || vParams[1].getType() != SGTScriptParam::PARM_TYPE_FLOAT || vParams[2].getType() != SGTScriptParam::PARM_TYPE_FLOAT || vParams[3].getType() != SGTScriptParam::PARM_TYPE_FLOAT) return out;
	int id = (int)vParams[0].getFloat();
	SGTGameObject *object = SGTSceneManager::Instance().GetObjectByInternID(id);
	if (object)
	{
		Ogre::Degree yDeg = Ogre::Degree(vParams[1].getFloat());
		Ogre::Degree pDeg = Ogre::Degree(vParams[2].getFloat());
		Ogre::Degree rDeg = Ogre::Degree(vParams[3].getFloat());
		Ogre::Matrix3 mat3;
		mat3.FromEulerAnglesYXZ(yDeg, pDeg, rDeg);
		Ogre::Quaternion q;
		q.FromRotationMatrix(mat3);
		object->SetGlobalOrientation(q);
	}
	return out;
}
std::vector<SGTScriptParam>SGTSceneManager::Lua_SetObjectScale(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> out;
	if (vParams.size() != 4) return out;
	if (vParams[0].getType() != SGTScriptParam::PARM_TYPE_FLOAT || vParams[1].getType() != SGTScriptParam::PARM_TYPE_FLOAT || vParams[2].getType() != SGTScriptParam::PARM_TYPE_FLOAT || vParams[3].getType() != SGTScriptParam::PARM_TYPE_FLOAT) return out;
	int id = (int)vParams[0].getFloat();
	SGTGameObject *object = SGTSceneManager::Instance().GetObjectByInternID(id);
	if (object)
	{
		float x = vParams[1].getFloat();
		float y = vParams[2].getFloat();
		float z = vParams[3].getFloat();
		object->SetGlobalScale(Ogre::Vector3(x,y,z));
	}
	return out;
}

std::vector<SGTScriptParam>
SGTSceneManager::Lua_CreateNpc(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> out;
	Ogre::Vector3 scale(1,1,1);
	Ogre::String mesh = "";
	int returnerID = -1;
	if (vParams.size() > 0)
	{
		if (vParams[0].getType() == SGTScriptParam::PARM_TYPE_STRING)
		{
			mesh = vParams[0].getString().c_str();
		}
	}
	if (vParams.size() == 4)
	{
		if (vParams[1].getType() == SGTScriptParam::PARM_TYPE_FLOAT)
		{
			scale.x = vParams[1].getFloat();
		}
		if (vParams[2].getType() == SGTScriptParam::PARM_TYPE_FLOAT)
		{
			scale.y = vParams[2].getFloat();
		}
		if (vParams[3].getType() == SGTScriptParam::PARM_TYPE_FLOAT)
		{
			scale.z = vParams[3].getFloat();
		}
	}
	if (mesh != "")
	{
		SGTGameObject *go = new SGTGameObject();
		//SGTGOCAnimatedCharacter *body = new SGTGOCAnimatedCharacter(mesh, scale);
		SGTGOCViewContainer *body = new SGTGOCViewContainer();
		body->AddItem(new SGTMeshRenderable("cube.1m.mesh", true));
		SGTGOCAI *ai = new SGTGOCAI();
		go->AddComponent(ai);		//Brain
		go->AddComponent(body);		//Body
		returnerID = (int)ai->GetID();
	}
	out.push_back(SGTScriptParam(returnerID));
	return out;
}

SGTGameObject* SGTSceneManager::CreateWaypoint()
{
	SGTGameObject *object = new SGTGameObject();
	object->AddComponent(new SGTGOCWaypoint());
	object->AddComponent(new SGTMeshDebugRenderable("Editor_Waypoint.mesh"));
	return object;
}

void SGTSceneManager::CreatePlayer()
{
	SGTGameObject* player = new SGTGameObject();
	player->AddComponent(new SGTGOCPlayerInput());
	player->AddComponent(new SGTGOCCameraController(SGTMain::Instance().GetCamera()));
	player->AddComponent(new SGTGOCCharacterController(Ogre::Vector3(1,1.8,1)));
	/*SGTGOCAnimatedCharacter *animated = new SGTGOCAnimatedCharacter("ninja.mesh", Ogre::Vector3(0.01,0.01,0.01));
	player->AddComponent(animated);*/
	SGTGOCViewContainer *container = new SGTGOCViewContainer();
	container->AddItem(new SGTMeshRenderable("cube.1m.mesh", true));
	player->AddComponent(container);
	player->SetGlobalPosition(Ogre::Vector3(0,10,0));

	/*SGTGOCViewContainer *view = new SGTGOCViewContainer();
	view->GetNode()->scale(0.1,0.1,0.1);
	view->AddItem(new SGTMeshRenderable("jaiqua.mesh", true));
	player->AddComponent(view);*/
}

void SGTSceneManager::ReceiveMessage(SGTMsg &msg)
{
	if (msg.mNewsgroup == "UPDATE_PER_FRAME")
	{
		float time = msg.mData.GetFloat("TIME");
		mDayTime += (time*0.001*mTimeScale);
		if (mDayTime >= mMaxDayTime) mDayTime = 0.0f;
	}
}

void SGTSceneManager::SetTimeScale(float scale)
{
	mTimeScale = scale;
	if (mWeatherController) mWeatherController->SetSpeedFactor(scale);
}
void SGTSceneManager::SetTime(int hours, int minutes)
{
	mDayTime = (float)(hours * 3600.0f + minutes * 60.0f);
}
int SGTSceneManager::GetHour()
{
	return (int)(mDayTime / 3600);
}
int SGTSceneManager::GetMinutes()
{
	int hour = GetHour();
	float fhour = 3600.0f * hour;
	float dif = mDayTime - fhour;
	return dif / 60;
}

SGTSceneManager& SGTSceneManager::Instance()
{
	static SGTSceneManager TheOneAndOnly;
	return TheOneAndOnly;
};