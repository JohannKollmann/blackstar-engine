
#include "IceGOCView.h"
#include "IceMain.h"
#include "IceGameObject.h"
#include "IceSceneManager.h"

namespace Ice
{

void GOCViewComponentEDT::AttachToGO(GameObject *go)
{
	GOCViewContainer *container = (GOCViewContainer*)go->GetComponent("View");
	if (container != 0)
	{
		if (container->GetComponentID() != "ViewContainer") 
		{
			Ogre::LogManager::getSingleton().logMessage("Error in GOCViewComponentEDT::AttachToGO: Game Object already has another view component!");
			return;
		}
	}
	else
	{
		container = new GOCViewContainer();
		go->AddComponent(container);
	}
	container->RemoveItem(GetTypeName());
	container->AddItem(this);
}

void GOCNodeRenderable::UpdatePosition(Ogre::Vector3 position)
{
	mNode->setPosition(position);
}

void GOCNodeRenderable::UpdateOrientation(Ogre::Quaternion orientation)
{
	mNode->setOrientation(orientation);
}


void GOCViewContainer::UpdateScale(Ogre::Vector3 scale)
{
	mNode->setScale(scale);
}

GOCViewContainer::GOCViewContainer(void)
{
	mNode = Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode();
}

GOCViewContainer::~GOCViewContainer(void)
{
	std::vector<GOCViewComponent*>::iterator i = mItems.begin();
	mNode->detachAllObjects();
	while (i != mItems.end())
	{
		GOCViewComponent *component = (*i);
		mItems.erase(i);
		delete component;
		i = mItems.begin();
	}
	Main::Instance().GetOgreSceneMgr()->destroySceneNode(mNode);
}

void GOCViewContainer::AddItem(GOCViewComponent *item)
{
	item->AttachToNode(mNode);
	mItems.push_back(item);
	if (mOwnerGO)
	{
		item->GetEditorVisual()->setUserAny(Ogre::Any(mOwnerGO));
	}
}

void GOCViewContainer::RemoveItem(Ogre::String type)
{
	for (std::vector<GOCViewComponent*>::iterator i = mItems.begin(); i != mItems.end(); i++)
	{
		if ((*i)->GetTypeName() == type)
		{
			GOCViewComponent *component = (*i);
			mItems.erase(i);
			delete component;
			return;
		}
	}
}

void GOCViewContainer::SetOwner(GameObject *go)
{
	mOwnerGO = go;
	mNode->setScale(mOwnerGO->GetGlobalScale());
	for (std::vector<GOCViewComponent*>::iterator i = mItems.begin(); i != mItems.end(); i++)
	{
		(*i)->GetEditorVisual()->setUserAny(Ogre::Any(mOwnerGO));
	}
}

GOCViewComponent* GOCViewContainer::GetItem(Ogre::String type)
{
	for (std::vector<GOCViewComponent*>::iterator i = mItems.begin(); i != mItems.end(); i++)
	{
		if (Ogre::String((*i)->GetTypeName().c_str()) == type)
		{
			return (*i);
		}
	}
	return 0;
}


void GOCViewContainer::ShowEditorVisual(bool show)
{
	for (std::vector<GOCViewComponent*>::iterator i = mItems.begin(); i != mItems.end(); i++)
	{
		(*i)->ShowEditorVisual(show);
	}
}

void GOCViewContainer::Save(LoadSave::SaveSystem& mgr)
{
	mgr.SaveAtom("std::vector<Saveable*>", (void*)(&mItems), "mItems");
}

void GOCViewContainer::Load(LoadSave::LoadSystem& mgr)
{
		std::list<GOCViewComponent*> managed;
		mgr.LoadAtom("std::list<Saveable*>", (void*)(&managed));
		for (auto i = managed.begin(); i != managed.end(); i++)
			mItems.push_back(*i);

	//mgr.LoadAtom("std::vector<Saveable*>", (void*)(&mItems));
	for (std::vector<GOCViewComponent*>::iterator i = mItems.begin(); i != mItems.end(); i++)
	{
		(*i)->AttachToNode(mNode);
	}
}

MeshRenderable::MeshRenderable(Ogre::String meshname, bool shadowcaster)
{
	Create(meshname, shadowcaster);
}

MeshRenderable::~MeshRenderable()
{
	Main::Instance().GetOgreSceneMgr()->destroyEntity(mEntity);
}

void MeshRenderable::Create(Ogre::String meshname, bool shadowcaster)
{
	if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", meshname))
	{
		Ogre::LogManager::getSingleton().logMessage("Error: Resource \"" + meshname + "\" does not exist. Loading dummy Resource...");
		meshname = "DummyMesh.mesh";
	}
	mEntity = Main::Instance().GetOgreSceneMgr()->createEntity(Ogre::StringConverter::toString(SceneManager::Instance().RequestID()), meshname);
	mEntity->setCastShadows(shadowcaster);
}

void MeshRenderable::AttachToNode(Ogre::SceneNode *node)
{
	node->attachObject(mEntity);
}

void MeshRenderable::CreateFromDataMap(DataMap *parameters)
{
	Ogre::String meshname = parameters->GetOgreString("MeshName");
	bool shacowcaster = parameters->GetBool("ShadowCaster");
	Create(meshname, shacowcaster);
}

void MeshRenderable::GetParameters(DataMap *parameters)
{
	parameters->AddOgreString("MeshName", mEntity->getMesh()->getName());
	parameters->AddBool("ShadowCaster", mEntity->getCastShadows());
}

void MeshRenderable::GetDefaultParameters(DataMap *parameters)
{
	parameters->AddOgreString("MeshName", "");
	parameters->AddBool("ShadowCaster", true);
}

void MeshRenderable::Save(LoadSave::SaveSystem& mgr)
{
	mgr.SaveAtom("Ogre::String", (void*)&mEntity->getMesh()->getName(), "MeshName");
	bool shadow = mEntity->getCastShadows();
	mgr.SaveAtom("bool", (void*)&shadow, "ShadowCaster");
}

void MeshRenderable::Load(LoadSave::LoadSystem& mgr)
{ 
	Ogre::String meshname;
	bool shadow = true;
	mgr.LoadAtom("Ogre::String", &meshname);
	mgr.LoadAtom("bool", &shadow);
	Create(meshname, shadow);
}

LoadSave::Saveable* MeshRenderable::NewInstance()
{
	MeshRenderable *meshr = new MeshRenderable();
	return meshr;
}

MeshDebugRenderable::MeshDebugRenderable(Ogre::String meshname)
{
	if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", meshname))
	{
		Ogre::LogManager::getSingleton().logMessage("Error: Resource \"" + meshname + "\" does not exist. Loading dummy Resource...");
		meshname = "DummyMesh.mesh";
	}
	mNode = Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode();
	mEntity = Main::Instance().GetOgreSceneMgr()->createEntity("DebugRenderable"+SceneManager::Instance().RequestIDStr(), meshname);
	mEntity->setCastShadows(false);
	mNode->attachObject(mEntity);
}
MeshDebugRenderable::~MeshDebugRenderable()
{
	Main::Instance().GetOgreSceneMgr()->destroyEntity(mEntity);
	Main::Instance().GetOgreSceneMgr()->destroySceneNode(mNode);
}
void MeshDebugRenderable::SetOwner(GameObject *go)
{
	mOwnerGO = go;
	mEntity->setUserAny(Ogre::Any(mOwnerGO));
}



//PFX
PfxRenderable::PfxRenderable(Ogre::String pfxresname)
{
	Create(pfxresname);
}

PfxRenderable::~PfxRenderable()
{
	Main::Instance().GetOgreSceneMgr()->destroyParticleSystem(mParticleSystem);
	Main::Instance().GetOgreSceneMgr()->destroyEntity(mEditorVisual);
}

void PfxRenderable::Create(Ogre::String pfxresname)
{
	mParticleResource = pfxresname;
	int id = SceneManager::Instance().RequestID();
	mParticleSystem = Main::Instance().GetOgreSceneMgr()->createParticleSystem(Ogre::StringConverter::toString(id), pfxresname);

	mEditorVisual = Main::Instance().GetOgreSceneMgr()->createEntity(Ogre::StringConverter::toString(id) + "_EditorVisual", "Editor_Particle.mesh");
	mEditorVisual->setCastShadows(false);
}

void PfxRenderable::AttachToNode(Ogre::SceneNode *node)
{
	node->attachObject(mParticleSystem);
	node->attachObject(mEditorVisual);
}

void PfxRenderable::ShowEditorVisual(bool show)
{
	mEditorVisual->setVisible(show);
}

void PfxRenderable::CreateFromDataMap(DataMap *parameters)
{
	mParticleResource = parameters->GetOgreString("PfxResourcename");
	Create(mParticleResource);
}

void PfxRenderable::GetParameters(DataMap *parameters)
{
	parameters->AddOgreString("PfxResourcename", mParticleResource);
}

void PfxRenderable::GetDefaultParameters(DataMap *parameters)
{
	parameters->AddOgreString("PfxResourcename", "");
}

void PfxRenderable::Save(LoadSave::SaveSystem& mgr)
{
	mgr.SaveAtom("Ogre::String", (void*)&mParticleResource, "PfxResourcename");
}

void PfxRenderable::Load(LoadSave::LoadSystem& mgr)
{ 
	mgr.LoadAtom("Ogre::String", &mParticleResource);
	Create(mParticleResource);
}


//Sound
Sound3D::Sound3D(Ogre::String audiofile, float referenceDistance, float maxDistance, bool streamed, bool looped, bool preBuffered)
{
	Create(audiofile, referenceDistance, maxDistance, streamed, looped, preBuffered);
}

Sound3D::~Sound3D()
{
	if (mSound)
	{
		mSound->stop();
		Main::Instance().GetSoundManager()->destroySound(mSound->getName());
	}
	Main::Instance().GetOgreSceneMgr()->destroyEntity(mEditorVisual);
}

void Sound3D::Create(Ogre::String audiofile, float referenceDistance, float maxDistance, bool streamed, bool looped, bool preBuffered)
{
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
	}

