
#include "SGTGOCView.h"
#include "SGTMain.h"
#include "SGTGameObject.h"
#include "SGTSceneManager.h"

void SGTGOCNodeRenderable::ReceiveObjectMessage(Ogre::SharedPtr<SGTObjectMsg> msg)
{
	if (msg->mName == "Update_Transform" || msg->mName == "Update_Position")
	{
		UpdatePosition(msg->mData.GetOgreVec3("Position"));
	}
	if (msg->mName == "Update_Transform" || msg->mName == "Update_Orientation")
	{
		UpdateOrientation(msg->mData.GetOgreQuat("Orientation"));
	}
}

void SGTGOCNodeRenderable::UpdatePosition(Ogre::Vector3 position)
{
	mNode->setPosition(position);
}

void SGTGOCNodeRenderable::UpdateOrientation(Ogre::Quaternion orientation)
{
	mNode->setOrientation(orientation);
}


void SGTGOCViewContainer::UpdateScale(Ogre::Vector3 scale)
{
	mNode->setScale(scale);
}

SGTGOCViewContainer::SGTGOCViewContainer(void)
{
	mNode = SGTMain::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode();
}

SGTGOCViewContainer::~SGTGOCViewContainer(void)
{
	SGTMain::Instance().GetOgreSceneMgr()->destroySceneNode(mNode);
}

void SGTGOCViewContainer::AddItem(SGTGOCViewComponent *item)
{
	item->AttachToNode(mNode);
	mItems.push_back(item);
	if (mOwnerGO)
	{
		item->GetEditorVisual()->setUserObject(mOwnerGO);
	}
}

void SGTGOCViewContainer::RemoveItem(SGTGOCViewComponent* item)
{
	mItems.remove(item);
	delete item;
}

void SGTGOCViewContainer::SetOwner(SGTGameObject *go)
{
	mOwnerGO = go;
	mNode->setScale(mOwnerGO->GetGlobalScale());
	for (std::list<SGTGOCViewComponent*>::iterator i = mItems.begin(); i != mItems.end(); i++)
	{
		(*i)->GetEditorVisual()->setUserObject(mOwnerGO);
	}
}

SGTGOCViewComponent* SGTGOCViewContainer::GetItem(Ogre::String type)
{
	for (std::list<SGTGOCViewComponent*>::iterator i = mItems.begin(); i != mItems.end(); i++)
	{
		if (Ogre::String((*i)->GetTypeName().c_str()) == type)
		{
			return (*i);
		}
	}
	return 0;
}


void SGTGOCViewContainer::ShowEditorVisual(bool show)
{
	for (std::list<SGTGOCViewComponent*>::iterator i = mItems.begin(); i != mItems.end(); i++)
	{
		(*i)->ShowEditorVisual(show);
	}
}

void SGTGOCViewContainer::Save(SGTSaveSystem& mgr)
{
	mgr.SaveAtom("std::list<SGTSaveable*>", (void*)(&mItems), "mItems");
}

void SGTGOCViewContainer::Load(SGTLoadSystem& mgr)
{
	mgr.LoadAtom("std::list<SGTSaveable*>", (void*)(&mItems));
	for (std::list<SGTGOCViewComponent*>::iterator i = mItems.begin(); i != mItems.end(); i++)
	{
		(*i)->AttachToNode(mNode);
	}
}

SGTMeshRenderable::SGTMeshRenderable(Ogre::String meshname, bool shadowcaster)
{
	Create(meshname, shadowcaster);
}

SGTMeshRenderable::~SGTMeshRenderable()
{
	SGTMain::Instance().GetOgreSceneMgr()->destroyEntity(mEntity);
}

void SGTMeshRenderable::Create(Ogre::String meshname, bool shadowcaster)
{
	if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", meshname))
	{
		Ogre::LogManager::getSingleton().logMessage("Error: Resource \"" + meshname + "\" does not exist. Loading dummy Resource...");
		meshname = "DummyMesh.mesh";
	}
	mEntity = SGTMain::Instance().GetOgreSceneMgr()->createEntity(Ogre::StringConverter::toString(SGTSceneManager::Instance().RequestID()), meshname);
	mEntity->setCastShadows(shadowcaster);
}

