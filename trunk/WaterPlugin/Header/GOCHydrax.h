#pragma once

#include "IceIncludes.h"
#include "IceGOComponent.h"
#include "IceGOCEditorInterface.h"
#define HYDRAX_LIB
#include "Hydrax.h"
#include "Noise/Perlin/Perlin.h"
#include "Modules/ProjectedGrid/ProjectedGrid.h"
#include "Modules/SimpleGrid/SimpleGrid.h"
#include "Modules/RadialGrid/RadialGrid.h"
#include "IceGameObject.h"
#include "IceMain.h"
#include "IceSceneManager.h"
#include "IceDataMap.h"
#include "IceMessageListener.h"
#include "IceWeatherController.h"

class __declspec(dllexport) GOCHydrax : public Ice::GOComponent, public Ice::GOCStaticEditorInterface, public Ice::ViewMessageListener
{
private:
	Hydrax::Hydrax *mHydrax;
	Hydrax::Module::Module *mModule;

	enum ModuleType
	{
		INF = 0,
		RECT = 1,
		RADIAL = 2
	};
	Ice::DataMap::Enum mModuleTypeEnum;
	Ogre::String mHdxFile;
	float mRadius;
	float mWidth;
	float mHeight;

	void _clear()
	{
		ICE_SAFEDELETE(mHydrax)
		mModule = nullptr;
		//ICE_SAFEDELETE(mModule)
	}
	void _create()
	{
		_clear();
		mHydrax = new Hydrax::Hydrax(Ice::Main::Instance().GetOgreSceneMgr(), Ice::Main::Instance().GetCamera(), Ice::Main::Instance().GetViewport());

		if (mModuleTypeEnum.selection == ModuleType::INF)
		{
			mModule = new Hydrax::Module::ProjectedGrid(mHydrax,
														new Hydrax::Noise::Perlin(),
														Ogre::Plane(Ogre::Vector3(0,1,0), Ogre::Vector3(0,0,0)),
														Hydrax::MaterialManager::NM_VERTEX,
														Hydrax::Module::ProjectedGrid::Options());
		}
		else if (mModuleTypeEnum.selection == ModuleType::RECT)
		{
			mModule = new Hydrax::Module::SimpleGrid(mHydrax,
													new Hydrax::Noise::Perlin(),
													Hydrax::MaterialManager::NormalMode::NM_RTT);
		}
		else if (mModuleTypeEnum.selection == ModuleType::RADIAL)
		{
			mModule = new Hydrax::Module::RadialGrid(mHydrax,
													new Hydrax::Noise::Perlin(),
													Hydrax::MaterialManager::NormalMode::NM_RTT);
		}
		if (!mModule)
		{
			IceWarning("Module creation failed")
			return;
		}

		mHydrax->setModule(mModule);

		std::vector<Ogre::RenderQueueGroupID> disabled_renderqueues;
		disabled_renderqueues.push_back((Ogre::RenderQueueGroupID)(Ogre::RENDER_QUEUE_SKIES_EARLY));
		disabled_renderqueues.push_back((Ogre::RenderQueueGroupID)(Ogre::RENDER_QUEUE_SKIES_LATE));
		disabled_renderqueues.push_back((Ogre::RenderQueueGroupID)(Ogre::RENDER_QUEUE_SKIES_EARLY + 2));
		disabled_renderqueues.push_back((Ogre::RenderQueueGroupID)(Ogre::RENDER_QUEUE_SKIES_EARLY + 3));
		disabled_renderqueues.push_back((Ogre::RenderQueueGroupID)(Ogre::RENDER_QUEUE_SKIES_EARLY + 4));
		disabled_renderqueues.push_back((Ogre::RenderQueueGroupID)(Ogre::RENDER_QUEUE_SKIES_EARLY + 5));
		disabled_renderqueues.push_back((Ogre::RenderQueueGroupID)(Ogre::RENDER_QUEUE_SKIES_EARLY + 6));
		mHydrax->getRttManager()->setDisableReflectionCustomNearCliplPlaneRenderQueues(disabled_renderqueues);
		mHydrax->getRttManager()->setBitsPerChannel(Hydrax::RttManager::RttType::RTT_REFLECTION, Hydrax::RttManager::BitsPerChannel::BPC_16);
		mHydrax->getRttManager()->setBitsPerChannel(Hydrax::RttManager::RttType::RTT_REFRACTION, Hydrax::RttManager::BitsPerChannel::BPC_16);
		mHydrax->getRttManager()->setBitsPerChannel(Hydrax::RttManager::RttType::RTT_DEPTH, Hydrax::RttManager::BitsPerChannel::BPC_16);
		mHydrax->getRttManager()->setBitsPerChannel(Hydrax::RttManager::RttType::RTT_DEPTH_REFLECTION, Hydrax::RttManager::BitsPerChannel::BPC_16);
		mHydrax->getRttManager()->setBitsPerChannel(Hydrax::RttManager::RttType::RTT_DEPTH_AIP, Hydrax::RttManager::BitsPerChannel::BPC_16);
		mHydrax->getRttManager()->setBitsPerChannel(Hydrax::RttManager::RttType::RTT_GPU_NORMAL_MAP, Hydrax::RttManager::BitsPerChannel::BPC_16);

		mHydrax->loadCfg(mHdxFile);

		if (mModuleTypeEnum.selection == ModuleType::RECT)
		{
			Hydrax::Module::SimpleGrid::Options options = ((Hydrax::Module::SimpleGrid*)mModule)->getOptions();
			options.MeshSize.setSize(mWidth, mHeight);
			((Hydrax::Module::SimpleGrid*)mModule)->setOptions(options);
		}
		else if (mModuleTypeEnum.selection == ModuleType::RADIAL)
		{
			Hydrax::Module::RadialGrid::Options options = ((Hydrax::Module::RadialGrid*)mModule)->getOptions();
			options.Radius = mRadius;
			((Hydrax::Module::RadialGrid*)mModule)->setOptions(options);
		}

		mHydrax->create();
		mHydrax->getRttManager()->getTexture(Hydrax::RttManager::RttType::RTT_REFRACTION)->getBuffer()->getRenderTarget()->getViewport(0)->setMaterialScheme("LowQuality");
		mHydrax->getRttManager()->getTexture(Hydrax::RttManager::RttType::RTT_REFLECTION)->getBuffer()->getRenderTarget()->getViewport(0)->setMaterialScheme("LowQuality");
	}

public:

	static void InstallPlugin()
	{
		Ogre::ResourceGroupManager::getSingleton().addResourceLocation("Data/Hydrax/", "FileSystem", HYDRAX_RESOURCE_GROUP);
		LoadSave::LoadSave::Instance().RegisterObject(&GOCHydrax::Register);
		Ice::SceneManager::Instance().RegisterGOCPrototype("E", std::make_shared<GOCHydrax>());
	}

	GOCHydrax(void)
	{
		mModule = nullptr;
		mHydrax = nullptr;
		mModuleTypeEnum.choices.push_back("Infinite");
		mModuleTypeEnum.choices.push_back("Rectangle");
		mModuleTypeEnum.choices.push_back("Radial");
		mModuleTypeEnum.selection = 0;
		Ice::MessageSystem::Instance().JoinNewsgroup(this, "START_RENDERING");
	}
	~GOCHydrax(void)
	{
		_clear();
	}

	goc_id_type& GetComponentID() const { static std::string name = "Hydrax"; return name; }

	void UpdatePosition(Ogre::Vector3 position)
	{
		if (mHydrax) mHydrax->setPosition(position);
	}
	void UpdateOrientation(Ogre::Quaternion orientation)
	{
	}
	void UpdateScale(Ogre::Vector3 scale)
	{
	}

	void ShowEditorVisual(bool show)
	{
	}

	//Editor interface
	BEGIN_GOCEDITORINTERFACE(GOCHydrax, "Hydrax")
		PROPERTY_STRING(mHdxFile, "Hdx File", ".hdx")
		PROPERTY_ENUM(mModuleTypeEnum, "Module Type", mModuleTypeEnum)
		PROPERTY_FLOAT(mRadius, "Radius", 10.0f)
		PROPERTY_FLOAT(mWidth, "Width", 10.0f)
		PROPERTY_FLOAT(mHeight, "Height", 10.0f)
	END_GOCEDITORINTERFACE

	void OnSetParameters()
	{
		_create();
	}

	void ReceiveMessage(Ice::Msg &msg)
	{
		if (mHydrax && msg.type == "START_RENDERING")
		{
			if (Ice::SceneManager::Instance().GetWeatherController())
			{
				mHydrax->setSunPosition(Ice::SceneManager::Instance().GetWeatherController()->GetSunLightPosition());
				Ogre::ColourValue c = Ice::SceneManager::Instance().GetWeatherController()->GetSunLightColour();
				mHydrax->setSunColor(Ogre::Vector3(c.r, c.g, c.b));
			}
			mHydrax->update(msg.params.GetFloat("TIME"));
		}
	}

	void Save(LoadSave::SaveSystem& mgr)
	{
	}
	void Load(LoadSave::LoadSystem& mgr)
	{
	}
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "Hydrax"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
	static LoadSave::Saveable* NewInstance() { return new GOCHydrax; };
};

