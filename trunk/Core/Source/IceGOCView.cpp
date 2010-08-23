
#include "IceGOCView.h"
#include "IceMain.h"
#include "IceGameObject.h"
#include "IceSceneManager.h"

namespace Ice
{

	GOCEditorVisualised::~GOCEditorVisualised()
	{
		if (mEditorVisual)
		{
			GetNode()->detachObject(mEditorVisual);
			Main::Instance().GetOgreSceneMgr()->destroyEntity(mEditorVisual);
			mEditorVisual = nullptr;
		}
	}
	void GOCEditorVisualised::ShowEditorVisual(bool show)
	{
		if (show && !mEditorVisual)
		{
			mEditorVisual = Main::Instance().GetOgreSceneMgr()->createEntity(GetEditorVisualMeshName());
			mEditorVisual->setUserAny(Ogre::Any(mOwnerGO));
			GetNode()->attachObject(mEditorVisual);
		}
		else if (!show && mEditorVisual)
		{
			GetNode()->detachObject(mEditorVisual);
			Main::Instance().GetOgreSceneMgr()->destroyEntity(mEditorVisual);
			mEditorVisual = nullptr;
		}
	}


	GOCMeshRenderable::GOCMeshRenderable(Ogre::String meshname, bool shadowcaster)
	{
		mEntity = nullptr;
		Create(meshname, shadowcaster);
	}

	GOCMeshRenderable::~GOCMeshRenderable()
	{
		_clear();
	}
	void GOCMeshRenderable::_clear()
	{
		if (mEntity)
		{
			if (mOwnerGO) GetNode()->detachObject(mEntity);
			Main::Instance().GetOgreSceneMgr()->destroyEntity(mEntity);
		}
		mEntity = nullptr;
	}

	void GOCMeshRenderable::Create(Ogre::String meshname, bool shadowcaster)
	{
		_clear();
		if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", meshname))
		{
			Ogre::LogManager::getSingleton().logMessage("Error: Resource \"" + meshname + "\" does not exist. Loading dummy Resource...");
			meshname = "DummyMesh.mesh";
		}
		mEntity = Main::Instance().GetOgreSceneMgr()->createEntity(Ogre::StringConverter::toString(SceneManager::Instance().RequestID()), meshname);
		mEntity->setCastShadows(shadowcaster);
		NotifyOwnerGO();
	}

	void GOCMeshRenderable::NotifyOwnerGO()
	{
		if (mEntity && mOwnerGO)
		{
			mEntity->setUserAny(Ogre::Any(mOwnerGO));
			GetNode()->attachObject(mEntity);
		}
	}

	void GOCMeshRenderable::SetParameters(DataMap *parameters)
	{
		Ogre::String meshname = parameters->GetOgreString("MeshName");
		bool shacowcaster = parameters->GetBool("ShadowCaster");
		Create(meshname, shacowcaster);
	}

	void GOCMeshRenderable::GetParameters(DataMap *parameters)
	{
		parameters->AddOgreString("MeshName", mEntity->getMesh()->getName());
		parameters->AddBool("ShadowCaster", mEntity->getCastShadows());
	}

	void GOCMeshRenderable::GetDefaultParameters(DataMap *parameters)
	{
		parameters->AddOgreString("MeshName", "");
		parameters->AddBool("ShadowCaster", true);
	}

