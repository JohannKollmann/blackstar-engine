
#include "IceWeatherController.h"
#include "IceMain.h"
#include "OIS/OIS.h"

namespace Ice
{

	WeatherController::WeatherController(void)
	{
	        // Pick components to create in the demo.
	        // You can comment any of those and it should still work
	        // It makes little sense to comment the first three.
		Caelum::CaelumSystem::CaelumComponent componentMask = static_cast<Caelum::CaelumSystem::CaelumComponent> (0
			| Caelum::CaelumSystem::CaelumComponent::CAELUM_COMPONENT_SKY_DOME
	        | Caelum::CaelumSystem::CaelumComponent::CAELUM_COMPONENT_MOON
	        //| Caelum::CaelumSystem::CaelumComponent::CAELUM_COMPONENT_SUN
	        | Caelum::CaelumSystem::CaelumComponent::CAELUM_COMPONENT_POINT_STARFIELD
	        | Caelum::CaelumSystem::CaelumComponent::CAELUM_COMPONENT_CLOUDS
	        | Caelum::CaelumSystem::CaelumComponent::CAELUM_COMPONENT_PRECIPITATION);
	        //| Caelum::CaelumSystem::CaelumComponent::CAELUM_COMPONENT_SCREEN_SPACE_FOG);
			//| Caelum::CaelumSystem::CaelumComponent::CAELUM_COMPONENT_GROUND_FOG);

		// Initialise Caelum
		mCaelumSystem = new Caelum::CaelumSystem (Ogre::Root::getSingletonPtr(), Main::Instance().GetOgreSceneMgr(), componentMask);//Caelum::CaelumSystem::CAELUM_COMPONENTS_NONE);
		mCaelumSystem->setSun (new Caelum::SpriteSun(Main::Instance().GetOgreSceneMgr(), mCaelumSystem->getCaelumCameraNode (), "sun_disc.png", Ogre::Degree(10)));

		/*mCaelumSystem->setSkyDome (new Caelum::SkyDome (Main::Instance().GetOgreSceneMgr(), mCaelumSystem->getCaelumCameraNode ()));
	    mCaelumSystem->setSun (new Caelum::SphereSun(Main::Instance().GetOgreSceneMgr(), mCaelumSystem->getCaelumCameraNode ()));
	    mCaelumSystem->setMoon (new Caelum::Moon(Main::Instance().GetOgreSceneMgr(), mCaelumSystem->getCaelumCameraNode ()));
	    mCaelumSystem->setCloudSystem (new Caelum::CloudSystem (Main::Instance().GetOgreSceneMgr(), mCaelumSystem->getCaelumGroundNode ()));
	    mCaelumSystem->setPointStarfield (new Caelum::PointStarfield (Main::Instance().GetOgreSceneMgr(), mCaelumSystem->getCaelumCameraNode ()));*/

		mCaelumSystem->attachViewport (Main::Instance().GetViewport());
		/*mCaelumSystem->setPrecipitationController (new Caelum::PrecipitationController (Main::Instance().GetOgreSceneMgr()));
		mCaelumSystem->getPrecipitationController ()->createViewportInstance (Main::Instance().GetCamera()->getViewport ());

	    if (mCaelumSystem->getPrecipitationController ()) {
	        mCaelumSystem->getPrecipitationController ()->setIntensity (0);
		}*/


		mCaelumSystem->getUniversalClock ()->setTimeScale (0);

	    mCaelumSystem->setManageSceneFog(false);
	    //mCaelumSystem->setSceneFogDensityMultiplier(0.0015);
		mCaelumSystem->setMinimumAmbientLight(Ogre::ColourValue(0.1f, 0.1f,0.1f));
	    mCaelumSystem->setManageAmbientLight (true); 

		mDay = 24;
		mMonth = 8;
		mCaelumSystem->getUniversalClock ()->setGregorianDateTime (2010, mMonth, mDay, 11, 0, 0);
		mCaelumSystem->setObserverLatitude(Ogre::Degree(0));
		mCaelumSystem->setObserverLongitude(Ogre::Degree(0));

		mCaelumSystem->getSun ()->setDiffuseMultiplier (Ogre::ColourValue (2.8f, 2.8f, 2.6f));
		mCaelumSystem->getSun ()->setSpecularMultiplier (Ogre::ColourValue (1.5f, 1.5f, 1.5f));
		mCaelumSystem->getSun ()->setAmbientMultiplier(Ogre::ColourValue(0.7f, 0.7f, 0.7f));

		mCaelumSystem->setEnsureSingleShadowSource(true);
		mCaelumSystem->setEnsureSingleLightSource(true);

		mCaelumSystem->getMoon()->setPhase(1.0f);
		mCaelumSystem->getMoon()->setDiffuseMultiplier(Ogre::ColourValue (0.6f, 0.6f, 0.8f));
		mCaelumSystem->getMoon()->setSpecularMultiplier(Ogre::ColourValue (0.5, 0.5, 0.5));
		mCaelumSystem->getMoon ()->setAmbientMultiplier(Ogre::ColourValue(0.25f, 0.25f, 0.25f));

		mPaused = false;

		SetSpeedFactor(mCaelumSystem->getUniversalClock ()->getTimeScale ());

		MessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
		MessageSystem::Instance().JoinNewsgroup(this, "KEY_UP");
	};

