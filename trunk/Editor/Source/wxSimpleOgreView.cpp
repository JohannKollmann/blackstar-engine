
#include "wxSimpleOgreView.h"
#include "IceMain.h"


wxSimpleOgreView::wxSimpleOgreView(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
	: wxOgre(parent, -1, pos, size, style)
{
	mNode = nullptr;
}

wxSimpleOgreView::~wxSimpleOgreView(void)
{
	Reset();
	if (mNode) mSceneMgr->destroySceneNode(mNode);
}

void wxSimpleOgreView::OnInit()
{
	mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC, "ObjectPreview");
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.8f, 0.8f, 0.8f));
	Ogre::Light *ambientDirLight = mSceneMgr->createLight("SkyLight");
	ambientDirLight->setType(Ogre::Light::LightTypes::LT_DIRECTIONAL);
	ambientDirLight->setDirection(Ogre::Vector3(0,-1,0.4f).normalisedCopy());
	ambientDirLight->setDiffuseColour(Ogre::ColourValue(0.6,0.6,0.6));
	ambientDirLight->setSpecularColour(Ogre::ColourValue(1,1,1));

	mCamera = mSceneMgr->createCamera("PreviewCamera");
	mCamera->lookAt(Ogre::Vector3(0,0,1));
	mCamera->setNearClipDistance(0.1f);
	mCamera->setFarClipDistance(100);

	mViewport = mRenderWindow->addViewport(mCamera);
	mViewport->setBackgroundColour(Ogre::ColourValue::Black);
	mCamera->setAspectRatio(Ogre::Real(mViewport->getActualWidth()) / Ogre::Real(mViewport->getActualHeight()));

	mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::Vector3(0, 0, 5));
}

void wxSimpleOgreView::OnRender()
{
}

void wxSimpleOgreView::ShowMesh(Ogre::String meshFileName)
{
	Reset();
	if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", meshFileName))
	{
		Ogre::LogManager::getSingleton().logMessage("Error: Resource \"" + meshFileName + "\" does not exist. Loading dummy Resource...");
		meshFileName = "DummyMesh.mesh";
	}
	Ogre::Entity *ent = mSceneMgr->createEntity(meshFileName);
	mNode->attachObject(ent);

	Ogre::Vector3 center = ent->getBoundingBox().getCenter();
	Ogre::Vector3 dimensions = ent->getBoundingBox().getSize();
	float max = dimensions.x > dimensions.y ? dimensions.x : dimensions.y;
	max = max > dimensions.z ? max : dimensions.z;
	float scale_factor = 1.0f / max;
	float z_offset = (1.0f - (dimensions.z * scale_factor)) * 0.5f;
	mNode->setScale(scale_factor, scale_factor, scale_factor);
	mNode->setPosition( Ogre::Vector3(0, 0, 1.9f-z_offset) + (scale_factor * center * -1.0f));
}

void wxSimpleOgreView::ShowParticleEffect(Ogre::String pfxName)
{
	Reset();
}

void wxSimpleOgreView::ShowTexture(Ogre::String texFileName)
{
	Reset();
}

void wxSimpleOgreView::Reset()
{
	if (!mNode) return;
	for (unsigned int i = 0; i < mNode->numAttachedObjects(); i++)
	{
		mNode->getCreator()->destroyMovableObject(mNode->getAttachedObject(i));
	}
	mNode->setScale(1,1,1);
	mNode->setPosition(0,0,5);
}

void wxSimpleOgreView::Update(float time)
{
	if (mNode) mNode->yaw(Ogre::Radian(time * 0.5f));
}