	void GOCMeshRenderable::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("Ogre::String", (void*)&mEntity->getMesh()->getName(), "MeshName");
		bool shadow = mEntity->getCastShadows();
		mgr.SaveAtom("bool", (void*)&shadow, "ShadowCaster");
	}

	void GOCMeshRenderable::Load(LoadSave::LoadSystem& mgr)
	{ 
		Ogre::String meshname;
		bool shadow = true;
		mgr.LoadAtom("Ogre::String", &meshname);
		mgr.LoadAtom("bool", &shadow);
		Create(meshname, shadow);
	}

	LoadSave::Saveable* GOCMeshRenderable::NewInstance()
	{
		GOCMeshRenderable *meshr = new GOCMeshRenderable();
		return meshr;
	}

	//PFX
	GOCPfxRenderable::GOCPfxRenderable(Ogre::String pfxresname)
	{
		mParticleSystem = nullptr;
		Create(pfxresname);
	}

	GOCPfxRenderable::~GOCPfxRenderable()
	{
		_clear();
	}
	void GOCPfxRenderable::_clear()
	{
		if (mParticleSystem)
		{
			if (mOwnerGO) GetNode()->detachObject(mParticleSystem);
			Main::Instance().GetOgreSceneMgr()->destroyParticleSystem(mParticleSystem);
		}
		mParticleSystem = nullptr;
	}

	void GOCPfxRenderable::Create(Ogre::String pfxresname)
	{
		_clear();
		mParticleResource = pfxresname;
		int id = SceneManager::Instance().RequestID();
		if (!Ogre::ParticleSystemManager::getSingleton().getTemplate(mParticleResource))
		{
			Ogre::LogManager::getSingleton().logMessage("Error: Resource \"" + mParticleResource + "\" does not exist. Loading dummy Resource...");
			mParticleResource = "DummyParticle";
		}
		else
		{
			mParticleSystem = Main::Instance().GetOgreSceneMgr()->createParticleSystem(Ogre::StringConverter::toString(id), pfxresname);
			NotifyOwnerGO();
		}
	}

	void GOCPfxRenderable::NotifyOwnerGO()
	{
		if (mParticleSystem && mOwnerGO) GetNode()->attachObject(mParticleSystem);
	}

	void GOCPfxRenderable::SetParameters(DataMap *parameters)
	{
		mParticleResource = parameters->GetOgreString("PfxResourcename");
		Create(mParticleResource);
	}

	void GOCPfxRenderable::GetParameters(DataMap *parameters)
	{
		parameters->AddOgreString("PfxResourcename", mParticleResource);
	}

	void GOCPfxRenderable::GetDefaultParameters(DataMap *parameters)
	{
		parameters->AddOgreString("PfxResourcename", "");
	}

	void GOCPfxRenderable::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("Ogre::String", (void*)&mParticleResource, "PfxResourcename");
	}

	void GOCPfxRenderable::Load(LoadSave::LoadSystem& mgr)
	{ 
		mgr.LoadAtom("Ogre::String", &mParticleResource);
		Create(mParticleResource);
	}


	//Sound
	GOCSound3D::GOCSound3D(Ogre::String audiofile, float referenceDistance, float maxDistance, bool streamed, bool looped, bool preBuffered)
	{
		mSound = nullptr;
		Create(audiofile, referenceDistance, maxDistance, streamed, looped, preBuffered);
	}

	GOCSound3D::~GOCSound3D()
	{
		_clear();
	}
	void GOCSound3D::_clear()
	{
		if (mSound)
		{
			mSound->stop();
			if (mOwnerGO) GetNode()->detachObject(mSound);
			Main::Instance().GetSoundManager()->destroySound(mSound->getName());
		}
		mSound = nullptr;
	}

	void GOCSound3D::Create(Ogre::String audiofile, float referenceDistance, float maxDistance, bool streamed, bool looped, bool preBuffered)
	{
		_clear();
		mAudioFile = audiofile;
		mReferenceDistance = referenceDistance;
		mMaxDistance = maxDistance;
		mStreamed = streamed;
		mLooped = looped;
		mPrebuffered = preBuffered;
		int id = SceneManager::Instance().RequestID();
		mSound = Main::Instance().GetSoundManager()->createSound(Ogre::StringConverter::toString(id), mAudioFile.c_str(), mStreamed, mLooped, mPrebuffered);
		if (mSound)
		{
			mSound->setMaxDistance(mMaxDistance);
			mSound->setReferenceDistance(mReferenceDistance);
			mSound->play();
			NotifyOwnerGO();
		}
	}

	void GOCSound3D::NotifyOwnerGO()
	{
		if (mSound && mOwnerGO) GetNode()->attachObject(mSound);
	}

	void GOCSound3D::SetParameters(DataMap *parameters)
	{
		mAudioFile = parameters->GetOgreString("AudioFile");
		mReferenceDistance = parameters->GetFloat("ReferenceDistance");
		mMaxDistance = parameters->GetFloat("MaxSoundRange");
		mStreamed = parameters->GetBool("Streamed");
		mLooped = parameters->GetBool("Looped");
		mPrebuffered = parameters->GetBool("Prebuffered");
		Create(mAudioFile, mReferenceDistance, mMaxDistance, mStreamed, mLooped, mPrebuffered);
	}

	void GOCSound3D::GetParameters(DataMap *parameters)
	{
		parameters->AddOgreString("AudioFile", mAudioFile);
		parameters->AddFloat("ReferenceDistance", mReferenceDistance);
		parameters->AddFloat("MaxSoundRange", mMaxDistance);
		parameters->AddBool("Streamed", mStreamed);
		parameters->AddBool("Looped", mLooped);
		parameters->AddBool("Prebuffered", mPrebuffered);
	}

	void GOCSound3D::GetDefaultParameters(DataMap *parameters)
	{
		parameters->AddOgreString("AudioFile", "");
		parameters->AddFloat("ReferenceDistance", 10.0f);
		parameters->AddFloat("MaxSoundRange", 50.0f);
		parameters->AddBool("Streamed", true);
		parameters->AddBool("Looped", true);
		parameters->AddBool("Prebuffered", true);
	}

	void GOCSound3D::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("Ogre::String", (void*)&mAudioFile, "AudioFile");
		mgr.SaveAtom("float", (void*)&mReferenceDistance, "ReferenceDistance");
		mgr.SaveAtom("float", (void*)&mMaxDistance, "MaxSoundRange");
		mgr.SaveAtom("bool", (void*)&mStreamed, "Streamed");
		mgr.SaveAtom("bool", (void*)&mLooped, "Looped");
		mgr.SaveAtom("bool", (void*)&mPrebuffered, "Prebuffered");
	}

	void GOCSound3D::Load(LoadSave::LoadSystem& mgr)
	{ 
		mgr.LoadAtom("Ogre::String", (void*)&mAudioFile);
		mgr.LoadAtom("float", (void*)&mReferenceDistance);
		mgr.LoadAtom("float", (void*)&mMaxDistance);
		mgr.LoadAtom("bool", (void*)&mStreamed);
		mgr.LoadAtom("bool", (void*)&mLooped);
		mgr.LoadAtom("bool", (void*)&mPrebuffered);
		Create(mAudioFile, mReferenceDistance, mMaxDistance, mStreamed, mLooped, mPrebuffered);
	}


	//Light
	GOCLocalLightRenderable::GOCLocalLightRenderable(Ogre::ColourValue diffuse, Ogre::ColourValue specular, bool spot, bool shadow_caster, float maxDistance, Ogre::Vector3 attenuation, Ogre::Vector3 spotlightdata, float flickerSpeed, float flickerIntensity)
	{
		mLight = nullptr;
		Create(diffuse, specular, shadow_caster, spot, maxDistance, attenuation, spotlightdata, flickerSpeed, flickerIntensity);
	}

	GOCLocalLightRenderable::~GOCLocalLightRenderable()
	{
		_clear();
	}
	void GOCLocalLightRenderable::_clear()
	{
		if (mLight)
		{
			if (mOwnerGO) GetNode()->detachObject(mLight);
			Main::Instance().GetOgreSceneMgr()->destroyLight(mLight);
		}
		mLight = nullptr;
	}

	void GOCLocalLightRenderable::Create(Ogre::ColourValue diffuse, Ogre::ColourValue specular, bool spot, bool shadow_caster, float maxDistance, Ogre::Vector3 attenuation, Ogre::Vector3 spotlightdata, float flickerSpeed, float flickerIntensity)
	{
		_clear();
		mOriginalColour = diffuse;
		mFlickerSpeed = flickerSpeed;
		mFlickerIntensity = flickerIntensity;
		mFlickerOffset = Ogre::Math::RangeRandom(1, 100);
		int id = SceneManager::Instance().RequestID();
		mLight = Main::Instance().GetOgreSceneMgr()->createLight(Ogre::StringConverter::toString(id));
		if (spot)
		{
			mEditorMeshName = "Editor_DirectionalLight.mesh";
			mLight->setType(Ogre::Light::LightTypes::LT_SPOTLIGHT);
			mLight->setCustomShadowCameraSetup(Main::Instance().GetDirectionalShadowCameraSetup());
			mLight->setSpotlightRange(Ogre::Radian(Ogre::Degree(spotlightdata.x)), Ogre::Radian(Ogre::Degree(spotlightdata.y)), spotlightdata.z);
		}
		else
		{
			mEditorMeshName = "Editor_Omnilight.mesh";
			mLight->setType(Ogre::Light::LightTypes::LT_POINT);
		}
		mLight->setCastShadows(shadow_caster);
		mLight->setAttenuation(maxDistance, attenuation.x, attenuation.y, attenuation.z);
		NotifyOwnerGO();
	}

	void GOCLocalLightRenderable::NotifyOwnerGO()
	{
		if (mLight && mOwnerGO) GetNode()->attachObject(mLight);
	}

	void GOCLocalLightRenderable::SetParameters(DataMap *parameters)
	{
		bool spot = parameters->GetBool("Spotlight");
		Ogre::Vector3 diffuse = parameters->GetOgreVec3("Diffuse");
		Ogre::Vector3 specular = parameters->GetOgreVec3("Specular");
		bool shadow = parameters->GetBool("ShadowCaster");
		float maxDist = parameters->GetFloat("MaxLightRange");
		Ogre::Vector3 atten = parameters->GetOgreVec3("Attenuation");
		Ogre::Vector3 spotdata = parameters->GetOgreVec3("SpotLightData");
		mFlickerSpeed = parameters->GetFloat("FlickerSpeed");
		mFlickerIntensity = parameters->GetFloat("FlickerIntensity");
		Create(Ogre::ColourValue(diffuse.x, diffuse.y, diffuse.z), Ogre::ColourValue(specular.x, specular.y, specular.z), spot, shadow, maxDist, atten, spotdata, mFlickerSpeed, mFlickerSpeed);
	}

	void GOCLocalLightRenderable::GetParameters(DataMap *parameters)
	{
		parameters->AddOgreVec3("Diffuse", Ogre::Vector3(mOriginalColour.r, mOriginalColour.g, mOriginalColour.b));
		parameters->AddOgreVec3("Specular", Ogre::Vector3(mLight->getSpecularColour().r, mLight->getSpecularColour().g, mLight->getSpecularColour().b));
		parameters->AddBool("ShadowCaster", mLight->getCastShadows());
		parameters->AddFloat("MaxLightRange", mLight->getAttenuationRange());
		parameters->AddOgreVec3("Attenuation", Ogre::Vector3(mLight->getAttenuationConstant(), mLight->getAttenuationLinear(), mLight->getAttenuationQuadric()));
		if (mLight->getType() == Ogre::Light::LightTypes::LT_SPOTLIGHT) parameters->AddBool("Spotlight", true);
		else parameters->AddBool("Spotlight", false);
		parameters->AddOgreVec3("SpotLightData", Ogre::Vector3(mLight->getSpotlightInnerAngle().valueDegrees(), mLight->getSpotlightOuterAngle().valueDegrees(), mLight->getSpotlightFalloff()));
		parameters->AddFloat("FlickerIntensity", mFlickerIntensity);
		parameters->AddFloat("FlickerSpeed", mFlickerSpeed);
	}

	void GOCLocalLightRenderable::GetDefaultParameters(DataMap *parameters)
	{
		parameters->AddOgreVec3("Diffuse", Ogre::Vector3(0,0,0));
		parameters->AddOgreVec3("Specular", Ogre::Vector3(0,0,0));;
		parameters->AddBool("ShadowCaster", true);
		parameters->AddFloat("MaxLightRange", 200);
		parameters->AddOgreVec3("Attenuation", Ogre::Vector3(1,0.01,0.0005));
		parameters->AddBool("Spotlight", true);
		parameters->AddOgreVec3("SpotLightData", Ogre::Vector3(40,50,0.5));
		parameters->AddFloat("FlickerIntensity", 0.0f);
		parameters->AddFloat("FlickerSpeed", 0.0f);
	}

	void GOCLocalLightRenderable::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("Ogre::Vector3", (void*)&Ogre::Vector3(mOriginalColour.r, mOriginalColour.g, mOriginalColour.b), "Diffuse");
		mgr.SaveAtom("Ogre::Vector3", (void*)&Ogre::Vector3(mLight->getSpecularColour().r, mLight->getSpecularColour().g, mLight->getSpecularColour().b), "Specular");
		bool shadow = mLight->getCastShadows();
		bool spot = mLight->getType() == Ogre::Light::LightTypes::LT_SPOTLIGHT;
		float range = mLight->getAttenuationRange();
		mgr.SaveAtom("bool", (void*)&shadow, "ShadowCaster");
		mgr.SaveAtom("float", (void*)&range, "MaxLightRange");
		mgr.SaveAtom("Ogre::Vector3", (void*)&Ogre::Vector3(mLight->getAttenuationConstant(), mLight->getAttenuationLinear(), mLight->getAttenuationQuadric()), "Attenuation");
		mgr.SaveAtom("bool", (void*)&spot, "Spotlight");
		mgr.SaveAtom("Ogre::Vector3", (void*)&Ogre::Vector3(mLight->getSpotlightInnerAngle().valueDegrees(), mLight->getSpotlightOuterAngle().valueDegrees(), mLight->getSpotlightFalloff()), "SpotlightData");
		mgr.SaveAtom("float", (void*)&mFlickerIntensity, "FlickerIntensity");
		mgr.SaveAtom("float", (void*)&mFlickerSpeed, "FlickerSpeed");
	}

	void GOCLocalLightRenderable::Load(LoadSave::LoadSystem& mgr)
	{
		bool spot = true;
		Ogre::Vector3 diffuse;
		Ogre::Vector3 specular;
		bool shadow = false;
		float maxDist = 200.0f;
		Ogre::Vector3 atten;
		Ogre::Vector3 spotdata;
		mgr.LoadAtom("Ogre::Vector3", (void*)&diffuse);
		mgr.LoadAtom("Ogre::Vector3", (void*)&specular);
		mgr.LoadAtom("bool", (void*)&shadow);
		mgr.LoadAtom("float", (void*)&maxDist);
		mgr.LoadAtom("Ogre::Vector3", (void*)&atten);
		mgr.LoadAtom("bool", (void*)&spot);
		mgr.LoadAtom("Ogre::Vector3", (void*)&spotdata);
		mgr.LoadAtom("float", (void*)&mFlickerIntensity);
		mgr.LoadAtom("float", (void*)&mFlickerSpeed);
		Create(Ogre::ColourValue(diffuse.x, diffuse.y, diffuse.z), Ogre::ColourValue(specular.x, specular.y, specular.z), spot, shadow, maxDist, atten, spotdata, mFlickerSpeed, mFlickerSpeed);
	}

};