void SGTMeshRenderable::AttachToNode(Ogre::SceneNode *node)
{
	node->attachObject(mEntity);
}

void SGTMeshRenderable::CreateFromDataMap(SGTDataMap *parameters)
{
	Ogre::String meshname = parameters->GetOgreString("MeshName");
	bool shacowcaster = parameters->GetBool("ShadowCaster");
	Create(meshname, shacowcaster);
}

void SGTMeshRenderable::GetParameters(SGTDataMap *parameters)
{
	parameters->AddOgreString("MeshName", mEntity->getMesh()->getName());
	parameters->AddBool("ShadowCaster", mEntity->getCastShadows());
}

void SGTMeshRenderable::GetDefaultParameters(SGTDataMap *parameters)
{
	parameters->AddOgreString("MeshName", "");
	parameters->AddBool("ShadowCaster", true);
}

void SGTMeshRenderable::Save(SGTSaveSystem& mgr)
{
	mgr.SaveAtom("Ogre::String", (void*)&mEntity->getMesh()->getName(), "MeshName");
	bool shadow = mEntity->getCastShadows();
	mgr.SaveAtom("bool", (void*)&shadow, "ShadowCaster");
}

void SGTMeshRenderable::Load(SGTLoadSystem& mgr)
{ 
	Ogre::String meshname;
	bool shadow = true;
	mgr.LoadAtom("Ogre::String", &meshname);
	mgr.LoadAtom("bool", &shadow);
	Create(meshname, shadow);
}

SGTMeshDebugRenderable::SGTMeshDebugRenderable(Ogre::String meshname)
{
	if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", meshname))
	{
		Ogre::LogManager::getSingleton().logMessage("Error: Resource \"" + meshname + "\" does not exist. Loading dummy Resource...");
		meshname = "DummyMesh.mesh";
	}
	mNode = SGTMain::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode();
	mEntity = SGTMain::Instance().GetOgreSceneMgr()->createEntity(Ogre::StringConverter::toString(SGTSceneManager::Instance().RequestID()), meshname);
	mEntity->setCastShadows(false);
	mNode->attachObject(mEntity);
}
SGTMeshDebugRenderable::~SGTMeshDebugRenderable()
{
	SGTMain::Instance().GetOgreSceneMgr()->destroyEntity(mEntity);
	SGTMain::Instance().GetOgreSceneMgr()->destroySceneNode(mNode);
}
void SGTMeshDebugRenderable::SetOwner(SGTGameObject *go)
{
	mOwnerGO = go;
	mEntity->setUserObject(mOwnerGO);
}



//PFX
SGTPfxRenderable::SGTPfxRenderable(Ogre::String pfxresname)
{
	Create(pfxresname);
}

SGTPfxRenderable::~SGTPfxRenderable()
{
	SGTMain::Instance().GetOgreSceneMgr()->destroyParticleSystem(mParticleSystem);
	SGTMain::Instance().GetOgreSceneMgr()->destroyEntity(mEditorVisual);
}

void SGTPfxRenderable::Create(Ogre::String pfxresname)
{
	mParticleResource = pfxresname;
	int id = SGTSceneManager::Instance().RequestID();
	mParticleSystem = SGTMain::Instance().GetOgreSceneMgr()->createParticleSystem(Ogre::StringConverter::toString(id), pfxresname);

	mEditorVisual = SGTMain::Instance().GetOgreSceneMgr()->createEntity(Ogre::StringConverter::toString(id) + "_EditorVisual", "Editor_Particle.mesh");
	mEditorVisual->setCastShadows(false);
}

void SGTPfxRenderable::AttachToNode(Ogre::SceneNode *node)
{
	node->attachObject(mParticleSystem);
	node->attachObject(mEditorVisual);
}

void SGTPfxRenderable::ShowEditorVisual(bool show)
{
	mEditorVisual->setVisible(show);
}

