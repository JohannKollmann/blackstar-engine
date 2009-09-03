
#include "..\Header\SGTWeatherController.h"
#include "SGTMain.h"
#include "OIS/OIS.h"

SGTWeatherController::SGTWeatherController(void)
{
        // Pick components to create in the demo.
        // You can comment any of those and it should still work
        // It makes little sense to comment the first three.
	Caelum::CaelumSystem::CaelumComponent componentMask = Caelum::CaelumSystem::CAELUM_COMPONENTS_DEFAULT;

	// Initialise Caelum
	mCaelumSystem = new Caelum::CaelumSystem (Ogre::Root::getSingletonPtr(), SGTMain::Instance().GetOgreSceneMgr(), componentMask);//Caelum::CaelumSystem::CAELUM_COMPONENTS_NONE);

	/*mCaelumSystem->setSkyDome (new Caelum::SkyDome (SGTMain::Instance().GetOgreSceneMgr(), mCaelumSystem->getCaelumCameraNode ()));
    mCaelumSystem->setSun (new Caelum::SphereSun(SGTMain::Instance().GetOgreSceneMgr(), mCaelumSystem->getCaelumCameraNode ()));
    mCaelumSystem->setMoon (new Caelum::Moon(SGTMain::Instance().GetOgreSceneMgr(), mCaelumSystem->getCaelumCameraNode ()));
    mCaelumSystem->setCloudSystem (new Caelum::CloudSystem (SGTMain::Instance().GetOgreSceneMgr(), mCaelumSystem->getCaelumGroundNode ()));
    mCaelumSystem->setPointStarfield (new Caelum::PointStarfield (SGTMain::Instance().GetOgreSceneMgr(), mCaelumSystem->getCaelumCameraNode ()));*/

	mCaelumSystem->attachViewport (SGTMain::Instance().GetViewport());
	/*mCaelumSystem->setPrecipitationController (new Caelum::PrecipitationController (SGTMain::Instance().GetOgreSceneMgr()));
	mCaelumSystem->getPrecipitationController ()->createViewportInstance (SGTMain::Instance().GetCamera()->getViewport ());

    if (mCaelumSystem->getPrecipitationController ()) {
        mCaelumSystem->getPrecipitationController ()->setIntensity (0);
	}*/


	mCaelumSystem->getUniversalClock ()->setTimeScale (0);

    mCaelumSystem->setManageSceneFog(false);
    mCaelumSystem->setSceneFogDensityMultiplier(0.0015);
	mCaelumSystem->setMinimumAmbientLight(Ogre::ColourValue(0.1, 0.1,0.1));
    mCaelumSystem->setManageAmbientLight (true); 

	mCaelumSystem->getUniversalClock ()->setGregorianDateTime (2007, 4, 9, 9, 30, 0);

	mCaelumSystem->getSun ()->setDiffuseMultiplier (Ogre::ColourValue (1.5, 1.5, 1.3));
	mCaelumSystem->getSun ()->setSpecularMultiplier (Ogre::ColourValue (1, 1, 1));

	/*mCaelumSystem->setEnsureSingleShadowSource(true);
	mCaelumSystem->getMoon()->setDiffuseMultiplier(Ogre::ColourValue (1, 1, 1));
	mCaelumSystem->getMoon()->setSpecularMultiplier(Ogre::ColourValue (1, 1, 1));
	mCaelumSystem->getMoon()->setPhase(1.0f);*/
	mCaelumSystem->getMoon()->getMainLight()->setCastShadows(false);
	  //mCaelumSystem->getSun()->getMainLight()->setCastShadows(false);

	// Register caelum to the render target
	//SGTMain::Instance().GetWindow()->addListener (mCaelumSystem);
	//Ogre::Root::getSingletonPtr()->addFrameListener (mCaelumSystem);

	mPaused = false;

	SetSpeedFactor(mCaelumSystem->getUniversalClock ()->getTimeScale ());

	SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "KEY_UP");

        /*// Test spinning the caelum root node. Looks wrong in the demo;
        // but at least the sky components are aligned with each other.
        if (false) {
            mCaelumSystem->getRootNode()->setOrientation(
                    Ogre::Quaternion(Ogre::Radian(Ogre::Math::PI), Ogre::Vector3::UNIT_Z) *
                    Ogre::Quaternion(Ogre::Radian(Ogre::Math::PI / 2), Ogre::Vector3::UNIT_X));
            mCaelumSystem->getRootNode()->_update(true, true);
        }

        // KNOWN BUG: The horizon is pure white if setManageFog is false.
        // I blame it on the transparent skydome.
        mCaelumSystem->setManageSceneFog(true);
		mCaelumSystem->setSceneFogDensityMultiplier(0.0001);

        // For sphere-based sun:
        //mCaelumSystem->setSun (NULL);
        //mCaelumSystem->setSun (new Caelum::SphereSun(mScene, mCaelumSystem->getRootNode ()));

      // Setup sun options
      if (mCaelumSystem->getSun ())
      {
		  mCaelumSystem->getSun ()->setAmbientMultiplier (Ogre::ColourValue(1.5, 1.5, 1.5));
         mCaelumSystem->getSun ()->setDiffuseMultiplier (Ogre::ColourValue(1.0, 1.0, 1.0));
         mCaelumSystem->getSun ()->setSpecularMultiplier (Ogre::ColourValue(1, 1, 1));
      }

      if (mCaelumSystem->getMoon())
      {
         mCaelumSystem->getMoon()->setAmbientMultiplier (Ogre::ColourValue(1.0, 1.0, 1.0));
         mCaelumSystem->getMoon()->setDiffuseMultiplier (Ogre::ColourValue(1, 1, 1));
         mCaelumSystem->getMoon()->setSpecularMultiplier (Ogre::ColourValue(1, 1, 1));
      } 

        // Disable ground fog ang haze.
        SetGroundFogEnabled(false);
        SetHazeEnabled(false);

        // Setup cloud options.
        // Tweak these settings to make the demo look pretty.
        if (mCaelumSystem->getClouds ()) {
            mCaelumSystem->getClouds ()->setCloudSpeed(Ogre::Vector2(0.000005, -0.000009));
            mCaelumSystem->getClouds ()->setCloudBlendTime(3600 * 24);
            mCaelumSystem->getClouds ()->setCloudCover(0.4);
        }

        // Setup starfield options
        if (mCaelumSystem->getStarfield ()) {
			mCaelumSystem->getStarfield ()->setInclination (Ogre::Degree (13));
        }

        // Set time acceleration.
        mCaelumSystem->getUniversalClock ()->setTimeScale (0);

        // Total solar eclipse maximum
		if (false) {
			mCaelumSystem->getUniversalClock ()->setGregorianDateTime (1999, 8, 11, 11, 3, 0);
			mCaelumSystem->getSolarSystemModel ()->setObserverLongitude (Ogre::Degree(24.3));
			mCaelumSystem->getSolarSystemModel ()->setObserverLatitude (Ogre::Degree(45.1));
		}

		// Winter dawn in Sydney, Australia
		if (true) {
            mCaelumSystem->getUniversalClock ()->setGregorianDateTime (2008, 5, 1, 24, 0, 0);
            mCaelumSystem->getSolarSystemModel ()->setObserverLongitude (
                    Ogre::Degree(151 + 12.0 / 60 + 35.9 / 3600));
            mCaelumSystem->getSolarSystemModel ()->setObserverLatitude (
                    Ogre::Degree(-33 - 51.0 / 60 - 40.0 / 3600));
		}

        // Sunrise with visible moon.
		if (false) {
            mCaelumSystem->getUniversalClock ()->setGregorianDateTime (2007, 4, 9, 23, 33, 0);
            mCaelumSystem->getSolarSystemModel ()->setObserverLongitude (
                    Ogre::Degree(45));
            mCaelumSystem->getSolarSystemModel ()->setObserverLatitude (
                    Ogre::Degree(45));
		}

	mCaelumSystem->getSun()->getMainLight()->setCastShadows(true);*/


	mOneTime = 0;

};

