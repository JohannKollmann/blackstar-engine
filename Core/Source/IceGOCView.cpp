
#include "IceGOCView.h"
#include "IceMain.h"
#include "IceGameObject.h"
#include "IceSceneManager.h"

namespace Ice
{

	GOCEditorVisualised::~GOCEditorVisualised()
	{
		if (mEditorVisual && !mOwnerGO.expired())
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
			mEditorVisual->setVisibilityFlags(Ice::VisibilityFlags::V_DEFAULT);
			mEditorVisual->setUserAny(Ogre::Any(GetOwner().get()));
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
			//if (mOwnerGO) GetNode()->detachObject(mEntity);
			Main::Instance().GetOgreSceneMgr()->destroyEntity(mEntity);
		}
		mEntity = nullptr;
	}

	void GOCMeshRenderable::Create(Ogre::String meshname, bool shadowcaster)
	{
		_clear();
		if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", meshname))
		{
			Log::Instance().LogMessage("Error: Resource \"" + meshname + "\" does not exist. Loading dummy Resource...");
			meshname = "DummyMesh.mesh";
		}
		mEntity = Main::Instance().GetOgreSceneMgr()->createEntity(Ogre::StringConverter::toString(SceneManager::Instance().RequestID()), meshname);
		mEntity->setCastShadows(shadowcaster);
		unsigned short src, dest;
		try
		{
			if (!mEntity->getMesh()->suggestTangentVectorBuildParams(Ogre::VES_TANGENT, src, dest))
				mEntity->getMesh()->buildTangentVectors(Ogre::VES_TANGENT, src, dest, true, true, true);
		} catch (Ogre::Exception &e) { IceWarning(e.getDescription()) }

		NotifyOwnerGO();
	}

	void GOCMeshRenderable::NotifyOwnerGO()
	{
		if (mEntity && !mOwnerGO.expired())
		{
			mEntity->setUserAny(Ogre::Any(GetOwner().get()));
			GetNode()->attachObject(mEntity);
		}
	}

	void GOCMeshRenderable::SetParameters(DataMap *parameters)
	{
		Ogre::String meshname = parameters->GetValue<Ogre::String>("MeshName", "");
		bool shacowcaster = parameters->GetValue<bool>("ShadowCaster", false);
		Create(meshname, shacowcaster);
		int visibilityMask = parameters->GetValue<int>("Visibility mask", Ice::VisibilityFlags::V_DEFAULT);
		mEntity->setVisibilityFlags(visibilityMask);
	}

	void GOCMeshRenderable::GetParameters(DataMap *parameters)
	{
		parameters->AddOgreString("MeshName", mEntity->getMesh()->getName());
		parameters->AddBool("ShadowCaster", mEntity->getCastShadows());
		parameters->AddInt("Visibility mask", mEntity->getVisibilityFlags());
	}

	void GOCMeshRenderable::GetDefaultParameters(DataMap *parameters)
	{
		parameters->AddOgreString("MeshName", "");
		parameters->AddBool("ShadowCaster", true);
		parameters->AddInt("Visibility mask", Ice::VisibilityFlags::V_DEFAULT);
	}

	void GOCMeshRenderable::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("Ogre::String", (void*)&mEntity->getMesh()->getName(), "MeshName");
		bool shadow = mEntity->getCastShadows();
		mgr.SaveAtom("bool", &shadow, "ShadowCaster");
		int flags = mEntity->getVisibilityFlags();
		mgr.SaveAtom("int", &flags, "Visibility mask");
	}

	void GOCMeshRenderable::Load(LoadSave::LoadSystem& mgr)
	{ 
		Ogre::String meshname;
		bool shadow = true;
		mgr.LoadAtom("Ogre::String", &meshname);
		mgr.LoadAtom("bool", &shadow);
		Create(meshname, shadow);
		int visibilityMask = Ice::VisibilityFlags::V_DEFAULT;
		mgr.LoadAtom("int", &visibilityMask);
		mEntity->setVisibilityFlags(visibilityMask);
	}

	LoadSave::Saveable* GOCMeshRenderable::NewInstance()
	{
		GOCMeshRenderable *meshr = ICE_NEW GOCMeshRenderable();
		return meshr;
	}
	
	std::vector<ScriptParam> GOCMeshRenderable::ReplaceMaterial(Script &caller, std::vector<ScriptParam> &params)
	{
		std::vector<Ice::ScriptParam> errout;
		errout.push_back(Ice::ScriptParam());
		if (!mEntity)
		{
			errout.push_back(Ice::ScriptParam(std::string("mEntity not a valid pointer")));
			return errout;
		}
		//replace material
		for(unsigned int iSubEnt=0; iSubEnt<mEntity->getNumSubEntities(); iSubEnt++)
		{
			Ogre::SubEntity* pSubEnt=mEntity->getSubEntity(iSubEnt);
			//Log::Instance().LogMessage(pSubEnt->getMaterialName());
			if(!pSubEnt->getMaterialName().compare(params[0].getString()))
			{//replace this material
				pSubEnt->setMaterialName(params[1].getString());
				return std::vector<Ice::ScriptParam>();
			}
		}
		errout.push_back(Ice::ScriptParam(std::string("material ") + params[0].getString() + std::string(" not found")));
		return errout;
	}

	//Billboard
	void GOCBillboard::NotifyOwnerGO()
	{
		if (mBillboardSet && !mOwnerGO.expired())
		{
			((Ogre::MovableObject*)mBillboardSet)->setUserAny(Ogre::Any(GetOwner().get()));
			GetNode()->attachObject(mBillboardSet);
		}
	}
	void GOCBillboard::_clear()
	{
		if (mBillboardSet)
		{
			Main::Instance().GetOgreSceneMgr()->destroyBillboardSet(mBillboardSet);
			if (!mMaterial.isNull()) Ogre::MaterialManager::getSingleton().remove(mMaterial->getName());
			mMaterial.setNull();
		}
		mBillboardSet = nullptr;
	}
	void GOCBillboard::_create()
	{
		_clear();
		mBillboardSet = Main::Instance().GetOgreSceneMgr()->createBillboardSet();
		Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().getByName(mMaterialName);
		if (!mat.isNull())
		{
			mMaterial = mat->clone(mat->getName() + SceneManager::Instance().RequestIDStr());
			Ogre::Technique *t = mMaterial->getTechnique("VolumetricBillboard");
			if (t)
			{
				//if (t->getPass(0)->getFragmentProgramParameters()->setN
				try
				{
					t->getPass(0)->getFragmentProgramParameters()->setNamedConstant("particleRadius", mRadius);
					t->getPass(0)->getFragmentProgramParameters()->setNamedConstant("particleDensity", mDensity);
				}
				catch (Ogre::Exception &e) {}
			}

			mBillboardSet->setMaterialName(mMaterial->getName());
			mBillboardSet->setUseAccurateFacing(true);
			//if (mRadius > 0) mDimensions = Ogre::Vector3(mRadius*2, mRadius*2, mRadius*2);
			mBillboardSet->setDefaultDimensions(mDimensions.x, mDimensions.y);
			mBillboardSet->createBillboard(0,0,0);
			mBillboardSet->setVisibilityFlags( Ice::VisibilityFlags::V_VOLUMETRIC);
			NotifyOwnerGO();
		}
		else IceWarning("Material " + mMaterialName + " not found!")
	}
	void GOCBillboard::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("Ogre::String", &mMaterialName, "Material Name");
		mgr.SaveAtom("Ogre::Vector3", &mDimensions, "Dimensions");
	}
	void GOCBillboard::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("Ogre::String", &mMaterialName);
		mgr.LoadAtom("Ogre::Vector3", &mDimensions);
	}
	LoadSave::Saveable* GOCBillboard::NewInstance()
	{
		GOCBillboard *billboard = ICE_NEW GOCBillboard();
		return billboard;
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
			if (!mOwnerGO.expired()) GetNode()->detachObject(mParticleSystem);
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
			Log::Instance().LogMessage("Error: Resource \"" + mParticleResource + "\" does not exist. Loading dummy Resource...");
			mParticleResource = "DummyParticle";
		}
		else
		{
			mParticleSystem = Main::Instance().GetOgreSceneMgr()->createParticleSystem(Ogre::StringConverter::toString(id), pfxresname);
			mParticleSystem->setVisibilityFlags(VisibilityFlags::V_VOLUMETRIC);		//for testing purposes
			NotifyOwnerGO();
		}
	}

	void GOCPfxRenderable::NotifyOwnerGO()
	{
		if (mParticleSystem && !mOwnerGO.expired()) GetNode()->attachObject(mParticleSystem);
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

	std::vector<ScriptParam> GOCPfxRenderable::SetEmitting(Script &caller, std::vector<ScriptParam> &params)
	{
		if (mParticleSystem) mParticleSystem->setEmitting(params[0].getBool());
		SCRIPT_RETURN()
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
			if (!mOwnerGO.expired()) GetNode()->detachObject(mSound);
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
		if (mSound && !mOwnerGO.expired()) GetNode()->attachObject(mSound);
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

	std::vector<ScriptParam> GOCSound3D::StartFade(Script &caller, std::vector<ScriptParam> &params)
	{
		if (mSound) mSound->startFade(params[0].getBool(), params[1].getFloat());
		SCRIPT_RETURN()
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
			if (!mOwnerGO.expired()) GetNode()->detachObject(mLight);
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
		mLight->setDiffuseColour(diffuse);
		mLight->setSpecularColour(specular);
		mLight->setAttenuation(maxDistance, attenuation.x, attenuation.y, attenuation.z);
		NotifyOwnerGO();
	}

	void GOCLocalLightRenderable::NotifyOwnerGO()
	{
		if (mLight && !mOwnerGO.expired()) GetNode()->attachObject(mLight);
	}

	void GOCLocalLightRenderable::SetParameters(DataMap *parameters)
	{
		bool spot = parameters->GetBool("Spotlight");
		Ogre::ColourValue diffuse = parameters->GetOgreCol("Diffuse");
		Ogre::ColourValue specular = parameters->GetOgreCol("Specular");
		bool shadow = parameters->GetBool("ShadowCaster");
		float maxDist = parameters->GetFloat("MaxLightRange");
		Ogre::Vector3 atten = parameters->GetOgreVec3("Attenuation");
		Ogre::Vector3 spotdata = parameters->GetOgreVec3("SpotLightData");
		mFlickerSpeed = parameters->GetFloat("FlickerSpeed");
		mFlickerIntensity = parameters->GetFloat("FlickerIntensity");
		Create(diffuse, specular, spot, shadow, maxDist, atten, spotdata, mFlickerSpeed, mFlickerSpeed);
	}

	void GOCLocalLightRenderable::GetParameters(DataMap *parameters)
	{
		parameters->AddOgreCol("Diffuse", mOriginalColour);
		parameters->AddOgreCol("Specular", mLight->getSpecularColour());
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
		parameters->AddOgreCol("Diffuse", Ogre::ColourValue());
		parameters->AddOgreCol("Specular", Ogre::ColourValue());;
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
