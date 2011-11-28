
#include "IceSceneManager.h"
#include "IceMain.h"
#include "LoadSave.h"
#include "IceWeatherController.h"
#include "Saveable.h"
#include "shellapi.h"
#include "IceScriptSystem.h"
#include "IceAIManager.h"
#include "IceProcessNode.h"
#include "IceProcessNodeQueue.h"
#include "IceProcessNodeManager.h"
#include "IceScriptedProcess.h"
#include "IceMaterialTable.h"

#include "IceCameraController.h"

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"

#include "IceUtils.h"

#include "mmsystem.h"

#include "Caelum.h"

namespace Ice
{

	SceneManager::SceneManager()
	{
		mWeatherController = 0;
		mIndoorRendering = false;
		mClockEnabled = true;
		mNextID = 0;
		mDayTime = 0.0f;
		mMaxDayTime = 86400.0f;
		mTimeScale = 64.0f;

		mShowEditorVisuals = false;

		mClearingScene = false;

		JoinNewsgroup(GlobalMessageIDs::UPDATE_PER_FRAME);
	}

	SceneManager::~SceneManager()
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

	void SceneManager::RegisterPlayer(GameObjectPtr player)
	{
		mPlayer = player;
	}

	void SceneManager::ShowEditorMeshes(bool show)
	{
		mShowEditorVisuals = show;
		for (auto i = mGameObjects.begin(); i != mGameObjects.end(); i++)
		{
			i->second->ShowEditorVisuals(show);
		}
	}

	WeatherController* SceneManager::GetWeatherController()
	{
		return mWeatherController;
	}

	void SceneManager::Init()
	{
		Reset();

		Main::Instance().GetOgreSceneMgr()->createStaticGeometry("StaticGeometry");
	}

	void SceneManager::PostInit()
	{
	}

	void SceneManager::ClearGameObjects()
	{
		mClearingScene = true;
		mGameObjects.clear();
		mClearingScene = false;
	}

	void SceneManager::Reset()
	{
		AIManager::Instance().GetNavigationMesh()->Reset();
		ClearGameObjects();
		SetToOutdoor();
	}

	void SceneManager::Shutdown()
	{
		SetToIndoor();

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

		if (!mWeatherController->GetCaelumSystem()->getSun()->getForceDisable())
			AIManager::Instance().RegisterLight(mWeatherController->GetCaelumSystem()->getSun()->getMainLight());
		else AIManager::Instance().RegisterLight(mWeatherController->GetCaelumSystem()->getMoon()->getMainLight());
	}

	GameObjectPtr SceneManager::AddLevelMesh(Ogre::String meshname)
	{
		Ice::GameObjectPtr object = CreateGameObject();
		//object->SetGlobalScale(Ogre::Vector3(0.1, 0.1, 0.1));	//test
		object->AddComponent(Ice::GOComponentPtr(new Ice::GOCMeshRenderable(meshname, true)));
		object->AddComponent(Ice::GOComponentPtr(new Ice::GOCStaticBody(meshname)));
		object->SetSelectable(false);
		AIManager::Instance().GetNavigationMesh()->AddOgreMesh(object->GetComponent<GOCMeshRenderable>()->GetEntity()->getMesh());
		return object;
	}

	void SceneManager::RegisterGameObject(GameObjectPtr object)
	{
		mGameObjects.insert(std::make_pair<int, GameObjectPtr>(object->GetID(), object));
	}

	void SceneManager::CreateNavigationMesh()
	{
		AIManager::Instance().GetNavigationMesh()->Reset();
		ITERATE(i, mGameObjects)
		{
			if (i->second->GetComponent<GOCMeshRenderable>() && i->second->GetComponent<GOCStaticBody>())
				AIManager::Instance().GetNavigationMesh()->AddOgreMesh(i->second->GetComponent<GOCMeshRenderable>()->GetEntity()->getMesh());
		}
		AIManager::Instance().GetNavigationMesh()->Update();
	}

	void SceneManager::LoadLevel(Ogre::String levelfile, bool load_dynamic)
	{
		Msg msg;
		msg.typeID = GlobalMessageIDs::LOADLEVEL_BEGIN;
		MessageSystem::Instance().MulticastMessage(msg, true);

		ClearGameObjects();

		LoadSave::LoadSystem *ls = LoadSave::LoadSave::Instance().LoadFile(levelfile);

		std::shared_ptr<DataMap> levelparams = ls->LoadTypedObject<DataMap>();
		SetParameters(levelparams.get());

		std::vector<GameObjectPtr> objects;
		ls->LoadAtom("vector<GameObjectPtr>", &objects);	//objects call RegisterObject

		ls->CloseFile();
		delete ls;

		CreateNavigationMesh();

		msg.typeID = GlobalMessageIDs::LOADLEVEL_END;
		MessageSystem::Instance().MulticastMessage(msg, true);
	}

