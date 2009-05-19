
#include "SGTOcean.h"
#include "SGTMain.h"
#include "SGTSceneManager.h"
#include "SGTWeatherController.h"

void SGTOcean::ReceiveMessage(SGTMsg &msg)
{
	if (msg.mNewsgroup == "UPDATE_PER_FRAME")
	{
		float time = msg.mData.GetFloat("TIME");
		if (SGTSceneManager::Instance().GetWeatherController())
		{
			//SGTSceneManager::Instance().GetWeatherController()->GetCaelumSystem()->getSun()->getNode()->_getDerivedPosition();
			Ogre::ColourValue suncolor = SGTSceneManager::Instance().GetWeatherController()->GetCaelumSystem()->getSun()->getLightColour();
			mHydrax->setSunPosition(SGTSceneManager::Instance().GetWeatherController()->GetCaelumSystem()->getSun()->getLightDirection() * Ogre::Vector3(-10000,-10000,-10000));
			Ogre::Vector3 lightcolour;
			lightcolour.x = SGTSceneManager::Instance().GetWeatherController()->GetCaelumSystem()->getSun()->getBodyColour().r;
			lightcolour.y = SGTSceneManager::Instance().GetWeatherController()->GetCaelumSystem()->getSun()->getBodyColour().g;
			lightcolour.z = SGTSceneManager::Instance().GetWeatherController()->GetCaelumSystem()->getSun()->getBodyColour().b;
			mHydrax->setSunColor(lightcolour);
		}
		mHydrax->update(time);
	}
}

SGTOcean::SGTOcean(void)
{
	/*SGTMain::Instance().GetOgreSceneMgr()->setSkyBox(true,"Sky/EarlyMorning", 300);
	Ogre::Light *Light = SGTMain::Instance().GetOgreSceneMgr()->createLight("Light0");
	Light->setType(Ogre::Light::LightTypes::LT_DIRECTIONAL);
	Light->setDirection(0,-0.2,1);
	Light->setPosition(Ogre::Vector3(0,0,0));
	Light->setDiffuseColour(2, 2, 2);
	Light->setSpecularColour(Ogre::ColourValue(1, 0.9, 0.6));*/

	mHydrax = new Hydrax::Hydrax(SGTMain::Instance().GetOgreSceneMgr(), SGTMain::Instance().GetCamera(), SGTMain::Instance().GetWindow()->getViewport(0));

		// Create our projected grid module  
		mGeometry 
			= new Hydrax::Module::ProjectedGrid(// Hydrax parent pointer
			                                    mHydrax,
												// Noise module
			                                    new Hydrax::Noise::Perlin(/*Generic one*/),
												// Base plane
			                                    Ogre::Plane(Ogre::Vector3(0,1,0), Ogre::Vector3(0,0,0)),
												// Normal mode
												Hydrax::MaterialManager::NM_VERTEX,
												// Projected grid options
										        Hydrax::Module::ProjectedGrid::Options(264));

		// Set our module
		mHydrax->setModule(static_cast<Hydrax::Module::Module*>(mGeometry));

		// Load all parameters from config file
		// Remarks: The config file must be in Hydrax resource group.
		// All parameters can be set/updated directly by code(Like previous versions),
		// but due to the high number of customizable parameters, Hydrax 0.4 allows save/load config files.
		mHydrax->loadCfg("HydraxDemo.hdx");

		mHydrax->create();

		//Ogre::MaterialPtr watermat = mHydrax->getMaterialManager()->getMaterial(Hydrax::MaterialManager::MAT_WATER);

		//watermat->getTechnique(0)->getPass(0)->setFog(true, Ogre::FOG_NONE);

		SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
}

SGTOcean::~SGTOcean(void)
{
}