void SGTPfxRenderable::CreateFromDataMap(SGTDataMap *parameters)
{
	mParticleResource = parameters->GetOgreString("PfxResourcename");
	Create(mParticleResource);
}

void SGTPfxRenderable::GetParameters(SGTDataMap *parameters)
{
	parameters->AddOgreString("PfxResourcename", mParticleResource);
}

void SGTPfxRenderable::GetDefaultParameters(SGTDataMap *parameters)
{
	parameters->AddOgreString("PfxResourcename", "");
}

void SGTPfxRenderable::Save(SGTSaveSystem& mgr)
{
	mgr.SaveAtom("Ogre::String", (void*)&mParticleResource, "PfxResourcename");
}

void SGTPfxRenderable::Load(SGTLoadSystem& mgr)
{ 
	mgr.LoadAtom("Ogre::String", &mParticleResource);
	Create(mParticleResource);
}


//Sound
SGTSound3D::SGTSound3D(Ogre::String audiofile, float referenceDistance, float maxDistance, bool streamed, bool looped, bool preBuffered)
{
	Create(audiofile, referenceDistance, maxDistance, streamed, looped, preBuffered);
}

SGTSound3D::~SGTSound3D()
{
	SGTMain::Instance().GetSoundManager()->destroySound(mSound->getName());
	SGTMain::Instance().GetOgreSceneMgr()->destroyEntity(mEditorVisual);
}

void SGTSound3D::Create(Ogre::String audiofile, float referenceDistance, float maxDistance, bool streamed, bool looped, bool preBuffered)
{
	mAudioFile = audiofile;
	mReferenceDistance = referenceDistance;
	mMaxDistance = maxDistance;
	mStreamed = streamed;
	mLooped = looped;
	mPrebuffered = preBuffered;
	int id = SGTSceneManager::Instance().RequestID();
	mSound = SGTMain::Instance().GetSoundManager()->createSound(Ogre::StringConverter::toString(id), mAudioFile.c_str(), mStreamed, mLooped, mPrebuffered);
	mSound->setMaxDistance(mMaxDistance);
	mSound->setReferenceDistance(mReferenceDistance);
	mSound->play();

	mEditorVisual = SGTMain::Instance().GetOgreSceneMgr()->createEntity(Ogre::StringConverter::toString(id) + "_EditorVisual", "Editor_Sound.mesh");
	mEditorVisual->setCastShadows(false);
}

void SGTSound3D::AttachToNode(Ogre::SceneNode *node)
{
	node->attachObject(mSound);
	node->attachObject(mEditorVisual);
}

void SGTSound3D::ShowEditorVisual(bool show)
{
	mEditorVisual->setVisible(show);
}

void SGTSound3D::CreateFromDataMap(SGTDataMap *parameters)
{
	mAudioFile = parameters->GetOgreString("AudioFile");
	mReferenceDistance = parameters->GetFloat("ReferenceDistance");
	mMaxDistance = parameters->GetFloat("MaxSoundRange");
	mStreamed = parameters->GetBool("Streamed");
	mLooped = parameters->GetBool("Looped");
	mPrebuffered = parameters->GetBool("Prebuffered");
	Create(mAudioFile, mReferenceDistance, mMaxDistance, mStreamed, mLooped, mPrebuffered);
}

void SGTSound3D::GetParameters(SGTDataMap *parameters)
{
	parameters->AddOgreString("AudioFile", mAudioFile);
	parameters->AddFloat("ReferenceDistance", mReferenceDistance);
	parameters->AddFloat("MaxSoundRange", mMaxDistance);
	parameters->AddBool("Streamed", mStreamed);
	parameters->AddBool("Looped", mLooped);
	parameters->AddBool("Prebuffered", mPrebuffered);
}

void SGTSound3D::GetDefaultParameters(SGTDataMap *parameters)
{
	parameters->AddOgreString("AudioFile", "");
	parameters->AddFloat("ReferenceDistance", 10.0f);
	parameters->AddFloat("MaxSoundRange", 50.0f);
	parameters->AddBool("Streamed", true);
	parameters->AddBool("Looped", true);
	parameters->AddBool("Prebuffered", true);
}