	void SceneManager::SaveLevel(Ogre::String levelfile)
	{
		Msg msg;
		msg.typeID = GlobalMessageIDs::SAVELEVEL_BEGIN;
		MessageSystem::Instance().MulticastMessage(msg, true);

		ShowEditorMeshes(false);
		LoadSave::SaveSystem *ss=LoadSave::LoadSave::Instance().CreateSaveFile(levelfile, levelfile + ".xml");
		DataMap map;
		GetParameters(&map);
		ss->SaveObject(&map, "LevelParams");
		std::vector<GameObjectPtr> objects;
		for (auto i = mGameObjects.begin(); i != mGameObjects.end(); i++)
			objects.push_back(i->second);
		ss->SaveAtom("vector<GameObjectPtr>", &objects, "Objects");
		//ss->SaveObject(AIManager::Instance().GetNavigationMesh(), "WayMesh");
		ss->CloseFiles();
		ICE_DELETE ss;

		msg.typeID = GlobalMessageIDs::SAVELEVEL_END;
		MessageSystem::Instance().MulticastMessage(msg, true);
	}

	void SceneManager::SetParameters(DataMap *parameters)
	{
		SetWeatherParameters(parameters);
		mStartupScriptName = parameters->GetValue<Ogre::String>("Startup Script", "");
		if (mStartupScriptName != "") ScriptSystem::GetInstance().CreateInstance(mStartupScriptName);
	}

	void SceneManager::GetParameters(DataMap *parameters)
	{
		parameters->AddOgreString("Startup Script", mStartupScriptName);

		GetWeatherParameters(parameters);
	}