	mEditorVisual = Main::Instance().GetOgreSceneMgr()->createEntity(Ogre::StringConverter::toString(id) + "_EditorVisual", "Editor_Sound.mesh");
	mEditorVisual->setCastShadows(false);
}

void Sound3D::AttachToNode(Ogre::SceneNode *node)
{
	if (mSound) node->attachObject(mSound);
	node->attachObject(mEditorVisual);
}

void Sound3D::ShowEditorVisual(bool show)
{
	mEditorVisual->setVisible(show);
}

void Sound3D::CreateFromDataMap(DataMap *parameters)
{
	mAudioFile = parameters->GetOgreString("AudioFile");
	mReferenceDistance = parameters->GetFloat("ReferenceDistance");
	mMaxDistance = parameters->GetFloat("MaxSoundRange");
	mStreamed = parameters->GetBool("Streamed");
	mLooped = parameters->GetBool("Looped");
	mPrebuffered = parameters->GetBool("Prebuffered");
	Create(mAudioFile, mReferenceDistance, mMaxDistance, mStreamed, mLooped, mPrebuffered);
}

void Sound3D::GetParameters(DataMap *parameters)
{
	parameters->AddOgreString("AudioFile", mAudioFile);
	parameters->AddFloat("ReferenceDistance", mReferenceDistance);
	parameters->AddFloat("MaxSoundRange", mMaxDistance);
	parameters->AddBool("Streamed", mStreamed);
	parameters->AddBool("Looped", mLooped);
	parameters->AddBool("Prebuffered", mPrebuffered);
}