void SGTWeatherController::SetGroundFogEnabled (bool enable)
{
    if (enable == GetGroundFogEnabled ())
	{
        return;
    }
    if (enable)
	{
		mCaelumSystem->setGroundFog (new Caelum::GroundFog (SGTMain::Instance().GetOgreSceneMgr(), mCaelumSystem->getCaelumGroundNode ()));
        mCaelumSystem->getGroundFog ()->findFogPassesByName ();
    }
	else
	{
		mCaelumSystem->setGroundFog (NULL);
    }
}

bool SGTWeatherController::GetGroundFogEnabled ()
{
	return mCaelumSystem->getGroundFog() != NULL;
}

SGTWeatherController::~SGTWeatherController(void)
{
	SGTMain::Instance().GetWindow()->removeListener (mCaelumSystem);
	mCaelumSystem->shutdown (false);
	SGTMessageSystem::Instance().QuitNewsgroup(this, "UPDATE_PER_FRAME");
	SGTMessageSystem::Instance().QuitNewsgroup(this, "KEY_UP");
};

Caelum::CaelumSystem* SGTWeatherController::GetCaelumSystem()
{
	return mCaelumSystem;
};

void SGTWeatherController::SetSpeedFactor(double speed)
{
	mSpeedFactor = speed;
    mCaelumSystem->getUniversalClock ()->setTimeScale (mPaused ? 0 : mSpeedFactor);
    Ogre::ControllerManager::getSingleton().setTimeFactor (mPaused ? 0 : mSpeedFactor);
};


