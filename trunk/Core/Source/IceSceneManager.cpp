
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
#include "IceGOCCameraController.h"
#include "IceGOCAnimatedCharacter.h"
#include "IceGOCWaypoint.h"
#include "IceGOCAI.h"
#include "IceAIManager.h"
#include "IceFollowPathway.h"
#include "IceLevelMesh.h"
#include "IceGOCMover.h"
#include "IceGOCScript.h"
#include "IceGOCForceField.h"
#include "IceProcessNode.h"
#include "IceProcessNodeQueue.h"
#include "IceProcessNodeManager.h"

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"

#include "IceUtils.h"

#include "mmsystem.h"

#include "Caelum.h"

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
		mPlayer = 0;
		mMaxDayTime = 86400.0f;
		mTimeScale = 64.0f;
		mDestroyStoppedSoundsDelay = 0;
		mDestroyStoppedSoundsLast = 0;

		mShowEditorVisuals = false;

		mClearingScene = false;

		MessageSystem::Instance().CreateNewsgroup("ENABLE_GAME_CLOCK");
		MessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");

		Ice::MessageSystem::Instance().CreateNewsgroup("LOADLEVEL_BEGIN");
		Ice::MessageSystem::Instance().CreateNewsgroup("LOADLEVEL_END");
		Ice::MessageSystem::Instance().CreateNewsgroup("SAVELEVEL_BEGIN");
		Ice::MessageSystem::Instance().CreateNewsgroup("SAVELEVEL_END");

		Ice::MessageSystem::Instance().CreateNewsgroup("ACTOR_ONSLEEP");
		Ice::MessageSystem::Instance().CreateNewsgroup("ACTOR_ONWAKE");

		Ice::MessageSystem::Instance().CreateNewsgroup("MATERIAL_ONCONTACT");
	}

	SceneManager::~SceneManager(void)
	{
	}

	unsigned int SceneManager::RequestID()
	{
		return mNextID++;
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
		auto cpy = mObjectMessageQueue;
		mObjectMessageQueue.clear();
		for (unsigned int i = 0; i < cpy.size(); i++)
		{
			cpy[i]->ProcessMessages();
		}
	}

	void SceneManager::RegisterComponentDefaultParams(Ogre::String editFamily, Ogre::String type, DataMap &params)
	{
		std::map<Ogre::String, std::map<Ogre::String, DataMap> >::iterator i = mGOCDefaultParameters.find(editFamily);
		if (i == mGOCDefaultParameters.end())
		{
			std::map<Ogre::String, DataMap> map;
			map.insert(std::make_pair<Ogre::String, DataMap>(type, params));
			mGOCDefaultParameters.insert(std::make_pair<Ogre::String, std::map<Ogre::String, DataMap> >(editFamily, map));
		}
		else
		{
			(*i).second.insert(std::make_pair<Ogre::String, DataMap>(type, params));
		}
	}

	void SceneManager::RegisterGOCPrototype(GOCEditorInterfacePtr prototype)
	{
		mGOCPrototypes.insert(std::make_pair(prototype->GetLabel(), prototype));
	}
	void SceneManager::RegisterGOCPrototype(Ogre::String editFamily, GOCEditorInterfacePtr prototype)
	{
		mGOCPrototypes.insert(std::make_pair(prototype->GetLabel(), prototype));
		DataMap params;
		prototype->GetDefaultParameters(&params);
		RegisterComponentDefaultParams(editFamily, prototype->GetLabel(), params);
	}

	void SceneManager::ShowEditorMeshes(bool show)
	{
		mShowEditorVisuals = show;
		for (auto i = mGameObjects.begin(); i != mGameObjects.end(); i++)
		{
			i->second->ShowEditorVisuals(show);
		}
	}

	GOCEditorInterface* SceneManager::GetGOCPrototype(Ogre::String type)
	{
		auto i = mGOCPrototypes.find(type);
		if (i != mGOCPrototypes.end())
			return i->second.get();
		return nullptr;
	}

	GOCEditorInterface* SceneManager::NewGOC(Ogre::String type)
	{
		auto i = mGOCPrototypes.find(type);
		if (i != mGOCPrototypes.end())
		{
			GOCEditorInterface *goc = (*i).second->New();
			return goc;
		}
		IceAssert(false);
		return nullptr;
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
		LoadSave::LoadSave::Instance().SetLogFunction(LogMessage);

		LoadSave::LoadSave::Instance().RegisterObject(&DataMap::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&DataMap::Item::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GenericProperty::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GameObject::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&ManagedGameObject::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&LoadSave::SaveableDummy::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCWaypoint::Register);

		LoadSave::LoadSave::Instance().RegisterObject(&GOCMeshRenderable::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCPfxRenderable::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCLocalLightRenderable::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCSound3D::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCRigidBody::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCStaticBody::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCAnimatedCharacter::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCAnimatedCharacterBone::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCCharacterController::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCPlayerInput::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCCameraController::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCSimpleCameraController::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCScript::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCScriptMessageCallback::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCAI::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCForceField::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCBillboard::Register);

		LoadSave::LoadSave::Instance().RegisterObject(&GOCMover::Register);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCAnimKey::Register);

		LoadSave::LoadSave::Instance().RegisterObject(&NavigationMesh::Register);

		RegisterGOCPrototype("A", GOCEditorInterfacePtr(new GOCMeshRenderable()));
		RegisterGOCPrototype("A", GOCEditorInterfacePtr(new GOCPfxRenderable()));
		RegisterGOCPrototype("A", GOCEditorInterfacePtr(new GOCLocalLightRenderable()));
		RegisterGOCPrototype("A", GOCEditorInterfacePtr(new GOCSound3D()));
		RegisterGOCPrototype("A", GOCEditorInterfacePtr(new GOCAnimatedCharacter()));
		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCAnimatedCharacterBone()));
		RegisterGOCPrototype("A", GOCEditorInterfacePtr(new GOCBillboard()));

		RegisterGOCPrototype("B_x", GOCEditorInterfacePtr(new GOCRigidBody()));
		RegisterGOCPrototype("B_x", GOCEditorInterfacePtr(new GOCStaticBody()));
		RegisterGOCPrototype("B_x", GOCEditorInterfacePtr(new GOCCharacterController()));

		RegisterGOCPrototype("C_x", GOCEditorInterfacePtr(new GOCAI()));
		RegisterGOCPrototype("C_x", GOCEditorInterfacePtr(new GOCPlayerInput()));

		RegisterGOCPrototype("D_x", GOCEditorInterfacePtr(new GOCSimpleCameraController()));
		RegisterGOCPrototype("D_x", GOCEditorInterfacePtr(new GOCCameraController()));

		RegisterGOCPrototype("E", GOCEditorInterfacePtr(new GOCMover()));
		RegisterGOCPrototype("E", GOCEditorInterfacePtr(new GOCScript()));
		RegisterGOCPrototype("E", GOCEditorInterfacePtr(new GOCForceField()));
		RegisterGOCPrototype(GOCEditorInterfacePtr(new GOCAnimKey()));

		//simple test:
		/*GameObject *test = new GameObject();
		auto bla = std::make_shared<GOCMeshRenderable>();
		Ice::DataMap blaParams;
		blaParams.AddOgreString("MeshName", "cube.1m.mesh");
		blaParams.AddBool("ShadowCaster", true);
		(dynamic_cast<GOCEditorInterface*>(bla.get()))->SetParameters(&blaParams);
		test->AddComponent(bla);
		delete test;*/

		//Shared Lua functions

		/**
		Joins a newsgroup.
		Example usage: Listen("UPDATE_PER_FRAME", myFunc)
		*/
		ScriptSystem::GetInstance().ShareCFunction("ReceiveGlobalMessage", &ScriptSystem::Lua_JoinNewsgroup);

		/**
		Logs a message to the log.
		Example usage: LogMessage("Everything is okay!")
		*/
		ScriptSystem::GetInstance().ShareCFunction("LogMessage", &SceneManager::Lua_LogMessage);

		/**
		Retrieves the object id of the script instance.
		@pre: Script is an object script.
		Example usage: id = This()
		*/
		ScriptSystem::GetInstance().ShareCFunction("This", &SceneManager::Lua_GetThis);

		ScriptSystem::GetInstance().ShareCFunction("GetObjectIDByName", &SceneManager::Lua_GetObjectByName);

		/**
		Loads a level.
		Example usage: LoadLevel("World.eew")
		*/
		ScriptSystem::GetInstance().ShareCFunction("LoadLevel", &SceneManager::Lua_LoadLevel);
		ScriptSystem::GetInstance().ShareCFunction("SaveLevel", &SceneManager::Lua_SaveLevel);

		/**
		Object get/set methods.
		Example usage: SetPosition(id, 1.0, 2.5, 3.1)
		*/
		ScriptSystem::GetInstance().ShareCFunction("Object_SetProperty", &GameObject::Lua_SetObjectProperty);
		ScriptSystem::GetInstance().ShareCFunction("Object_GetProperty", &GameObject::Lua_GetObjectProperty);
		ScriptSystem::GetInstance().ShareCFunction("Object_HasProperty", &GameObject::Lua_HasObjectProperty);

		ScriptSystem::GetInstance().ShareCFunction("Object_SetPosition", &GameObject::Lua_SetObjectPosition);
		ScriptSystem::GetInstance().ShareCFunction("Object_SetOrientation", &GameObject::Lua_SetObjectOrientation);
		ScriptSystem::GetInstance().ShareCFunction("Object_SetScale", &GameObject::Lua_SetObjectScale);
		ScriptSystem::GetInstance().ShareCFunction("Object_GetName", &GameObject::Lua_GetObjectName);
		ScriptSystem::GetInstance().ShareCFunction("Object_HasScriptListener", &GameObject::Lua_HasScriptListener);
		ScriptSystem::GetInstance().ShareCFunction("Object_GetChild", &GameObject::Lua_GetChildObjectByName);
		ScriptSystem::GetInstance().ShareCFunction("Object_GetParent", &GameObject::Lua_GetParent);
		ScriptSystem::GetInstance().ShareCFunction("Object_IsNpc", &GameObject::Lua_IsNpc);
		ScriptSystem::GetInstance().ShareCFunction("Object_Play3DSound", &GameObject::Lua_Object_Play3DSound);
		ScriptSystem::GetInstance().ShareCFunction("Object_GetDistToObject", &GameObject::Lua_Object_GetDistToObject);

		ScriptSystem::GetInstance().ShareCFunction("Object_ReceiveMessage", &GameObject::Lua_ReceiveObjectMessage);
		ScriptSystem::GetInstance().ShareCFunction("Object_SendMessage", &GameObject::Lua_SendObjectMessage);


		//Not implemented
		ScriptSystem::GetInstance().ShareCFunction("Npc_AddState", &GOCAI::Lua_Npc_AddState);

		/**
		Terminates the active state.
		*/
		ScriptSystem::GetInstance().ShareCFunction("Npc_KillActiveState", &GOCAI::Lua_Npc_KillActiveState);

		/**
		Terminates all states in the queue.
		*/
		ScriptSystem::GetInstance().ShareCFunction("Npc_ClearQueue", &GOCAI::Lua_Npc_ClearQueue);

		/**
		Adds a scripted state to the daily routine queue.
		*/
		ScriptSystem::GetInstance().ShareCFunction("Npc_AddTA", &GOCAI::Lua_Npc_AddTA);

		/**
		Tells the npc to go to a certain waypoint.
		*/
		ScriptSystem::GetInstance().ShareCFunction("Npc_GotoWP", &GOCAI::Lua_Npc_GotoWP);
		ScriptSystem::GetInstance().ShareCFunction("Npc_OpenDialog", &GOCAI::Lua_Npc_OpenDialog);

		ScriptSystem::GetInstance().ShareCFunction("GetPlayer", &SceneManager::Lua_GetPlayer);

		ScriptSystem::GetInstance().ShareCFunction("Forcefield_SetActive", &GOCForceField::Lua_Forcefield_Activate);

		ScriptSystem::GetInstance().ShareCFunction("Character_GetGroundMaterial", &GOCCharacterController::Lua_Character_GetGroundMaterial);
		ScriptSystem::GetInstance().ShareCFunction("Character_SetSpeedFactor", &GOCCharacterController::Lua_Character_SetSpeed);

		ScriptSystem::GetInstance().ShareCFunction("PFX_SetEmitting", &GOCPfxRenderable::Lua_SetEmitting);

		ScriptSystem::GetInstance().ShareCFunction("Sound3D_StartFade", &GOCSound3D::Lua_StartFade);

		ScriptSystem::GetInstance().ShareCFunction("Object_RunFunction", &GOCScript::Lua_RunFunction);

		ScriptSystem::GetInstance().ShareCFunction("AnimProcess_Create", &GOCAnimatedCharacter::Lua_AnimProcess_Create);
		ScriptSystem::GetInstance().ShareCFunction("Process_AddDependency", &ProcessNode::Lua_AddDependency);
		ScriptSystem::GetInstance().ShareCFunction("Process_Kill", &ProcessNode::Lua_KillProcess);
		ScriptSystem::GetInstance().ShareCFunction("Process_SetActive", &ProcessNode::Lua_SetActive);
		ScriptSystem::GetInstance().ShareCFunction("Process_FireFinish", &ProcessNode::Lua_TriggerWaiting);
		ScriptSystem::GetInstance().ShareCFunction("ProcessQueue_Create", &ProcessNodeQueue::Lua_ProcessQueue_Create);
		ScriptSystem::GetInstance().ShareCFunction("ProcessQueue_Enqueue", &ProcessNodeQueue::Lua_ProcessQueue_Enqueue);
		ScriptSystem::GetInstance().ShareCFunction("ProcessQueue_Push", &ProcessNodeQueue::Lua_ProcessQueue_PushFront);

		ScriptSystem::GetInstance().ShareCFunction("TimerProcess_Create", &ProcessNodeManager::Lua_ProcessTimer_Create);

		/**
		Triggers a mover.
		*/
		ScriptSystem::GetInstance().ShareCFunction("Mover_Trigger", &GOCMover::Lua_TriggerMover);
		ScriptSystem::GetInstance().ShareCFunction("Mover_Pause", &GOCMover::Lua_PauseMover);
		ScriptSystem::GetInstance().ShareCFunction("Mover_Stop", &GOCMover::Lua_StopMover);

		//Physical Body
		ScriptSystem::GetInstance().ShareCFunction("Body_GetSpeed", &GOCPhysics::Lua_Body_GetSpeed);
		ScriptSystem::GetInstance().ShareCFunction("Body_AddImpulse", &GOCPhysics::Lua_Body_AddImpulse);

		//Time get/set methods
		ScriptSystem::GetInstance().ShareCFunction("GetGameTimeHour", &SceneManager::Lua_GetGameTimeHour);
		ScriptSystem::GetInstance().ShareCFunction("GetGameTimeMinutes", &SceneManager::Lua_GetGameTimeMinutes);
		ScriptSystem::GetInstance().ShareCFunction("SetGameTime", &SceneManager::Lua_SetGameTime);
		ScriptSystem::GetInstance().ShareCFunction("SetGameTimeScale", &SceneManager::Lua_SetGameTimeScale);

		ScriptSystem::GetInstance().ShareCFunction("Play3DSound", &SceneManager::Lua_Play3DSound);
		ScriptSystem::GetInstance().ShareCFunction("CreateMaterialProfile", &SceneManager::Lua_CreateMaterialProfile);

		ScriptSystem::GetInstance().ShareCFunction("GetFocusObject", &SceneManager::Lua_GetFocusObject);

		ScriptSystem::GetInstance().ShareCFunction("Node_SetVisible", &GOCOgreNode::Lua_SetVisible);

		ScriptSystem::GetInstance().ShareCFunction("ConcatToString", &SceneManager::Lua_ConcatToString);

		ScriptSystem::GetInstance().ShareCFunction("GetRandomNumber", &SceneManager::Lua_GetRandomNumber);

	}

	void SceneManager::PostInit()
	{
		mSoundMaterialTable.InitBindingsFromCfg("OgreMaterialSoundBindings.cfg");
	}

	void SceneManager::ClearGameObjects()
	{
		mClearingScene = true;
		std::map<int, ManagedGameObject*>::iterator i = mGameObjects.begin();
		while (i != mGameObjects.end())
		{
			delete i->second;
			i = mGameObjects.begin();
		}
		mGameObjects.clear();
		mClearingScene = false;
	}

	void SceneManager::Reset()
	{
		DestroyStoppedSounds();
		ClearGameObjects();
		if (mLevelMesh)
		{
			ICE_DELETE mLevelMesh;
			mLevelMesh = nullptr;
		}
		SetToOutdoor();
	}

	void SceneManager::Shutdown()
	{
		DestroyStoppedSounds();

		SetToIndoor();

		mGOCPrototypes.clear();

		mGOCDefaultParameters.clear();

		AIManager::Instance().Shutdown();
	}


	void SceneManager::SetToIndoor()
	{
		if (mWeatherController)
		{
			ICE_DELETE mWeatherController;
			mWeatherController = NULL;
		}
		mIndoorRendering = true;
	}

	void SceneManager::SetToOutdoor()
	{
		/*Main::Instance().GetOgreSceneMgr()->setSkyBox(true, "Skybox/LostValley");
		Main::Instance().GetOgreSceneMgr()->getSkyBoxNode()->getAttachedObject(0)->setVisibilityFlags( Ice::VisibilityFlags::V_SKY);
		Ogre::Light *Light = Main::Instance().GetOgreSceneMgr()->createLight("Light0");
		Light->setType(Ogre::Light::LT_DIRECTIONAL);
		Light->setDirection(0, -1, 0.9);
		Light->setDiffuseColour(2, 2, 2);
		Light->setSpecularColour(Ogre::ColourValue(1, 0.9, 0.6));*/
		
		if (!mWeatherController) mWeatherController = ICE_NEW WeatherController();
		SetTimeScale(mTimeScale);
		SetTime(11, 0);
		mIndoorRendering = false;
	}

	bool SceneManager::HasLevelMesh()
	{
		return (mLevelMesh == nullptr ? false : true);
	}

	LevelMesh* SceneManager::GetLevelMesh()
	{
		return mLevelMesh;
	}


	void SceneManager::LoadLevelMesh(Ogre::String meshname)
	{
		if (mLevelMesh)
		{
			if (mLevelMesh->GetMeshFileName() == meshname) return;
		}

		if (mLevelMesh) ICE_DELETE mLevelMesh;
		mLevelMesh = ICE_NEW LevelMesh(meshname);
	}

	void SceneManager::LoadLevel(Ogre::String levelfile, bool load_dynamic)
	{
		Msg msg;
		msg.type = "LOADLEVEL_BEGIN";
		MessageSystem::Instance().SendInstantMessage(msg);

		ClearGameObjects();

		LoadSave::LoadSystem *ls=LoadSave::LoadSave::Instance().LoadFile(levelfile);

		DataMap *levelparams = (DataMap*)ls->LoadObject();
		SetParameters(levelparams);

		std::vector<ManagedGameObject*> objects;
		ls->LoadAtom("std::vector<Saveable*>", &objects);
		//Objects call SceneManager::RegisterObject

		ls->CloseFile();
		ICE_DELETE ls;

		msg.type = "LOADLEVEL_END";
		MessageSystem::Instance().SendInstantMessage(msg);
	}

	void SceneManager::SaveLevel(Ogre::String levelfile)
	{
		Msg msg;
		msg.type = "SAVELEVEL_BEGIN";
		MessageSystem::Instance().SendInstantMessage(msg);

		ShowEditorMeshes(false);
		LoadSave::SaveSystem *ss=LoadSave::LoadSave::Instance().CreateSaveFile(levelfile, levelfile + ".xml");
		DataMap map;
		GetParameters(&map);
		ss->SaveObject(&map, "LevelParams");
		std::vector<LoadSave::Saveable*> objects;
		for (std::map<int, ManagedGameObject*>::iterator i = mGameObjects.begin(); i != mGameObjects.end(); i++)
		{
			objects.push_back(i->second);
		}
		ss->SaveAtom("std::vector<Saveable*>", &objects, "Objects");
		//ss->SaveObject(AIManager::Instance().GetNavigationMesh(), "WayMesh");
		ss->CloseFiles();
		ICE_DELETE ss;

		msg.type = "SAVELEVEL_END";
		MessageSystem::Instance().SendInstantMessage(msg);
	}

	void SceneManager::SetParameters(DataMap *parameters)
	{
		Ogre::String levelmesh = parameters->GetOgreString("LevelMesh");
		if (levelmesh != "")
		{
			LoadLevelMesh(levelmesh.c_str());
			//AIManager::Instance().SetNavigationMesh((NavigationMesh*)ls->LoadObject());
			Ice::AIManager::Instance().GetNavigationMesh()->ImportOgreMesh(Ice::SceneManager::Instance().GetLevelMesh()->GetEntity()->getMesh());
		}
		else if (mLevelMesh)
		{
			ICE_DELETE mLevelMesh;
			mLevelMesh = nullptr;
		}

		mStartupScriptName = parameters->GetValue<Ogre::String>("Startup Script", "");
		if (mStartupScriptName != "") ScriptSystem::GetInstance().CreateInstance(mStartupScriptName);

		bool indoor = parameters->GetBool("Indoor");
		if (indoor) SetToIndoor();
		else
		{
			SetToOutdoor();
			mWeatherController->GetCaelumSystem()->getSun()->setAmbientMultiplier(parameters->GetOgreCol("AmbientLight"));
			mWeatherController->GetCaelumSystem()->getSun()->setDiffuseMultiplier(parameters->GetOgreCol("Sun_DiffuseLight"));
			mWeatherController->GetCaelumSystem()->getSun()->setSpecularMultiplier(parameters->GetOgreCol("Sun_SpecularLight"));
			mWeatherController->Update(0);
		}

		Ogre::GpuSharedParametersPtr hdrParams = Ogre::GpuProgramManager::getSingleton().getSharedParameters("HDRParams");
		/*hdrParams->removeAllConstantDefinitions();
		hdrParams->addConstantDefinition("Luminence_Factor", Ogre::GpuConstantType::GCT_FLOAT4);
		hdrParams->addConstantDefinition("Tonemap_White", Ogre::GpuConstantType::GCT_FLOAT1);
		hdrParams->addConstantDefinition("Brightpass_Threshold", Ogre::GpuConstantType::GCT_FLOAT4);
		hdrParams->addConstantDefinition("LinearTonemap_KeyLumScale", Ogre::GpuConstantType::GCT_FLOAT1);
		hdrParams->addConstantDefinition("LinearTonemap_KeyMax", Ogre::GpuConstantType::GCT_FLOAT1);
		hdrParams->addConstantDefinition("LinearTonemap_KeyMaxOffset", Ogre::GpuConstantType::GCT_FLOAT1);
		hdrParams->addConstantDefinition("LinearTonemap_KeyMin", Ogre::GpuConstantType::GCT_FLOAT1);*/

		Ogre::ColourValue col = parameters->GetOgreCol("Luminence_Factor"); col.a = 0;
		hdrParams->setNamedConstant("Luminence_Factor", col);
		hdrParams->setNamedConstant("Tonemap_White", parameters->GetFloat("Tonemap_White"));
		col = parameters->GetOgreCol("Brightpass_Threshold"); col.a = 0;
		hdrParams->setNamedConstant("Brightpass_Threshold", col);

		try { col = parameters->GetOgreCol("BrightpassAmbient_Threshold");} catch(Ogre::Exception) {col = Ogre::ColourValue(1,1,1,0); }
		col.a = 0;hdrParams->setNamedConstant("BrightpassAmbient_Threshold", col);
		hdrParams->setNamedConstant("BloomAmbient_GlareScale", parameters->GetValue<float>("BloomAmbient_GlareScale", 0.5f));
		hdrParams->setNamedConstant("BloomAmbient_GlarePower", parameters->GetValue<float>("BloomAmbient_GlarePower", 0.5f));

		hdrParams->setNamedConstant("Bloom_GlareScale", parameters->GetValue<float>("Bloom_GlareScale", 1.0f));
		hdrParams->setNamedConstant("Bloom_GlarePower", parameters->GetValue<float>("Bloom_GlarePower", 1.0f));
		hdrParams->setNamedConstant("Bloom_StarScale", parameters->GetValue<float>("Bloom_StarScale", 1.0f));
		hdrParams->setNamedConstant("Bloom_StarPower", parameters->GetValue<float>("Bloom_StarPower", 1.0f));

		hdrParams->setNamedConstant("LinearTonemap_KeyLumScale", parameters->GetFloat("LinearTonemap_KeyLumScale"));
		hdrParams->setNamedConstant("LinearTonemap_KeyMax", parameters->GetFloat("LinearTonemap_KeyMax"));
		hdrParams->setNamedConstant("LinearTonemap_KeyMaxOffset", parameters->GetFloat("LinearTonemap_KeyMaxOffset"));
		hdrParams->setNamedConstant("LinearTonemap_KeyMin", parameters->GetFloat("LinearTonemap_KeyMin"));

		hdrParams->setNamedConstant("LightAdaption_Exponent", parameters->GetValue<float>("LightAdaption_Exponent", 2.0f));
		hdrParams->setNamedConstant("LightAdaption_Factor", parameters->GetValue<float>("LightAdaption_Factor", 0.1f));
		hdrParams->setNamedConstant("ShadowAdaption_Exponent", parameters->GetValue<float>("ShadowAdaption_Exponent", 2));
		hdrParams->setNamedConstant("ShadowAdaption_Factor", parameters->GetValue<float>("ShadowAdaption_Factor", 0.1f));
	}

	void SceneManager::GetParameters(DataMap *parameters)
	{
		if (mLevelMesh) parameters->AddOgreString("LevelMesh", mLevelMesh->GetMeshFileName());
		else parameters->AddOgreString("LevelMesh", "");
		parameters->AddBool("Indoor", mIndoorRendering);

		parameters->AddOgreString("Startup Script", mStartupScriptName);

		if (mWeatherController)
		{
			parameters->AddOgreCol("AmbientLight", mWeatherController->GetCaelumSystem()->getSun()->getAmbientMultiplier());
			parameters->AddOgreCol("Sun_DiffuseLight", mWeatherController->GetCaelumSystem()->getSun()->getDiffuseMultiplier());
			parameters->AddOgreCol("Sun_SpecularLight", mWeatherController->GetCaelumSystem()->getSun()->getSpecularMultiplier());
		}

		Ogre::GpuSharedParametersPtr hdrParams = Ogre::GpuProgramManager::getSingleton().getSharedParameters("HDRParams");
		float *buf = hdrParams->getFloatPointer(0);
		parameters->AddOgreVec3("Luminence_Factor", Ogre::Vector3(buf[0], buf[1], buf[2]));
		parameters->AddFloat("Tonemap_White", buf[4]);	//skip buf[3] (float4)
		parameters->AddOgreVec3("Brightpass_Threshold", Ogre::Vector3(buf[5], buf[6], buf[7]));

		parameters->AddOgreVec3("BrightpassAmbient_Threshold", Ogre::Vector3(buf[9], buf[10], buf[11]));

		parameters->AddFloat("BloomAmbient_GlareScale", buf[13]);
		parameters->AddFloat("BloomAmbient_GlarePower", buf[14]);

		parameters->AddFloat("Bloom_GlareScale", buf[15]);
		parameters->AddFloat("Bloom_GlarePower", buf[16]);
		parameters->AddFloat("Bloom_StarScale", buf[17]);
		parameters->AddFloat("Bloom_StarPower", buf[18]);

		parameters->AddFloat("LinearTonemap_KeyLumScale", buf[19]);
		parameters->AddFloat("LinearTonemap_KeyMax", buf[20]);
		parameters->AddFloat("LinearTonemap_KeyMaxOffset", buf[21]);
		parameters->AddFloat("LinearTonemap_KeyMin", buf[22]);

		parameters->AddFloat("LightAdaption_Exponent", buf[23]);
		parameters->AddFloat("LightAdaption_Factor", buf[24]);
		parameters->AddFloat("ShadowAdaption_Exponent", buf[25]);
		parameters->AddFloat("ShadowAdaption_Factor", buf[26]);
	}

	std::map<int, ManagedGameObject*>& SceneManager::GetGameObjects()
	{
		return mGameObjects;
	}

	void  SceneManager::NotifyGODelete(ManagedGameObject *object)
	{
		std::map<int, ManagedGameObject*>::iterator i = mGameObjects.find(object->GetID());
		if (i != mGameObjects.end()) mGameObjects.erase(i);
	}
	int SceneManager::RegisterObject(ManagedGameObject *object)
	{
		int id = RequestID();
		mGameObjects.insert(std::make_pair<int, ManagedGameObject*>(id, object));
		return id;
	}
	void SceneManager::RemoveGameObject(ManagedGameObject *object)
	{
		ICE_DELETE object;
	}
	ManagedGameObject* SceneManager::CreateGameObject()
	{
		return ICE_NEW ManagedGameObject();
	}

	ManagedGameObject* SceneManager::GetObjectByInternID(int id)
	{
		std::map<int, ManagedGameObject*>::iterator i = mGameObjects.find(id);
		if (i != mGameObjects.end()) return i->second;
		return 0;
	}

	void SceneManager::AddToMessageQueue(GameObject *object)
	{
		mObjectMessageQueue.push_back(object);
	}

	std::vector<ScriptParam> SceneManager::Lua_LoadLevel(Script& caller, std::vector<ScriptParam> vParams)
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
	std::vector<ScriptParam> SceneManager::Lua_SaveLevel(Script& caller, std::vector<ScriptParam> vParams)
	{
		std::vector<ScriptParam> out;
		if (vParams.size() == 1)
		{
			if (vParams[0].getType() == ScriptParam::PARM_TYPE_STRING)
			{
				SceneManager::Instance().SaveLevel(vParams[0].getString());
			}
		}
		return out;
	}

	void
	SceneManager::LogMessage(std::string strError)
	{
		Ogre::LogManager::getSingleton().logMessage(strError);
	}

	std::vector<ScriptParam>
	SceneManager::Lua_LogMessage(Script& caller, std::vector<ScriptParam> vParams)
	{
		Ogre::String msg = Lua_ConcatToString(caller, vParams)[0].getString();
		LogMessage(msg);
		return std::vector<ScriptParam>();
	}

	std::vector<ScriptParam> SceneManager::Lua_ConcatToString(Script& caller, std::vector<ScriptParam> vParams)
	{
		Ogre::String str = "";
		for(unsigned int iArg=0; iArg<vParams.size(); iArg++)
		{
			if(vParams[iArg].getType()==ScriptParam::PARM_TYPE_STRING)
				str = str + vParams[iArg].getString().c_str();
			else if(vParams[iArg].getType()==ScriptParam::PARM_TYPE_BOOL)
				str = str + Ogre::StringConverter::toString(vParams[iArg].getBool());
			else if(vParams[iArg].getType()==ScriptParam::PARM_TYPE_FLOAT)
			{
				float val = static_cast<float>(vParams[iArg].getFloat());
				str = str + Ogre::StringConverter::toString(val);
			}
			else if(vParams[iArg].getType()==ScriptParam::PARM_TYPE_INT)
				str = str + Ogre::StringConverter::toString(vParams[iArg].getInt());
		}
		SCRIPT_RETURNVALUE(str)
	}

	std::vector<ScriptParam> SceneManager::Lua_GetRandomNumber(Script& caller, std::vector<ScriptParam> vParams)
	{
		int random = 0;
		auto err = Utils::TestParameters(vParams, "int int");
		if (err == "")
		{
			random = static_cast<int>(Ogre::Math::RangeRandom(vParams[0].getInt(), vParams[1].getInt()));
			SCRIPT_RETURNVALUE(random)
		}
		else SCRIPT_RETURNERROR(err)
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

		GameObject *object = Instance().CreateGameObject();
		object->AddComponent(GOComponentPtr(new GOCMeshRenderable(mesh, shadows)));
		if (collision == -1)
		{
			object->AddComponent(GOComponentPtr(new GOCStaticBody(mesh)));
		}
		else if (collision >= 0 && collision <= 3)
		{
			object->AddComponent(GOComponentPtr(new GOCRigidBody(mesh, 10, collision)));
		}
		out.push_back(ScriptParam(object->GetID()));
		return out;
	}

	void SceneManager::ReceiveMessage(Msg &msg)
	{
		if (msg.type == "UPDATE_PER_FRAME")
		{
			float time = msg.params.GetFloat("TIME");
			if (mClockEnabled)
			{
				mDayTime += (time*mTimeScale);
				if (mDayTime >= mMaxDayTime) mDayTime = 0.0f;
				AIManager::Instance().Update(time);
				if (mWeatherController) mWeatherController->Update(time);
				ITERATE(i, mTimeListeners)
					(*i)->UpdateScene(time);
			}

			//DestroyStoppedSounds();
		}
	}

	void SceneManager::EnableClock(bool enable)
	{
		mClockEnabled = enable;
		Msg msg;
		msg.type = "ENABLE_GAME_CLOCK";
		msg.params.AddBool("enable", enable);
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
		if (mWeatherController) mWeatherController->SetTime(hours, minutes);
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

	std::vector<ScriptParam> SceneManager::Lua_GetThis(Script& caller, std::vector<ScriptParam> vParams)
	{
		std::vector<ScriptParam> out;
		int id = -1;
		ScriptUser *scriptUser = ScriptSystem::GetInstance().GetScriptableObject(caller.GetID());
		if (!scriptUser) IceWarning("No object is associated with the calling script.")
		else id = scriptUser->GetThisID();
		out.push_back(ScriptParam(id));
		return out;	
	}

	std::vector<ScriptParam> SceneManager::Lua_GetObjectByName(Script& caller, std::vector<ScriptParam> vParams)
	{
		int returnID = -1;
		auto err = Utils::TestParameters(vParams, "string");
		if (err == "")
		{
			Ogre::String name = vParams[0].getString();
			for (auto i = Instance().mGameObjects.begin(); i != Instance().mGameObjects.end(); i++)
			{
				if (i->second->GetName() == name)
				{
					returnID = i->first;
					break;
				}
			}
		}
		else SCRIPT_RETURNERROR(err)
		std::vector<ScriptParam> out;
		out.push_back(returnID);
		return out;
	}

	std::vector<ScriptParam> SceneManager::Lua_GetFocusObject(Script& caller, std::vector<ScriptParam> params)
	{
		OgrePhysX::Scene::QueryHit hit;
		float maxDist = 5;
		int id = -1;
		Ogre::Vector3 origin = Main::Instance().GetCamera()->getDerivedPosition();
		if (SceneManager::Instance().GetPlayer())
			origin = SceneManager::Instance().GetPlayer()->GetGlobalPosition() + Ogre::Vector3(0, 2, 0);
		Ogre::Vector3 dir = Main::Instance().GetCamera()->getDerivedDirection().normalisedCopy();
		std::vector<OgrePhysX::Scene::QueryHit> query;
		Main::Instance().GetPhysXScene()->raycastAllShapes(query, Ogre::Ray(origin, dir), NX_ALL_SHAPES, -1, maxDist);
		float cdist = 10.0f;
		for (std::vector<OgrePhysX::Scene::QueryHit>::iterator i = query.begin(); i != query.end(); i++)
		{
			if (i->hitActor->userData)
			{
				GameObject *object = (GameObject*)i->hitActor->userData;
				if (object == SceneManager::Instance().GetPlayer()) continue;
				if (i->distance < cdist)
				{
					cdist = i->distance;
					id = object->GetID();
				}
			}
		}

		std::vector<ScriptParam> returner;
		returner.push_back(ScriptParam(id));
		return returner;
	}

	std::vector<ScriptParam> SceneManager::Lua_CreateMaterialProfile(Script& caller, std::vector<ScriptParam> vParams)
	{
		//CreateMaterialProfile(string name, float restitution = 0.1f, float staticFriction = 0.5f, float dynamicFriction = 0.5f
		std::vector<ScriptParam> ret;
		std::vector<Ice::ScriptParam> vRef;
		vRef.push_back(ScriptParam(std::string()));	//Name
		std::string strErrString=Ice::Utils::TestParameters(vParams, vRef, true);
		if (strErrString != "")
		{
			Utils::LogParameterErrors(caller, strErrString);
			ret.push_back(ScriptParam(0));
			return ret;
		}
		NxMaterialDesc desc;
		desc.setToDefault();
		desc.frictionCombineMode=NxCombineMode::NX_CM_AVERAGE;
		NxMaterial* material = Main::Instance().GetPhysXScene()->getNxScene()->createMaterial(desc);
		material->setRestitution(0.1f);
		material->setStaticFriction(0.5f);
		material->setDynamicFriction(0.5f);
		if(vParams.size()!=1)
		{
			vRef.push_back(ScriptParam(1.0));	//restitution
			vRef.push_back(ScriptParam(1.0));	//staticFriction
			vRef.push_back(ScriptParam(1.0));	//dynamicFriction
			std::string strErrString=Ice::Utils::TestParameters(vParams, vRef);
			if (strErrString != "")
			{
				Utils::LogParameterErrors(caller, strErrString);
				ret.push_back(ScriptParam(0));
				return ret;
			}
			material->setRestitution(vParams[1].getFloat());
			material->setStaticFriction(vParams[2].getFloat());
			material->setDynamicFriction(vParams[3].getFloat());
		}
		Instance().mSoundMaterialTable.AddMaterialProfile(vParams[0].getString().c_str(), material->getMaterialIndex());
		ret.push_back(ScriptParam((int)material->getMaterialIndex()));
		return ret;
	}

	std::vector<ScriptParam> SceneManager::Lua_Play3DSound(Script& caller, std::vector<ScriptParam> vParams)
	{
		Instance().DestroyStoppedSounds();

		std::vector<Ice::ScriptParam> vRef;
		vRef.push_back(ScriptParam(0.0f));	//x pos
		vRef.push_back(ScriptParam(0.0f));	//y pos
		vRef.push_back(ScriptParam(0.0f));	//z pos
		vRef.push_back(ScriptParam(std::string()));	//Soundfile
		vRef.push_back(ScriptParam(0.0f));	//Range
		vRef.push_back(ScriptParam(0.0f));	//Loudness
		std::string strErrString=Ice::Utils::TestParameters(vParams, vRef);
		if (strErrString != "")
		{
			Utils::LogParameterErrors(caller, strErrString);
			return std::vector<ScriptParam>();
		}
		Ogre::Vector3 position;
		position.x = vParams[0].getFloat();
		position.y = vParams[1].getFloat();
		position.z = vParams[2].getFloat();
		std::string soundFile = vParams[3].getString();
		float range = vParams[4].getFloat();
		float loudness = vParams[5].getFloat();
		//if (loudness > 1) loudness = 1;
		if (Ogre::ResourceGroupManager::getSingleton().resourceExists("General", soundFile))
		{
			int id = Ice::SceneManager::Instance().RequestID();
			Ogre::SceneNode *node = Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode(position);
			OgreOggSound::OgreOggISound *sound = Ice::Main::Instance().GetSoundManager()->createSound(Ogre::StringConverter::toString(id), soundFile, true, false);
			sound->setReferenceDistance(range/2);
			sound->setMaxDistance(range);
			sound->setVolume(loudness);
			sound->play();
			node->attachObject(sound);
			SceneManager::Instance().RegisterSound(sound);
		}
		return std::vector<ScriptParam>();
	}

	std::vector<ScriptParam> SceneManager::Lua_GetGameTimeHour(Script& caller, std::vector<ScriptParam> vParams)
	{
		std::vector<ScriptParam> out;
		out.push_back(ScriptParam(Instance().GetHour()));
		return out;
	}
	std::vector<ScriptParam> SceneManager::Lua_GetGameTimeMinutes(Script& caller, std::vector<ScriptParam> vParams)
	{
		std::vector<ScriptParam> out;
		out.push_back(ScriptParam(Instance().GetMinutes()));
		return out;
	}
	std::vector<ScriptParam> SceneManager::Lua_SetGameTime(Script& caller, std::vector<ScriptParam> vParams)
	{
		auto err = Utils::TestParameters(vParams, "int int");
		if (err == "")
		{
			int hour = vParams[0].getInt();
			int minutes = vParams[1].getInt();
			Instance().SetTime(hour, minutes);
		}
		else SCRIPT_RETURNERROR(err)
		SCRIPT_RETURN()
	}
	std::vector<ScriptParam> SceneManager::Lua_SetGameTimeScale(Script& caller, std::vector<ScriptParam> vParams)
	{
		auto err = Utils::TestParameters(vParams, "float");
		if (err == "")
		{
			float scale = vParams[0].getFloat();
			Instance().SetTimeScale(scale);
		}
		SCRIPT_RETURN()
	}

	std::vector<ScriptParam> SceneManager::Lua_GetPlayer(Script& caller, std::vector<ScriptParam> params)
	{
		int errout = -1;
		if (!Instance().GetPlayer()) SCRIPT_RETURNVALUE(errout)
		else SCRIPT_RETURNVALUE(Instance().GetPlayer()->GetID())
	}



	void SceneManager::RegisterSound(OgreOggSound::OgreOggISound* sound)
	{
		mPlayingSounds.push_back(sound);
		mDestroyStoppedSoundsDelay = 10;
	}

	void SceneManager::DestroyStoppedSounds()
	{
		float time = (float)(timeGetTime() / 1000);
		/*if (time - mDestroyStoppedSoundsLast < mDestroyStoppedSoundsDelay)
			return;*/
		auto iter = mPlayingSounds.begin();
		for (unsigned int i = 0; i < mPlayingSounds.size(); i++)
		{
			if ((*iter)->isStopped())
			{
				//Main::Instance().GetOgreSceneMgr()->destroySceneNode((*iter)->getParentSceneNode());
				Main::Instance().GetSoundManager()->destroySound(*iter);
				iter = mPlayingSounds.erase(iter);
			}
			else iter++;
		}
		mDestroyStoppedSoundsLast = time;
	}

	void SceneManager::AcquireCamera(CameraController *cam)
	{
		if (mCameraStack.size() > 0) mCameraStack.top()->DetachCamera();
		mCameraStack.push(cam);
		cam->AttachCamera(Main::Instance().GetCamera());
	}
	void SceneManager::TerminateCurrentCameraController()
	{
		IceAssert((mCameraStack.size() > 0));
		mCameraStack.top()->DetachCamera();
		mCameraStack.pop();
		if (mCameraStack.size() > 0) mCameraStack.top()->AttachCamera(Main::Instance().GetCamera());
	}

	SceneManager& SceneManager::Instance()
	{
		static SceneManager TheOneAndOnly;
		return TheOneAndOnly;
	};

}