void Sound3D::GetDefaultParameters(DataMap *parameters)
{
	parameters->AddOgreString("AudioFile", "");
	parameters->AddFloat("ReferenceDistance", 10.0f);
	parameters->AddFloat("MaxSoundRange", 50.0f);
	parameters->AddBool("Streamed", true);
	parameters->AddBool("Looped", true);
	parameters->AddBool("Prebuffered", true);
}

void Sound3D::Save(LoadSave::SaveSystem& mgr)
{
	mgr.SaveAtom("Ogre::String", (void*)&mAudioFile, "AudioFile");
	mgr.SaveAtom("float", (void*)&mReferenceDistance, "ReferenceDistance");
	mgr.SaveAtom("float", (void*)&mMaxDistance, "MaxSoundRange");
	mgr.SaveAtom("bool", (void*)&mStreamed, "Streamed");
	mgr.SaveAtom("bool", (void*)&mLooped, "Looped");
	mgr.SaveAtom("bool", (void*)&mPrebuffered, "Prebuffered");
}

void Sound3D::Load(LoadSave::LoadSystem& mgr)
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
LocalLightRenderable::LocalLightRenderable(Ogre::ColourValue diffuse, Ogre::ColourValue specular, bool spot, bool shadow_caster, float maxDistance, Ogre::Vector3 attenuation, Ogre::Vector3 spotlightdata, float flickerSpeed, float flickerIntensity)
{
	Create(diffuse, specular, shadow_caster, spot, maxDistance, attenuation, spotlightdata, flickerSpeed, flickerIntensity);
}

