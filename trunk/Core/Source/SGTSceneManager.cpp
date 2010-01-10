
#include "SGTSceneManager.h"
#include "SGTMain.h"
#include "SGTLoadSave.h"
#include "SGTWeatherController.h"
#include "SGTSaveable.h"
#include "SGTLevelMesh.h"
#include "shellapi.h"
#include "SGTScriptSystem.h"
#include "standard_atoms.h"
#include "SGTGOCPhysics.h"
#include "SGTGOCView.h"
#include "SGTGOCPlayerInput.h"
#include "SGTGOCAnimatedCharacter.h"
#include "SGTGOCIntern.h"
#include "SGTGOCAI.h"
#include "SGTAIManager.h"
#include "SGTFollowPathway.h"
#include "SGTLevelMesh.h"

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"

#include "SGTUtils.h"

SGTSceneManager::SGTSceneManager(void)
{
	mWeatherController = 0;
	mIndoorRendering = false;
	mClockEnabled = true;
	mLevelMesh = 0;
	mNextID = 0;
	mDayTime = 0.0f;
	mMaxDayTime = 86400.0f;
	mTimeScale = 64.0f;
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

void SGTSceneManager::RegisterPlayer(SGTGameObject *player)
{
	mPlayer = player;
}

void SGTSceneManager::UpdateGameObjects()
{
	for (std::vector<SGTGameObject*>::iterator i = mObjectMessageQueue.begin(); i != mObjectMessageQueue.end(); i++)
	{
		(*i)->ProcessMessages();
	}
	mObjectMessageQueue.clear();
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
	for (std::map<int, SGTGameObject*>::iterator i = mGameObjects.begin(); i != mGameObjects.end(); i++)
	{
		if (show)
		{
			if (i->second->GetComponent("Waypoint") && !(i->second->GetComponent("MeshDebugRenderable")))
			{
				i->second->AddComponent(new SGTMeshDebugRenderable("Editor_Waypoint.mesh"));
			}
		}
		else
		{
			if (i->second->GetComponent("MeshDebugRenderable"))
			{
				i->second->RemoveComponent("MeshDebugRenderable");
			}
		}
		i->second->ShowEditorVisuals(show);
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

	SGTMain::Instance().GetOgreSceneMgr()->createStaticGeometry("StaticGeometry");

	RegisterStandardAtoms();

	SGTDataMap params;
	SGTLoadSave::Instance().RegisterObject(&SGTDataMap::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTGenericProperty::Register);
	SGTLoadSave::Instance().RegisterObject(&ComponentSection::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTGameObject::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTSaveableDummy::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTGOCWaypoint::Register);

	SGTLoadSave::Instance().RegisterObject(&SGTMeshRenderable::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTPfxRenderable::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTLocalLightRenderable::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTSound3D::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTGOCRigidBody::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTGOCStaticBody::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTGOCViewContainer::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTGOCAnimatedCharacter::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTGOCAnimatedCharacterBone::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTGOCCharacterController::Register);
	SGTLoadSave::Instance().RegisterObject(&SGTGOCPlayerInput::Register);
	RegisterEditorInterface("A", "Mesh", (EDTCreatorFn)&SGTMeshRenderable::NewEditorInterfaceInstance, SGTMeshRenderable::GetDefaultParameters);
	RegisterEditorInterface("A", "PFX", (EDTCreatorFn)&SGTPfxRenderable::NewEditorInterfaceInstance, SGTPfxRenderable::GetDefaultParameters);
	RegisterEditorInterface("A", "Light", (EDTCreatorFn)&SGTLocalLightRenderable::NewEditorInterfaceInstance, SGTLocalLightRenderable::GetDefaultParameters);
	RegisterEditorInterface("A", "Sound3D", (EDTCreatorFn)&SGTSound3D::NewEditorInterfaceInstance, SGTSound3D::GetDefaultParameters);
	RegisterEditorInterface("A", "Skeleton", (EDTCreatorFn)&SGTGOCAnimatedCharacter::NewEditorInterfaceInstance, SGTGOCAnimatedCharacter::GetDefaultParameters);
	RegisterEditorInterface("", "AnimatedCharacterBone", (EDTCreatorFn)&SGTGOCAnimatedCharacterBone::NewEditorInterfaceInstance, SGTGOCAnimatedCharacterBone::GetDefaultParameters);

	RegisterEditorInterface("B_x", "RigidBody", (EDTCreatorFn)&SGTGOCRigidBody::NewEditorInterfaceInstance, SGTGOCRigidBody::GetDefaultParameters);
	RegisterEditorInterface("B_x", "StaticBody", (EDTCreatorFn)&SGTGOCStaticBody::NewEditorInterfaceInstance, SGTGOCStaticBody::GetDefaultParameters);
	RegisterEditorInterface("B_x", "Character", (EDTCreatorFn)&SGTGOCCharacterController::NewEditorInterfaceInstance, SGTGOCCharacterController::GetDefaultParameters);

	RegisterEditorInterface("C_x", "Scripted AI", (EDTCreatorFn)&SGTGOCAI::NewEditorInterfaceInstance, SGTGOCAI::GetDefaultParameters);
	RegisterEditorInterface("C_x", "Player Control", (EDTCreatorFn)&SGTGOCPlayerInput::NewEditorInterfaceInstance, SGTGOCPlayerInput::GetDefaultParameters);

	RegisterEditorInterface("D", "Camera", (EDTCreatorFn)&SGTGOCCameraController::NewEditorInterfaceInstance, SGTGOCCameraController::GetDefaultParameters);

	//Setup Lua Callback
	SGTScriptSystem::GetInstance().ShareCFunction("LogMessage", &SGTSceneManager::Lua_LogMessage);
	SGTScriptSystem::GetInstance().ShareCFunction("LoadLevel", &SGTSceneManager::Lua_LoadLevel);
	SGTScriptSystem::GetInstance().ShareCFunction("CreatePlayer", &SGTSceneManager::Lua_CreatePlayer);
	SGTScriptSystem::GetInstance().ShareCFunction("CreateNpc", &SGTSceneManager::Lua_CreateNpc);
	SGTScriptSystem::GetInstance().ShareCFunction("Npc_SetProperty", &SGTSceneManager::Lua_Npc_SetProperty);
	SGTScriptSystem::GetInstance().ShareCFunction("Npc_GetProperty", &SGTSceneManager::Lua_Npc_GetProperty);
	SGTScriptSystem::GetInstance().ShareCFunction("Npc_AddState", &SGTSceneManager::Lua_Npc_AddState);
	SGTScriptSystem::GetInstance().ShareCFunction("Npc_AddTA", &SGTSceneManager::Lua_Npc_AddTA);
	SGTScriptSystem::GetInstance().ShareCFunction("Npc_GotoWP", &SGTSceneManager::Lua_Npc_GotoWP);

	SGTScriptSystem::GetInstance().ShareCFunction("InsertMesh", &SGTSceneManager::Lua_InsertMesh);
	SGTScriptSystem::GetInstance().ShareCFunction("SetObjectPosition", &SGTSceneManager::Lua_SetObjectPosition);
	SGTScriptSystem::GetInstance().ShareCFunction("SetObjectOrientation", &SGTSceneManager::Lua_SetObjectOrientation);
	SGTScriptSystem::GetInstance().ShareCFunction("SetObjectScale", &SGTSceneManager::Lua_SetObjectScale);

	SGTScriptSystem::GetInstance().ShareCFunction("GetFocusObject", &SGTSceneManager::Lua_GetFocusObject);

}

void SGTSceneManager::ClearGameObjects()
{
	std::map<int, SGTGameObject*>::iterator i = mGameObjects.begin();
	while (i != mGameObjects.end())
	{
		delete i->second;
		i = mGameObjects.begin();
	}
	mGameObjects.clear();
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
	SetTimeScale(mTimeScale);
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

void SGTSceneManager::LoadLevel(Ogre::String levelfile, bool load_dynamic)
{
	ClearGameObjects();
	mNextID = 0;

	SGTLoadSystem *ls=SGTLoadSave::Instance().LoadFile(levelfile);

	SGTDataMap *levelparams = (SGTDataMap*)ls->LoadObject();
	CreateFromDataMap(levelparams);

	//Todo
	ls->LoadAtom("std::list<SGTSaveable*>", &mGameObjects);
	if (load_dynamic)
	{
		ls->LoadAtom("std::list<SGTSaveable*>", &mGameObjects);
	}
	ls->CloseFile();
	delete ls;
}

void SGTSceneManager::SaveLevel(Ogre::String levelfile)
{
	ShowEditorMeshes(false);
	//Todo
	/*SGTSaveSystem *ss=SGTLoadSave::Instance().CreateSaveFile(levelfile, levelfile + ".xml");
	SGTDataMap map;
	GetParameters(&map);
	ss->SaveObject(&map, "LevelParams");
	std::list<SGTGameObject*> staticobjects;
	std::list<SGTGameObject*> dynamicobjects;
	for (std::list<SGTGameObject*>::iterator i = mGameObjects.begin(); i != mGameObjects.end(); i++)
	{
		if ((*i)->IsStatic()) staticobjects.push_back(*i);
		else dynamicobjects.push_back(*i);
	}
	ss->SaveAtom("std::list<SGTSaveable*>", &staticobjects, "Static Objects");
	ss->SaveAtom("std::list<SGTSaveable*>", &dynamicobjects, "Dynamic Objects");
	ss->CloseFiles();
	delete ss;*/
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

std::map<int, SGTGameObject*>& SGTSceneManager::GetGameObjects()
{
	return mGameObjects;
}

int SGTSceneManager::RegisterObject(SGTGameObject *object)
{
	int id = RequestID();
	mGameObjects.insert(std::make_pair<int, SGTGameObject*>(id, object));
	return id;
}

void  SGTSceneManager::UnregisterObject(int id)
{
	std::map<int, SGTGameObject*>::iterator i = mGameObjects.find(id);
	if (i != mGameObjects.end()) mGameObjects.erase(i);
}

SGTGameObject* SGTSceneManager::GetObjectByInternID(int id)
{
	std::map<int, SGTGameObject*>::iterator i = mGameObjects.find(id);
	if (i != mGameObjects.end()) return i->second;
	return 0;
}

void SGTSceneManager::AddToMessageQueue(SGTGameObject *object)
{
	mObjectMessageQueue.push_back(object);
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


std::vector<SGTScriptParam> SGTSceneManager::Lua_Npc_SetProperty(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> out;
	if (vParams.size() < 3) return out;
	if (!vParams[0].hasInt()) return out;
	if (vParams[1].getType() != SGTScriptParam::PARM_TYPE_STRING) return out;
	int id = vParams[0].getInt();
	std::string key = vParams[1].getString();
	SGTGOCAI *ai = SGTAIManager::Instance().GetAIByID(id);
	if (ai) ai->SetProperty(key, vParams[2]);
	return out;
}
std::vector<SGTScriptParam> SGTSceneManager::Lua_Npc_GetProperty(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> out;
	if (vParams.size() < 2) return out;
	if (!vParams[0].hasInt()) return out;
	if (vParams[1].getType() != SGTScriptParam::PARM_TYPE_STRING) return out;
	int id = vParams[0].getInt();
	std::string key = vParams[1].getString();
	SGTGOCAI *ai = SGTAIManager::Instance().GetAIByID(id);
	SGTScriptParam p(0);
	if (ai) p = ai->GetProperty(key);
	out.push_back(p);
	return out;
}

std::vector<SGTScriptParam> SGTSceneManager::Lua_Npc_AddState(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> out;
	return out;
}

std::vector<SGTScriptParam> SGTSceneManager::Lua_Npc_AddTA(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> out;
	std::vector<SGTScriptParam> ref;
	std::string sdummy;
	float fdummy = 0;
	ref.push_back(SGTScriptParam(fdummy));
	ref.push_back(SGTScriptParam(sdummy));
	ref.push_back(SGTScriptParam(fdummy));
	ref.push_back(SGTScriptParam(fdummy));
	Ogre::String param_test = SGTUtils::TestParameters(vParams, ref, true);
	if (param_test == "")
	{
		int id = vParams[0].getInt();
		std::string ta_script = vParams[1].getString();
		int end_timeH = vParams[2].getInt();
		int end_timeM = vParams[3].getInt();
		bool time_abs = true;
		std::vector<SGTScriptParam> miscparams;
		std::vector<SGTScriptParam>::iterator i = vParams.begin();
		i++;i++;i++;i++;
		for (; i != vParams.end(); i++)
		{
			miscparams.push_back((*i));
		}
	
	/*if (vParams.size() == 6)
	{
		if (vParams[5].getType() == SGTScriptParam::PARM_TYPE_BOOL) time_abs = vParams[5].getBool();
	}*/

		SGTGOCAI *ai = SGTAIManager::Instance().GetAIByID(id);
		if (ai) ai->AddScriptedState(new SGTDayCycle(ai, ta_script, miscparams, end_timeH, end_timeM, time_abs));
	}
	else
	{
		Ogre::String msg = "[Script] Error in \"" + caller.GetScriptName() + "\": " + param_test;
		Ogre::LogManager::getSingleton().logMessage(msg);
	}
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
	if (ai) ai->AddState(new SGTFollowPathway(ai, wp));
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

std::vector<SGTScriptParam> SGTSceneManager::Lua_CreatePlayer(SGTScript& caller, std::vector<SGTScriptParam> vParams)
{
	std::vector<SGTScriptParam> out;
	std::vector<SGTScriptParam> ref;
	std::string dummy;
	ref.push_back(SGTScriptParam(dummy));
	Ogre::String test = SGTUtils::TestParameters(vParams, ref, false);
	if (test == "")
	{
		Ogre::String model = vParams[0].getString();
		SGTGameObject* player = new SGTGameObject();
		player->AddComponent(new SGTGOCPlayerInput());
		player->AddComponent(new SGTGOCCameraController(SGTMain::Instance().GetCamera()));
		player->AddComponent(new SGTGOCCharacterController(Ogre::Vector3(0.5,1.8,0.5)));
		SGTGOCAnimatedCharacter *animated = new SGTGOCAnimatedCharacter(model);
		player->AddComponent(animated);
		player->SetGlobalPosition(Ogre::Vector3(0,10,0));
	}
	else Ogre::LogManager::getSingleton().logMessage(test);
	return out;
}


void SGTSceneManager::ReceiveMessage(SGTMsg &msg)
{
	if (msg.mNewsgroup == "UPDATE_PER_FRAME")
	{
		float time = msg.mData.GetFloat("TIME");
		if (mClockEnabled)
		{
			mDayTime += (time*0.001*mTimeScale);
			if (mDayTime >= mMaxDayTime) mDayTime = 0.0f;
			SGTAIManager::Instance().Update(time);
			if (mWeatherController) mWeatherController->Update(time);
		}
	}
}

void SGTSceneManager::EnableClock(bool enable)
{
	mClockEnabled = enable;
	/*if (enable)
	{
		SGTAIManager::Instance().mMsgPaused = !enable;
		mWeatherController->mMsgPaused = !enable;
	}*/
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


Ogre::TexturePtr SGTSceneManager::CreatePreviewRender(Ogre::SceneNode *node, Ogre::String name, float width, float height)
{
	Ogre::Camera *camera = node->getCreator()->createCamera(name + "_Cam");
	camera->setNearClipDistance(0.1f);
	camera->setFarClipDistance(0);
	camera->setPosition(mNextPreviewPos);
	camera->setDirection(0,0,1);
	camera->setAspectRatio(width / height);
	Ogre::Vector3 node_dimensions = Ogre::Vector3(1,1,1);
	Ogre::Vector3 center = Ogre::Vector3(0,0,0);
	if (node->getAttachedObject(0))
	{
		if (!node->getAttachedObject(0)->getBoundingBox().isNull())
		{
			node_dimensions = node->getAttachedObject(0)->getBoundingBox().getSize();
			center = node->getAttachedObject(0)->getBoundingBox().getCenter();
		}
	}
	float max = node_dimensions.x > node_dimensions.y ? node_dimensions.x : node_dimensions.y;
	max = max > node_dimensions.z ? max : node_dimensions.z;
	float scale_factor = 1.0f / max;
	float z_offset = (1.0f - (node_dimensions.z * scale_factor)) * 0.5f;
	node->scale(scale_factor, scale_factor, scale_factor);
	node->setPosition(mNextPreviewPos + Ogre::Vector3(0, 0, 1.9f-z_offset) + (scale_factor * center * -1.0f));
	mNextPreviewPos.x += node_dimensions.x * 2;
	Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().createManual(name + "_Tex", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, width, height, 0, Ogre::PF_R8G8B8, Ogre::TU_RENDERTARGET); 
    Ogre::RenderTexture *renderTexture = texture->getBuffer()->getRenderTarget();
    renderTexture->addViewport(camera);
	renderTexture->setAutoUpdated(true);
    renderTexture->getViewport(0)->setClearEveryFrame(true);
	renderTexture->getViewport(0)->setBackgroundColour(Ogre::ColourValue::Black);
    renderTexture->getViewport(0)->setOverlaysEnabled(false);
	if (width == 256 && height == 256) Ogre::CompositorManager::getSingleton().addCompositor(renderTexture->getViewport(0), "AntiAlias_256")->setEnabled(true);
	
	/*Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create(name + "_Mat", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    Ogre::Technique *technique = material->createTechnique();
    technique->createPass();
    material->getTechnique(0)->getPass(0)->setLightingEnabled(false);
    material->getTechnique(0)->getPass(0)->createTextureUnitState(name + "_Tex");*/
	return texture;
}
void SGTSceneManager::DestroyPreviewRender(Ogre::String name)
{
	if (Ogre::TextureManager::getSingleton().resourceExists(name + "_Tex"))
	{
		Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName(name + "_Tex");
		Ogre::Viewport *vp = texture->getBuffer()->getRenderTarget()->getViewport(0);
		vp->getCamera()->getSceneManager()->destroyCamera(vp->getCamera());
		texture->getBuffer()->getRenderTarget()->removeAllViewports();
		Ogre::TextureManager::getSingleton().remove(name + "_Tex");
	}
	//if (Ogre::MaterialManager::getSingleton().resourceExists(name + "_Mat"))	Ogre::MaterialManager::getSingleton().remove(name + "_Mat");
}

std::vector<SGTScriptParam> SGTSceneManager::Lua_GetFocusObject(SGTScript& caller, std::vector<SGTScriptParam> params)
{
	OgrePhysX::Scene::QueryHit hit;
	float maxDist = 5;
	std::string s = "";
	Ogre::Vector3 origin = SGTMain::Instance().GetCamera()->getDerivedPosition();
	Ogre::Vector3 dir = SGTMain::Instance().GetCamera()->getDerivedDirection().normalisedCopy();
	if (SGTMain::Instance().GetPhysXScene()->raycastClosestShape(hit, Ogre::Ray(origin, dir), NX_ALL_SHAPES, -1, maxDist))
	{
		SGTGameObject *object = (SGTGameObject*)hit.hitActor->userData;
		if (object)
		{
			if (1)			//Todo: if (object->GetComponent("Item"))
			{
				s = object->GetName().c_str();
				//...
			}
		}
	}
	std::vector<SGTScriptParam> returner;
	returner.push_back(SGTScriptParam(s));
	return returner;
}

SGTSceneManager& SGTSceneManager::Instance()
{
	static SGTSceneManager TheOneAndOnly;
	return TheOneAndOnly;
};