	void WeatherController::SetGroundFogEnabled (bool enable)
	{
	    if (enable == GetGroundFogEnabled ())
		{
	        return;
	    }
	    if (enable)
		{
			mCaelumSystem->setGroundFog (new Caelum::GroundFog (Main::Instance().GetOgreSceneMgr(), mCaelumSystem->getCaelumGroundNode ()));
	        mCaelumSystem->getGroundFog ()->findFogPassesByName ();
	    }
		else
		{
			mCaelumSystem->setGroundFog (NULL);
	    }
	}

	bool WeatherController::GetGroundFogEnabled ()
	{
		return mCaelumSystem->getGroundFog() != NULL;
	}

	WeatherController::~WeatherController(void)
	{
		Main::Instance().GetWindow()->removeListener (mCaelumSystem);
		mCaelumSystem->shutdown (false);
		MessageSystem::Instance().QuitNewsgroup(this, "UPDATE_PER_FRAME");
		MessageSystem::Instance().QuitNewsgroup(this, "KEY_UP");
	};

	Caelum::CaelumSystem* WeatherController::GetCaelumSystem()
	{
		return mCaelumSystem;
	};

	void WeatherController::SetSpeedFactor(double speed)
	{
		mSpeedFactor = speed;
	    mCaelumSystem->getUniversalClock ()->setTimeScale (mPaused ? 0 : mSpeedFactor);
	    Ogre::ControllerManager::getSingleton().setTimeFactor (mPaused ? 0 : mSpeedFactor);
	}

	void WeatherController::SetTime(int hour, int minutes)
	{
		mCaelumSystem->getUniversalClock ()->setGregorianDateTime(2010, mMonth, mDay, hour, minutes, 0);
	}


	void WeatherController::Update(float time)
	{
		mCaelumSystem->updateSubcomponents(time);
	};

	void WeatherController::UpdateViewport()
	{
		if (mCaelumSystem->getDepthComposer ())
		{
			Ogre::Viewport *v = Main::Instance().GetViewport();
			Caelum::DepthComposerInstance* inst = mCaelumSystem->getDepthComposer ()->getViewportInstance (v);
			inst->getDepthRenderer()->getDepthRenderViewport()->setDimensions(v->getActualLeft(), v->getActualTop(), v->getActualWidth(), v->getActualHeight());
		}
	}

	void WeatherController::ReceiveMessage(Msg &msg)
	{
		if (msg.type == "UPDATE_PER_FRAME")
		{
			mCaelumSystem->notifyCameraChanged(Main::Instance().GetCamera());
		}
	}

};