LocalLightRenderable::~LocalLightRenderable()
{
	Main::Instance().GetOgreSceneMgr()->destroyLight(mLight);
	Main::Instance().GetOgreSceneMgr()->destroyEntity(mEditorVisual);
}

void LocalLightRenderable::Create(Ogre::ColourValue diffuse, Ogre::ColourValue specular, bool spot, bool shadow_caster, float maxDistance, Ogre::Vector3 attenuation, Ogre::Vector3 spotlightdata, float flickerSpeed, float flickerIntensity)
{
	mOriginalColour = diffuse;
	mFlickerSpeed = flickerSpeed;
	mFlickerIntensity = flickerIntensity;
	mFlickerOffset = Ogre::Math::RangeRandom(1, 100);
	int id = SceneManager::Instance().RequestID();
	mLight = Main::Instance().GetOgreSceneMgr()->createLight(Ogre::StringConverter::toString(id));
	Ogre::String editormeshname = "";
	if (spot)
	{
		editormeshname = "Editor_DirectionalLight.mesh";
		mLight->setType(Ogre::Light::LightTypes::LT_SPOTLIGHT);
		mLight->setCustomShadowCameraSetup(Main::Instance().GetDirectionalShadowCameraSetup());
		mLight->setSpotlightRange(Ogre::Radian(Ogre::Degree(spotlightdata.x)), Ogre::Radian(Ogre::Degree(spotlightdata.y)), spotlightdata.z);
	}
	else
	{
		editormeshname = "Editor_Omnilight.mesh";
		mLight->setType(Ogre::Light::LightTypes::LT_POINT);
	}
	mLight->setCastShadows(shadow_caster);
	mLight->setAttenuation(maxDistance, attenuation.x, attenuation.y, attenuation.z);

	mEditorVisual = Main::Instance().GetOgreSceneMgr()->createEntity(Ogre::StringConverter::toString(id) + "_EditorVisual", editormeshname);
	mEditorVisual->setCastShadows(false);
}

void LocalLightRenderable::AttachToNode(Ogre::SceneNode *node)
{
	node->attachObject(mLight);
	node->attachObject(mEditorVisual);
}

void LocalLightRenderable::ShowEditorVisual(bool show)
{
	mEditorVisual->setVisible(show);
}

void LocalLightRenderable::CreateFromDataMap(DataMap *parameters)
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

void LocalLightRenderable::GetParameters(DataMap *parameters)
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

void LocalLightRenderable::GetDefaultParameters(DataMap *parameters)
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

void LocalLightRenderable::Save(LoadSave::SaveSystem& mgr)
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

void LocalLightRenderable::Load(LoadSave::LoadSystem& mgr)
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