void SGTWeatherController::Update()
{
        // Update the haze sun light position
        /*if (GetHazeEnabled() && mCaelumSystem->getSun ())
		{
            Ogre::MaterialPtr mat = static_cast<Ogre::MaterialPtr>(Ogre::MaterialManager::getSingleton ().getByName ("CaelumDemoTerrain1"));
            if (mat.isNull() == false && mat->getTechnique(0) && mat->getTechnique (0)->getPass ("CaelumHaze"))
			{
                mat->getTechnique (0)->getPass ("CaelumHaze")->getVertexProgramParameters ()->setNamedConstant ("sunDirection", mCaelumSystem->getSun ()->getSunDirection ());
            }
        }*/


	/*
	Zufallsgeneriertes Wettersystem:
		- Alle 4 Spielstunden zufallsbedingt schweren Wetterwechsel (Sonnenschein, bewölt, Regen, Gewitter) einleiten
		- Alle halbe Spielstunden leichten Wetterwechsel (Variierung der Wolkendichte) zufallsbedingt einleiten
	*/
};

void SGTWeatherController::ReceiveMessage(SGTMsg &msg)
{
	if (msg.mNewsgroup == "UPDATE_PER_FRAME")
	{
		mCaelumSystem->notifyCameraChanged(SGTMain::Instance().GetCamera());
		mCaelumSystem->updateSubcomponents(msg.mData.GetFloat("TIME"));
		Update();

	/*if (mOneTime == 10)
	{
		Ogre::RenderSystem *psys = Ogre::Root::getSingletonPtr()->getRenderSystem();
        if (psys != NULL)
        {
            Ogre::RenderSystem::RenderTargetIterator iter = psys->getRenderTargetIterator();
            while (iter.hasMoreElements())
            {
                Ogre::RenderTarget *prt = iter.getNext();
				Ogre::LogManager::getSingleton().logMessage("Dumping " + prt->getName());
                prt->writeContentsToFile(prt->getName() + ".bmp");
            }           
        }
		mOneTime++;
	}
	else mOneTime++;*/
	}
}