	void SceneManager::SetWeatherParameters(DataMap *parameters)
	{
		DataMap currParams;
		GetWeatherParameters(&currParams);

		bool indoor = parameters->GetBool("Indoor", currParams.GetBool("Indoor"));
		if (indoor) SetToIndoor();
		else
		{
			SetToOutdoor();
			mWeatherController->GetCaelumSystem()->getSun()->setAmbientMultiplier(parameters->GetOgreCol("AmbientLight", currParams.GetOgreCol("AmbientLight")));
			mWeatherController->GetCaelumSystem()->getSun()->setDiffuseMultiplier(parameters->GetOgreCol("Sun_DiffuseLight", currParams.GetOgreCol("Sun_DiffuseLight")));
			mWeatherController->GetCaelumSystem()->getSun()->setSpecularMultiplier(parameters->GetOgreCol("Sun_SpecularLight", currParams.GetOgreCol("Sun_SpecularLight")));
			mWeatherController->Update(0);
		}

		Ogre::GpuSharedParametersPtr hdrParams = Ogre::GpuProgramManager::getSingleton().getSharedParameters("HDRParams");

		Ogre::ColourValue col = parameters->GetOgreCol("Luminence_Factor", currParams.GetOgreCol("Luminence_Factor")); col.a = 0;
		hdrParams->setNamedConstant("Luminence_Factor", col);
		hdrParams->setNamedConstant("Tonemap_White", parameters->GetFloat("Tonemap_White"));
		col = parameters->GetOgreCol("Brightpass_Threshold", currParams.GetOgreCol("Brightpass_Threshold")); col.a = 0;
		hdrParams->setNamedConstant("Brightpass_Threshold", col);

		col = parameters->GetOgreCol("BrightpassAmbient_Threshold", currParams.GetOgreCol("BrightpassAmbient_Threshold")); col.a = 0;
		hdrParams->setNamedConstant("BrightpassAmbient_Threshold", col);
		hdrParams->setNamedConstant("BloomAmbient_GlareScale", parameters->GetValue<float>("BloomAmbient_GlareScale", 0.5f));
		hdrParams->setNamedConstant("BloomAmbient_GlarePower", parameters->GetValue<float>("BloomAmbient_GlarePower", 0.5f));

		hdrParams->setNamedConstant("Bloom_GlareScale", parameters->GetValue<float>("Bloom_GlareScale", parameters->GetFloat("Bloom_GlareScale")));
		hdrParams->setNamedConstant("Bloom_GlarePower", parameters->GetValue<float>("Bloom_GlarePower", parameters->GetFloat("Bloom_GlarePower")));
		hdrParams->setNamedConstant("Bloom_StarScale", parameters->GetValue<float>("Bloom_StarScale", parameters->GetFloat("Bloom_StarScale")));
		hdrParams->setNamedConstant("Bloom_StarPower", parameters->GetValue<float>("Bloom_StarPower", parameters->GetFloat("Bloom_StarPower")));

		hdrParams->setNamedConstant("LinearTonemap_KeyLumScale", parameters->GetFloat("LinearTonemap_KeyLumScale", parameters->GetFloat("LinearTonemap_KeyLumScale")));
		hdrParams->setNamedConstant("LinearTonemap_KeyMax", parameters->GetFloat("LinearTonemap_KeyMax", parameters->GetFloat("LinearTonemap_KeyMax")));
		hdrParams->setNamedConstant("LinearTonemap_KeyMaxOffset", parameters->GetFloat("LinearTonemap_KeyMaxOffset", parameters->GetFloat("LinearTonemap_KeyMaxOffset")));
		hdrParams->setNamedConstant("LinearTonemap_KeyMin", parameters->GetFloat("LinearTonemap_KeyMin", parameters->GetFloat("LinearTonemap_KeyMin")));

		hdrParams->setNamedConstant("LightAdaption_Exponent", parameters->GetValue<float>("LightAdaption_Exponent", parameters->GetFloat("LightAdaption_Exponent")));
		hdrParams->setNamedConstant("LightAdaption_Factor", parameters->GetValue<float>("LightAdaption_Factor", parameters->GetFloat("LightAdaption_Factor")));
		hdrParams->setNamedConstant("ShadowAdaption_Exponent", parameters->GetValue<float>("ShadowAdaption_Exponent", parameters->GetFloat("ShadowAdaption_Exponent")));
		hdrParams->setNamedConstant("ShadowAdaption_Factor", parameters->GetValue<float>("ShadowAdaption_Factor", parameters->GetFloat("ShadowAdaption_Factor")));
	}
	void SceneManager::GetWeatherParameters(DataMap *parameters)
	{
		parameters->AddBool("Indoor", mIndoorRendering);
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

	std::unordered_map<int, GameObjectPtr>& SceneManager::GetGameObjects()
	{
		return mGameObjects;
	}

	void SceneManager::RemoveGameObject(int objectID)
	{
		auto i = mGameObjects.find(objectID);
		if (i != mGameObjects.end()) mGameObjects.erase(i);
	}
	GameObjectPtr SceneManager::CreateGameObject()
	{
		GameObjectPtr object = std::make_shared<GameObject>();
		object->SetWeakThis(std::weak_ptr<GameObject>(object));
		mGameObjects.insert(std::make_pair<int, GameObjectPtr>(object->GetID(), object));
		return object;
	}

	GameObjectPtr SceneManager::GetObjectByInternID(int id)
	{
		auto i = mGameObjects.find(id);
		if (i != mGameObjects.end()) return i->second;
		return GameObjectPtr();
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

	std::vector<ScriptParam>
	SceneManager::Lua_LogMessage(Script& caller, std::vector<ScriptParam> vParams)
	{
		Ogre::String msg = Lua_ConcatToString(caller, vParams)[0].getString();
		Log::Instance().LogMessage(msg);
		return std::vector<ScriptParam>();
	}

	std::vector<ScriptParam> SceneManager::Lua_ConcatToString(Script& caller, std::vector<ScriptParam> vParams)
	{
		Ogre::String str = "";
		for(unsigned int iArg=0; iArg<vParams.size(); iArg++)
		{
			switch(vParams[iArg].getType())
			{
			case ScriptParam::PARM_TYPE_STRING:
				str = str + vParams[iArg].getString().c_str();
				break;
			case ScriptParam::PARM_TYPE_BOOL:
				str = str + Ogre::StringConverter::toString(vParams[iArg].getBool());
				break;
			case ScriptParam::PARM_TYPE_FLOAT:
			{
				float val = static_cast<float>(vParams[iArg].getFloat());
				str = str + Ogre::StringConverter::toString(val);
				break;
			}
			case ScriptParam::PARM_TYPE_INT:
				str = str + Ogre::StringConverter::toString(vParams[iArg].getInt());
				break;
			case ScriptParam::PARM_TYPE_TABLE:
			{
				str = str + "{ ";
				std::map<ScriptParam, ScriptParam> mTable=vParams[iArg].getTable();
				for(std::map<ScriptParam, ScriptParam>::const_iterator it=mTable.begin(); it!=mTable.end();)
				{
					str = str + "[";
					//stl-spam...
					str = str + Lua_ConcatToString(caller, std::vector<ScriptParam>(1, it->first))[0].getString().c_str();
					str = str + "]=";
					str = str + Lua_ConcatToString(caller, std::vector<ScriptParam>(1, it->second))[0].getString().c_str();
					it++;
					if(it!=mTable.end())
						str = str + ", ";
				}
				str = str + " }";
				break;
			}
			default:
				break;
			}
		}
		SCRIPT_RETURNVALUE(str)
	}

	std::vector<ScriptParam> SceneManager::Lua_GetRandomNumber(Script& caller, std::vector<ScriptParam> vParams)
	{
		float random = 0;
		auto err = Utils::TestParameters(vParams, "float float");
		if (err == "")
		{
			random = Ogre::Math::RangeRandom(vParams[0].getFloat(), vParams[1].getFloat());
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

		GameObjectPtr object = Instance().CreateGameObject();
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
		if (msg.typeID == GlobalMessageIDs::UPDATE_PER_FRAME)
		{
			float time = msg.params.GetFloat("TIME");
			if (mClockEnabled)
			{
				mDayTime += (time*mTimeScale);
				if (mDayTime >= mMaxDayTime) mDayTime = 0.0f;
				if (mWeatherController) mWeatherController->Update(time);
			}
		}
	}

	void SceneManager::EnableClock(bool enable)
	{
		mClockEnabled = enable;
		Msg msg;
		msg.typeID = GlobalMessageIDs::ENABLE_GAME_CLOCK;
		msg.params.AddBool("enable", enable);
		MessageSystem::Instance().MulticastMessage(msg);
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
		float maxDist = 5;
		int id = -1;
		Ogre::Vector3 origin = Main::Instance().GetCamera()->getDerivedPosition();
		if (SceneManager::Instance().GetPlayer())
			origin = SceneManager::Instance().GetPlayer()->GetGlobalPosition() + Ogre::Vector3(0, 2, 0);
		Ogre::Vector3 dir = Main::Instance().GetCamera()->getDerivedDirection().normalisedCopy();
		PxRaycastHit buffer[10];
		bool blockingHit;
		PxU32 numHits = Main::Instance().GetPhysXScene()->getPxScene()->raycastMultiple(OgrePhysX::toPx(origin), OgrePhysX::toPx(dir), maxDist, PxSceneQueryFlag::eNORMAL, buffer, 10, blockingHit);
		for (unsigned int i = 0; i < numHits; ++i)
		{
			if (buffer[i].shape->getActor().userData)
			{
				GameObject *object = (GameObject*)buffer[i].shape->getActor().userData;
				if (object == SceneManager::Instance().GetPlayer().get()) continue;
				id = object->GetID();
				break;
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

		float staticFriction = 0.5f;
		float dynamicFriction = 0.5f;
		float restitution = 0.5f;

		if(vParams.size()!=1)
		{
			vRef.push_back(ScriptParam(1.0));	//staticFriction
			vRef.push_back(ScriptParam(1.0));	//dynamicFriction
			vRef.push_back(ScriptParam(1.0));	//restitution
			strErrString=Ice::Utils::TestParameters(vParams, vRef);
			if (strErrString != "")
			{
				SCRIPT_RETURNERROR(strErrString)
			}
			staticFriction = vParams[1].getFloat(); 
			staticFriction = vParams[2].getFloat(); 
			staticFriction = vParams[3].getFloat(); 
		}

		MaterialTable::Instance().AddMaterialProfile(vParams[0].getString().c_str(),
			OgrePhysX::getPxPhysics()->createMaterial(staticFriction, dynamicFriction, restitution));
		
		SCRIPT_RETURN()
	}

	std::vector<ScriptParam> SceneManager::Lua_Play3DSound(Script& caller, std::vector<ScriptParam> vParams)
	{
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
			sound->setListener(&(Instance().mOggListener));
			node->attachObject(sound);
			AIManager::Instance().NotifySound(soundFile, position, range, loudness);
		}
		return std::vector<ScriptParam>();
	}

	void SceneManager::OggListener::soundStopped(OgreOggSound::OgreOggISound* sound)
	{
		Main::Instance().GetOgreSceneMgr()->destroySceneNode(sound->getParentSceneNode());
		Main::Instance().GetSoundManager()->destroySound(sound);
	}
	SceneManager::OggCamSync::OggCamSync()
	{
		JoinNewsgroup(GlobalMessageIDs::UPDATE_PER_FRAME);
	}
	void SceneManager::OggCamSync::ReceiveMessage(Msg &msg)
	{
		Main::Instance().GetSoundManager()->getListener()->setPosition(Main::Instance().GetCamera()->getDerivedPosition());
		Main::Instance().GetSoundManager()->getListener()->setOrientation(Main::Instance().GetCamera()->getDerivedOrientation());
	}

	std::vector<ScriptParam> SceneManager::Lua_CreateGameObject(Script& caller, std::vector<ScriptParam> vParams)
	{
		GameObjectPtr object = Instance().CreateGameObject();
		SCRIPT_RETURNVALUE(object->GetID());
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
