
#include "IceSceneManager.h"
#include "IceMain.h"
#include "LoadSave.h"
#include "IceWeatherController.h"
#include "Saveable.h"
#include "IceLevelMesh.h"
#include "shellapi.h"
#include "IceScriptSystem.h"
#include "standard_atoms.h"
#include "IceGOCPhysics.h"
#include "IceGOCView.h"
#include "IceGOCPlayerInput.h"
#include "IceGOCAnimatedCharacter.h"
#include "IceGOCIntern.h"
#include "IceGOCAI.h"
#include "IceAIManager.h"
#include "IceFollowPathway.h"
#include "IceLevelMesh.h"
#include "IcePathfinder.h"

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"

#include "IceUtils.h"

namespace Ice
{

SceneManager::SceneManager(void)
{
	mWeatherController = 0;
	mIndoorRendering = false;
	mClockEnabled = true;
	mLevelMesh = 0;
	mNextID = 0;
	mDayTime = 0.0f;
	mMaxDayTime = 86400.0f;
	mTimeScale = 64.0f;
	MessageSystem::Instance().CreateNewsgroup("ENABLE_GAME_CLOCK");
	MessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
}

SceneManager::~SceneManager(void)
{
	MessageSystem::Instance().QuitNewsgroup(this, "UPDATE_PER_FRAME");
}

unsigned int SceneManager::RequestID()
{
	mNextID++;
	return (mNextID-1);
}

Ogre::String SceneManager::RequestIDStr()
{
	return Ogre::StringConverter::toString(RequestID());
}

void SceneManager::RegisterPlayer(GameObject *player)
{
	mPlayer = player;
}

void SceneManager::UpdateGameObjects()
{
	for (std::vector<GameObject*>::iterator i = mObjectMessageQueue.begin(); i != mObjectMessageQueue.end(); i++)
	{
		(*i)->ProcessMessages();
	}
	mObjectMessageQueue.clear();
}

void SceneManager::RegisterEditorInterface(Ogre::String family, Ogre::String type, EDTCreatorFn RegisterFn, GOCDefaultParametersFn DefaulParametersFn)
{
	mEditorInterfaces.insert(std::make_pair<Ogre::String, EDTCreatorFn>(type, RegisterFn));
	if (family != "") RegisterComponentDefaultParameters(family, type, DefaulParametersFn);
}

void SceneManager::RegisterComponentDefaultParameters(Ogre::String family, Ogre::String type, GOCDefaultParametersFn RegisterFn)
{
	DataMap *parameters = new DataMap();
	RegisterFn(parameters);
	std::map<Ogre::String, std::map<Ogre::String, DataMap*> >::iterator i = mGOCDefaultParameters.find(family);
	if (i == mGOCDefaultParameters.end())
	{
		std::map<Ogre::String, DataMap*> map;
		map.insert(std::make_pair<Ogre::String, DataMap*>(type, parameters));
		mGOCDefaultParameters.insert(std::make_pair<Ogre::String, std::map<Ogre::String, DataMap*> >(family, map));
	}
	else
	{
		(*i).second.insert(std::make_pair<Ogre::String, DataMap*>(type, parameters));
	}
}

void SceneManager::ShowEditorMeshes(bool show)
{
	for (std::map<int, GameObject*>::iterator i = mGameObjects.begin(); i != mGameObjects.end(); i++)
	{
		if (show)
		{
			if (i->second->GetComponent("Waypoint") && !(i->second->GetComponent("MeshDebugRenderable")))
			{
				i->second->AddComponent(new MeshDebugRenderable("Editor_Waypoint.mesh"));
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

GOCEditorInterface* SceneManager::CreateComponent(Ogre::String type, DataMap *parameters)
{
	std::map<Ogre::String, EDTCreatorFn>::iterator i = mEditorInterfaces.find(type);
	GOCEditorInterface *goc = 0;
	if (i != mEditorInterfaces.end())
	{
		goc = (*i).second();
		goc->CreateFromDataMap(parameters);
		return goc;
	}
	Ogre::LogManager::getSingleton().logMessage("WARNING: SceneManager::CreateComponent - Can't find \"" + type + "\". Returning NULL.");
	return goc;
}

WeatherController* SceneManager::GetWeatherController()
{
	return mWeatherController;
}

void SceneManager::Init()
{
	Reset();

	Main::Instance().GetOgreSceneMgr()->createStaticGeometry("StaticGeometry");

	RegisterStandardAtoms();

	DataMap params;
	LoadSave::LoadSave::Instance().RegisterObject(&DataMap::Register);
	LoadSave::LoadSave::Instance().RegisterObject(&GenericProperty::Register);
	LoadSave::LoadSave::Instance().RegisterObject(&ComponentSection::Register);
	LoadSave::LoadSave::Instance().RegisterObject(&GameObject::Register);
	LoadSave::LoadSave::Instance().RegisterObject(&LoadSave::SaveableDummy::Register);
	LoadSave::LoadSave::Instance().RegisterObject(&GOCWaypoint::Register);

	LoadSave::LoadSave::Instance().RegisterObject(&MeshRenderable::Register);
	LoadSave::LoadSave::Instance().RegisterObject(&PfxRenderable::Register);
	LoadSave::LoadSave::Instance().RegisterObject(&LocalLightRenderable::Register);
	LoadSave::LoadSave::Instance().RegisterObject(&Sound3D::Register);
	LoadSave::LoadSave::Instance().RegisterObject(&GOCRigidBody::Register);
	LoadSave::LoadSave::Instance().RegisterObject(&GOCStaticBody::Register);
	LoadSave::LoadSave::Instance().RegisterObject(&GOCViewContainer::Register);
	LoadSave::LoadSave::Instance().RegisterObject(&GOCAnimatedCharacter::Register);
	LoadSave::LoadSave::Instance().RegisterObject(&GOCAnimatedCharacterBone::Register);
	LoadSave::LoadSave::Instance().RegisterObject(&GOCCharacterController::Register);
	LoadSave::LoadSave::Instance().RegisterObject(&GOCPlayerInput::Register);

	LoadSave::LoadSave::Instance().RegisterObject(&GOCAI::Register);

	RegisterEditorInterface("A", "Mesh", (EDTCreatorFn)&MeshRenderable::NewEditorInterfaceInstance, MeshRenderable::GetDefaultParameters);
	RegisterEditorInterface("A", "PFX", (EDTCreatorFn)&PfxRenderable::NewEditorInterfaceInstance, PfxRenderable::GetDefaultParameters);
	RegisterEditorInterface("A", "Light", (EDTCreatorFn)&LocalLightRenderable::NewEditorInterfaceInstance, LocalLightRenderable::GetDefaultParameters);
	RegisterEditorInterface("A", "Sound3D", (EDTCreatorFn)&Sound3D::NewEditorInterfaceInstance, Sound3D::GetDefaultParameters);
	RegisterEditorInterface("A", "Skeleton", (EDTCreatorFn)&GOCAnimatedCharacter::NewEditorInterfaceInstance, GOCAnimatedCharacter::GetDefaultParameters);
	RegisterEditorInterface("", "AnimatedCharacterBone", (EDTCreatorFn)&GOCAnimatedCharacterBone::NewEditorInterfaceInstance, GOCAnimatedCharacterBone::GetDefaultParameters);

	RegisterEditorInterface("B_x", "Rigid Body", (EDTCreatorFn)&GOCRigidBody::NewEditorInterfaceInstance, GOCRigidBody::GetDefaultParameters);
	RegisterEditorInterface("B_x", "Static Body", (EDTCreatorFn)&GOCStaticBody::NewEditorInterfaceInstance, GOCStaticBody::GetDefaultParameters);
	RegisterEditorInterface("B_x", "Character", (EDTCreatorFn)&GOCCharacterController::NewEditorInterfaceInstance, GOCCharacterController::GetDefaultParameters);

	RegisterEditorInterface("C_x", "Scripted AI", (EDTCreatorFn)&GOCAI::NewEditorInterfaceInstance, GOCAI::GetDefaultParameters);
	RegisterEditorInterface("C_x", "Player Control", (EDTCreatorFn)&GOCPlayerInput::NewEditorInterfaceInstance, GOCPlayerInput::GetDefaultParameters);

	RegisterEditorInterface("D", "Camera", (EDTCreatorFn)&GOCCameraController::NewEditorInterfaceInstance, GOCCameraController::GetDefaultParameters);

	//Setup Lua Callback
	ScriptSystem::GetInstance().ShareCFunction("LogMessage", &SceneManager::Lua_LogMessage);
	ScriptSystem::GetInstance().ShareCFunction("LoadLevel", &SceneManager::Lua_LoadLevel);
	ScriptSystem::GetInstance().ShareCFunction("CreatePlayer", &SceneManager::Lua_CreatePlayer);
	ScriptSystem::GetInstance().ShareCFunction("CreateNpc", &SceneManager::Lua_CreateNpc);
	ScriptSystem::GetInstance().ShareCFunction("Npc_SetProperty", &SceneManager::Lua_Npc_SetProperty);
	ScriptSystem::GetInstance().ShareCFunction("Npc_GetProperty", &SceneManager::Lua_Npc_GetProperty);
	ScriptSystem::GetInstance().ShareCFunction("Npc_AddState", &SceneManager::Lua_Npc_AddState);
	ScriptSystem::GetInstance().ShareCFunction("Npc_AddTA", &SceneManager::Lua_Npc_AddTA);
	ScriptSystem::GetInstance().ShareCFunction("Npc_GotoWP", &SceneManager::Lua_Npc_GotoWP);
	ScriptSystem::GetInstance().ShareCFunction("Npc_GetDistToWP", &SceneManager::Lua_Npc_GetDistToWP);

	ScriptSystem::GetInstance().ShareCFunction("InsertMesh", &SceneManager::Lua_InsertMesh);
	ScriptSystem::GetInstance().ShareCFunction("SetObjectPosition", &SceneManager::Lua_SetObjectPosition);
	ScriptSystem::GetInstance().ShareCFunction("SetObjectOrientation", &SceneManager::Lua_SetObjectOrientation);
	ScriptSystem::GetInstance().ShareCFunction("SetObjectScale", &SceneManager::Lua_SetObjectScale);

	ScriptSystem::GetInstance().ShareCFunction("GetFocusObject", &SceneManager::Lua_GetFocusObject);

}

void SceneManager::ClearGameObjects()
{
	std::map<int, GameObject*>::iterator i = mGameObjects.begin();
	while (i != mGameObjects.end())
	{
		delete i->second;
		i = mGameObjects.begin();
	}
	mGameObjects.clear();
}

void SceneManager::Reset()
{
	SetToOutdoor();
}

void SceneManager::Shutdown()
{
	SetToIndoor();
}


void SceneManager::SetToIndoor()
{
	if (mWeatherController)
	{
		delete mWeatherController;
		mWeatherController = NULL;
	}
	mIndoorRendering = true;
}

void SceneManager::SetToOutdoor()
{
	/*Main::Instance().GetOgreSceneMgr()->setSkyBox(true,"Sky/ClubTropicana", 2000);
	Ogre::Light *Light = Main::Instance().GetOgreSceneMgr()->createLight("Light0");
	Light->setType(Ogre::Light::LT_DIRECTIONAL);
	Light->setDirection(0, -1, 0.9);
	Light->setDiffuseColour(1, 1, 1);
	Light->setSpecularColour(Ogre::ColourValue(1, 0.9, 0.6)/5);*/
	
	if (!mWeatherController) mWeatherController = new WeatherController();
	SetTimeScale(mTimeScale);
	mIndoorRendering = false;
}

bool SceneManager::HasLevelMesh()
{
	return (mLevelMesh == NULL ? false : true);
}

LevelMesh* SceneManager::GetLevelMesh()
{
	return mLevelMesh;
}


void SceneManager::LoadLevelMesh(Ogre::String meshname)
{
	if (mIndoorRendering)
	{
	}
	else
	{
		if (mLevelMesh) delete mLevelMesh;
		mLevelMesh = new LevelMesh(meshname);
	}
}

void SceneManager::LoadLevel(Ogre::String levelfile, bool load_dynamic)
{
	ClearGameObjects();
	mNextID = 0;

	LoadSave::LoadSystem *ls=LoadSave::LoadSave::Instance().LoadFile(levelfile);

	DataMap *levelparams = (DataMap*)ls->LoadObject();
	CreateFromDataMap(levelparams);

	std::list<LoadSave::Saveable*> objects;
	ls->LoadAtom("std::list<Saveable*>", &objects);
	//Objects call SceneManager::RegisterObject

	ls->CloseFile();
	delete ls;
}

void SceneManager::SaveLevel(Ogre::String levelfile)
{
	ShowEditorMeshes(false);
	LoadSave::SaveSystem *ss=LoadSave::LoadSave::Instance().CreateSaveFile(levelfile, levelfile + ".xml");
	DataMap map;
	GetParameters(&map);
	ss->SaveObject(&map, "LevelParams");
	std::list<LoadSave::Saveable*> objects;
	for (std::map<int, GameObject*>::iterator i = mGameObjects.begin(); i != mGameObjects.end(); i++)
	{
		objects.push_back(i->second);
	}
	ss->SaveAtom("std::list<Saveable*>", &objects, "Objects");
	ss->CloseFiles();
	delete ss;
}

void SceneManager::CreateFromDataMap(DataMap *parameters)
{
	Ogre::String levelmesh = parameters->GetOgreString("LevelMesh");
	if (levelmesh != "") LoadLevelMesh(levelmesh.c_str());
	bool indoor = parameters->GetBool("Indoor");
	if (indoor) SetToIndoor();
	else SetToOutdoor();
}

void SceneManager::GetParameters(DataMap *parameters)
{
	if (mLevelMesh) parameters->AddOgreString("LevelMesh", mLevelMesh->GetMeshFileName());
	else parameters->AddOgreString("LevelMesh", "");
	parameters->AddBool("Indoor", mIndoorRendering);
}

std::map<int, GameObject*>& SceneManager::GetGameObjects()
{
	return mGameObjects;
}

int SceneManager::RegisterObject(GameObject *object)
{
	int id = RequestID();
	mGameObjects.insert(std::make_pair<int, GameObject*>(id, object));
	return id;
}

void  SceneManager::UnregisterObject(int id)
{
	std::map<int, GameObject*>::iterator i = mGameObjects.find(id);
	if (i != mGameObjects.end()) mGameObjects.erase(i);
}

GameObject* SceneManager::GetObjectByInternID(int id)
{
	std::map<int, GameObject*>::iterator i = mGameObjects.find(id);
	if (i != mGameObjects.end()) return i->second;
	return 0;
}

void SceneManager::AddToMessageQueue(GameObject *object)
{
	mObjectMessageQueue.push_back(object);
}

std::vector<ScriptParam>
SceneManager::Lua_LoadLevel(Script& caller, std::vector<ScriptParam> vParams)
{
	std::vector<ScriptParam> out;
	if (vParams.size() == 1)
	{
		if (vParams[0].getType() == ScriptParam::PARM_TYPE_STRING)
		{
			SceneManager::Instance().LoadLevel(vParams[0].getString());
		}
	}
	return out;
}

std::vector<ScriptParam>
SceneManager::Lua_LogMessage(Script& caller, std::vector<ScriptParam> vParams)
{
	Ogre::String msg = "";
	for(unsigned int iArg=0; iArg<vParams.size(); iArg++)
	{
		if(vParams[iArg].getType()==ScriptParam::PARM_TYPE_STRING)
			msg = msg + vParams[iArg].getString().c_str();
		else if(vParams[iArg].getType()==ScriptParam::PARM_TYPE_BOOL)
			msg = msg + Ogre::StringConverter::toString(vParams[iArg].getBool());
		else if(vParams[iArg].getType()==ScriptParam::PARM_TYPE_FLOAT)
		{
			float val = static_cast<float>(vParams[iArg].getFloat());
			msg = msg + Ogre::StringConverter::toString(val);
		}
		else if(vParams[iArg].getType()==ScriptParam::PARM_TYPE_INT)
			msg = msg + Ogre::StringConverter::toString(vParams[iArg].getInt());
	}
	Ogre::LogManager::getSingleton().logMessage(msg);
	return std::vector<ScriptParam>();
}


std::vector<ScriptParam> SceneManager::Lua_Npc_SetProperty(Script& caller, std::vector<ScriptParam> vParams)
{
	std::vector<ScriptParam> out;
	if (vParams.size() < 3) return out;
	if (!vParams[0].hasInt()) return out;
	if (vParams[1].getType() != ScriptParam::PARM_TYPE_STRING) return out;
	int id = vParams[0].getInt();
	std::string key = vParams[1].getString();
	GOCAI *ai = AIManager::Instance().GetAIByID(id);
	if (ai) ai->SetProperty(key, vParams[2]);
	return out;
}
std::vector<ScriptParam> SceneManager::Lua_Npc_GetProperty(Script& caller, std::vector<ScriptParam> vParams)
{
	std::vector<ScriptParam> out;
	if (vParams.size() < 2) return out;
	if (!vParams[0].hasInt()) return out;
	if (vParams[1].getType() != ScriptParam::PARM_TYPE_STRING) return out;
	int id = vParams[0].getInt();
	std::string key = vParams[1].getString();
	GOCAI *ai = AIManager::Instance().GetAIByID(id);
	ScriptParam p(0);
	if (ai) p = ai->GetProperty(key);
	out.push_back(p);
	return out;
}

std::vector<ScriptParam> SceneManager::Lua_Npc_AddState(Script& caller, std::vector<ScriptParam> vParams)
{
	std::vector<ScriptParam> out;
	return out;
}

std::vector<ScriptParam> SceneManager::Lua_Npc_AddTA(Script& caller, std::vector<ScriptParam> vParams)
{
	std::vector<ScriptParam> out;
	std::vector<ScriptParam> ref;
	std::string sdummy;
	float fdummy = 0;
	ref.push_back(ScriptParam(fdummy));
	ref.push_back(ScriptParam(sdummy));
	ref.push_back(ScriptParam(fdummy));
	ref.push_back(ScriptParam(fdummy));
	Ogre::String param_test = Utils::TestParameters(vParams, ref, true);
	if (param_test == "")
	{
		int id = vParams[0].getInt();
		std::string ta_script = vParams[1].getString();
		int end_timeH = vParams[2].getInt();
		int end_timeM = vParams[3].getInt();
		bool time_abs = true;
		std::vector<ScriptParam> miscparams;
		std::vector<ScriptParam>::iterator i = vParams.begin();
		i++;i++;i++;i++;
		for (; i != vParams.end(); i++)
		{
			miscparams.push_back((*i));
		}
	
	/*if (vParams.size() == 6)
	{
		if (vParams[5].getType() == ScriptParam::PARM_TYPE_BOOL) time_abs = vParams[5].getBool();
	}*/

		GOCAI *ai = AIManager::Instance().GetAIByID(id);
		if (ai) ai->AddScriptedState(new DayCycle(ai, ta_script, miscparams, end_timeH, end_timeM, time_abs));
	}
	else
	{
		Ogre::String msg = "[Script] Error in \"" + caller.GetScriptName() + "\": " + param_test;
		Ogre::LogManager::getSingleton().logMessage(msg);
	}
	return out;
}
std::vector<ScriptParam> SceneManager::Lua_Npc_GotoWP(Script& caller, std::vector<ScriptParam> vParams)
{
	std::vector<ScriptParam> out;
	if (vParams.size() < 2) return out;
	if (!vParams[0].hasInt()) return out;
	if (vParams[1].getType() != ScriptParam::PARM_TYPE_STRING) return out;
	int id = vParams[0].getInt();
	std::string wp = vParams[1].getString();
	GOCAI *ai = AIManager::Instance().GetAIByID(id);
	if (ai) ai->AddState(new FollowPathway(ai, wp));
	return out;
}
std::vector<ScriptParam> SceneManager::Lua_Npc_GetDistToWP(Script& caller, std::vector<ScriptParam> vParams)
{
	std::vector<ScriptParam> out;
	if (vParams.size() < 2) return out;
	if (!vParams[0].hasInt()) return out;
	if (vParams[1].getType() != ScriptParam::PARM_TYPE_STRING) return out;
	int id = vParams[0].getInt();
	std::string wp = vParams[1].getString();
	GOCWaypoint *pWp = Pathfinder::Instance().GetWPByName(wp);
	GOCAI *ai = AIManager::Instance().GetAIByID(id);
	float returner = 0;
	if (pWp && ai)
	{
		if (pWp->GetOwner() && ai->GetOwner())
			returner = pWp->GetOwner()->GetGlobalPosition().distance(ai->GetOwner()->GetGlobalPosition());
	}
	out.push_back(returner);
	return out;
}

std::vector<ScriptParam> SceneManager::Lua_InsertMesh(Script& caller, std::vector<ScriptParam> vParams)
{
	std::vector<ScriptParam> out;
	if (vParams.size() != 3) return out;
	if (vParams[0].getType() != ScriptParam::PARM_TYPE_STRING) return out;
	Ogre::String mesh = vParams[0].getString().c_str();
	if (vParams[1].getType() != ScriptParam::PARM_TYPE_BOOL) return out;
	bool shadows = vParams[1].getBool();
	if (vParams[2].getType() != ScriptParam::PARM_TYPE_FLOAT) return out;
	int collision = (int)vParams[2].getFloat();

	GameObject *object = new GameObject();
	GOCViewContainer *container = new GOCViewContainer();
	container->AddItem(new MeshRenderable(mesh, shadows));
	object->AddComponent(container);
	if (collision == -1)
	{
		object->AddComponent(new GOCStaticBody(mesh));
	}
	else if (collision >= 0 && collision <= 3)
	{
		object->AddComponent(new GOCRigidBody(mesh, 10, collision));
	}
	out.push_back(ScriptParam(object->GetID()));
	return out;
}

std::vector<ScriptParam> SceneManager::Lua_SetObjectPosition(Script& caller, std::vector<ScriptParam> vParams)
{
	std::vector<ScriptParam> out;
	if (vParams.size() != 4) return out;
	if (vParams[0].getType() != ScriptParam::PARM_TYPE_FLOAT || vParams[1].getType() != ScriptParam::PARM_TYPE_FLOAT || vParams[2].getType() != ScriptParam::PARM_TYPE_FLOAT || vParams[3].getType() != ScriptParam::PARM_TYPE_FLOAT) return out;
	int id = (int)vParams[0].getFloat();
	GameObject *object = SceneManager::Instance().GetObjectByInternID(id);
	if (object)
	{
		float x = vParams[1].getFloat();
		float y = vParams[2].getFloat();
		float z = vParams[3].getFloat();
		object->SetGlobalPosition(Ogre::Vector3(x,y,z));
	}
	return out;
}
std::vector<ScriptParam>SceneManager::Lua_SetObjectOrientation(Script& caller, std::vector<ScriptParam> vParams)
{
	std::vector<ScriptParam> out;
	if (vParams.size() != 4) return out;
	if (vParams[0].getType() != ScriptParam::PARM_TYPE_FLOAT || vParams[1].getType() != ScriptParam::PARM_TYPE_FLOAT || vParams[2].getType() != ScriptParam::PARM_TYPE_FLOAT || vParams[3].getType() != ScriptParam::PARM_TYPE_FLOAT) return out;
	int id = (int)vParams[0].getFloat();
	GameObject *object = SceneManager::Instance().GetObjectByInternID(id);
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
std::vector<ScriptParam>SceneManager::Lua_SetObjectScale(Script& caller, std::vector<ScriptParam> vParams)
{
	std::vector<ScriptParam> out;
	if (vParams.size() != 4) return out;
	if (vParams[0].getType() != ScriptParam::PARM_TYPE_FLOAT || vParams[1].getType() != ScriptParam::PARM_TYPE_FLOAT || vParams[2].getType() != ScriptParam::PARM_TYPE_FLOAT || vParams[3].getType() != ScriptParam::PARM_TYPE_FLOAT) return out;
	int id = (int)vParams[0].getFloat();
	GameObject *object = SceneManager::Instance().GetObjectByInternID(id);
	if (object)
	{
		float x = vParams[1].getFloat();
		float y = vParams[2].getFloat();
		float z = vParams[3].getFloat();
		object->SetGlobalScale(Ogre::Vector3(x,y,z));
	}
	return out;
}

std::vector<ScriptParam>
SceneManager::Lua_CreateNpc(Script& caller, std::vector<ScriptParam> vParams)
{
	std::vector<ScriptParam> out;
	Ogre::Vector3 scale(1,1,1);
	Ogre::String mesh = "";
	int returnerID = -1;
	if (vParams.size() > 0)
	{
		if (vParams[0].getType() == ScriptParam::PARM_TYPE_STRING)
		{
			mesh = vParams[0].getString().c_str();
		}
	}
	if (vParams.size() == 4)
	{
		if (vParams[1].getType() == ScriptParam::PARM_TYPE_FLOAT)
		{
			scale.x = vParams[1].getFloat();
		}
		if (vParams[2].getType() == ScriptParam::PARM_TYPE_FLOAT)
		{
			scale.y = vParams[2].getFloat();
		}
		if (vParams[3].getType() == ScriptParam::PARM_TYPE_FLOAT)
		{
			scale.z = vParams[3].getFloat();
		}
	}
	if (mesh != "")
	{
		GameObject *go = new GameObject();
		//GOCAnimatedCharacter *body = new GOCAnimatedCharacter(mesh, scale);
		GOCViewContainer *body = new GOCViewContainer();
		body->AddItem(new MeshRenderable("cube.1m.mesh", true));
		GOCAI *ai = new GOCAI();
		go->AddComponent(ai);		//Brain
		go->AddComponent(body);		//Body
		returnerID = (int)ai->GetID();
	}
	out.push_back(ScriptParam(returnerID));
	return out;
}

std::vector<ScriptParam> SceneManager::Lua_CreatePlayer(Script& caller, std::vector<ScriptParam> vParams)
{
	std::vector<ScriptParam> out;
	std::vector<ScriptParam> ref;
	std::string dummy;
	ref.push_back(ScriptParam(dummy));
	Ogre::String test = Utils::TestParameters(vParams, ref, false);
	if (test == "")
	{
		Ogre::String model = vParams[0].getString();
		GameObject* player = new GameObject();
		player->AddComponent(new GOCPlayerInput());
		player->AddComponent(new GOCCameraController(Main::Instance().GetCamera()));
		player->AddComponent(new GOCCharacterController(Ogre::Vector3(0.5,1.8,0.5)));
		GOCAnimatedCharacter *animated = new GOCAnimatedCharacter(model);
		player->AddComponent(animated);
		player->SetGlobalPosition(Ogre::Vector3(0,10,0));
	}
	else Ogre::LogManager::getSingleton().logMessage(test);
	return out;
}


void SceneManager::ReceiveMessage(Msg &msg)
{
	if (msg.mNewsgroup == "UPDATE_PER_FRAME")
	{
		float time = msg.mData.GetFloat("TIME");
		if (mClockEnabled)
		{
			mDayTime += (time*0.001*mTimeScale);
			if (mDayTime >= mMaxDayTime) mDayTime = 0.0f;
			AIManager::Instance().Update(time);
			if (mWeatherController) mWeatherController->Update(time);
		}
	}
}

void SceneManager::EnableClock(bool enable)
{
	mClockEnabled = enable;
	Msg msg;
	msg.mNewsgroup = "ENABLE_GAME_CLOCK";
	msg.mData.AddBool("enable", enable);
	MessageSystem::Instance().SendInstantMessage(msg);
}

void SceneManager::SetTimeScale(float scale)
{
	mTimeScale = scale;
	if (mWeatherController) mWeatherController->SetSpeedFactor(scale);
}
void SceneManager::SetTime(int hours, int minutes)
{
	mDayTime = (float)(hours * 3600.0f + minutes * 60.0f);
}
int SceneManager::GetHour()
{
	return (int)(mDayTime / 3600);
}
int SceneManager::GetMinutes()
{
	int hour = GetHour();
	float fhour = 3600.0f * hour;
	float dif = mDayTime - fhour;
	return dif / 60;
}


Ogre::TexturePtr SceneManager::CreatePreviewRender(Ogre::SceneNode *node, Ogre::String name, float width, float height)
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
void SceneManager::DestroyPreviewRender(Ogre::String name)
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

std::vector<ScriptParam> SceneManager::Lua_GetFocusObject(Script& caller, std::vector<ScriptParam> params)
{
	OgrePhysX::Scene::QueryHit hit;
	float maxDist = 5;
	std::string s = "";
	Ogre::Vector3 origin = Main::Instance().GetCamera()->getDerivedPosition();
	Ogre::Vector3 dir = Main::Instance().GetCamera()->getDerivedDirection().normalisedCopy();
	if (Main::Instance().GetPhysXScene()->raycastClosestShape(hit, Ogre::Ray(origin, dir), NX_ALL_SHAPES, -1, maxDist))
	{
		GameObject *object = (GameObject*)hit.hitActor->userData;
		if (object)
		{
			if (1)			//Todo: if (object->GetComponent("Item"))
			{
				s = object->GetName().c_str();
				//...
			}
		}
	}
	std::vector<ScriptParam> returner;
	returner.push_back(ScriptParam(s));
	return returner;
}

SceneManager& SceneManager::Instance()
{
	static SceneManager TheOneAndOnly;
	return TheOneAndOnly;
};

};