void SGTSound3D::Save(SGTSaveSystem& mgr)
{
	mgr.SaveAtom("Ogre::String", (void*)&mAudioFile, "AudioFile");
	mgr.SaveAtom("float", (void*)&mReferenceDistance, "ReferenceDistance");
	mgr.SaveAtom("float", (void*)&mMaxDistance, "MaxSoundRange");
	mgr.SaveAtom("bool", (void*)&mStreamed, "Streamed");
	mgr.SaveAtom("bool", (void*)&mLooped, "Looped");
	mgr.SaveAtom("bool", (void*)&mPrebuffered, "Prebuffered");
}

void SGTSound3D::Load(SGTLoadSystem& mgr)
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
SGTLocalLightRenderable::SGTLocalLightRenderable(Ogre::ColourValue diffuse, Ogre::ColourValue specular, bool spot, bool shadow_caster, float maxDistance, Ogre::Vector3 attenuation, Ogre::Vector3 spotlightdata, float flickerSpeed, float flickerIntensity)
{
	Create(diffuse, specular, shadow_caster, spot, maxDistance, attenuation, spotlightdata, flickerSpeed, flickerIntensity);
}

SGTLocalLightRenderable::~SGTLocalLightRenderable()
{
	SGTMain::Instance().GetOgreSceneMgr()->destroyLight(mLight);
	SGTMain::Instance().GetOgreSceneMgr()->destroyEntity(mEditorVisual);
}

void SGTLocalLightRenderable::Create(Ogre::ColourValue diffuse, Ogre::ColourValue specular, bool spot, bool shadow_caster, float maxDistance, Ogre::Vector3 attenuation, Ogre::Vector3 spotlightdata, float flickerSpeed, float flickerIntensity)
{
	mOriginalColour = diffuse;
	mFlickerSpeed = flickerSpeed;
	mFlickerIntensity = flickerIntensity;
	mFlickerOffset = Ogre::Math::RangeRandom(1, 100);
	int id = SGTSceneManager::Instance().RequestID();
	mLight = SGTMain::Instance().GetOgreSceneMgr()->createLight(Ogre::StringConverter::toString(id));
	Ogre::String editormeshname = "";
	if (spot)
	{
		editormeshname = "Editor_DirectionalLight.mesh";
		mLight->setType(Ogre::Light::LightTypes::LT_SPOTLIGHT);
		mLight->setSpotlightRange(Ogre::Radian(Ogre::Degree(spotlightdata.x)), Ogre::Radian(Ogre::Degree(spotlightdata.y)), spotlightdata.z);
	}
	else
	{
		editormeshname = "Editor_Omnilight.mesh";
		mLight->setType(Ogre::Light::LightTypes::LT_POINT);
	}
	mLight->setCastShadows(shadow_caster);
	mLight->setAttenuation(maxDistance, attenuation.x, attenuation.y, attenuation.z);

	mEditorVisual = SGTMain::Instance().GetOgreSceneMgr()->createEntity(Ogre::StringConverter::toString(id) + "_EditorVisual", editormeshname);
	mEditorVisual->setCastShadows(false);
}

void SGTLocalLightRenderable::AttachToNode(Ogre::SceneNode *node)
{
	node->attachObject(mLight);
	node->attachObject(mEditorVisual);
}

void SGTLocalLightRenderable::ShowEditorVisual(bool show)
{
	mEditorVisual->setVisible(show);
}

void SGTLocalLightRenderable::CreateFromDataMap(SGTDataMap *parameters)
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

void SGTLocalLightRenderable::GetParameters(SGTDataMap *parameters)
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

void SGTLocalLightRenderable::GetDefaultParameters(SGTDataMap *parameters)
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

void SGTLocalLightRenderable::Save(SGTSaveSystem& mgr)
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

void SGTLocalLightRenderable::Load(SGTLoadSystem& mgr)
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