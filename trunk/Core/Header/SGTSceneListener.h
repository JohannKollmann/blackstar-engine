#pragma once

#include "Ogre.h"
#include "SGTIncludes.h"
#include "SGTMain.h"

class SGTDllExport RefractionTextureListener : public Ogre::RenderTargetListener
{

public:

	void preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
    {
		if (SGTMain::Instance().mWaterTestEntity != 0)
		{
			SGTMain::Instance().mWaterTestEntity->setVisible(false);
		}
    }

    void postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
    {
		if (SGTMain::Instance().mWaterTestEntity != 0)
		{
			SGTMain::Instance().mWaterTestEntity->setVisible(true);
		}
    }

};
class SGTDllExport ReflectionTextureListener : public Ogre::RenderTargetListener
{

public:
	
			class CReflectionQueueListener : public Ogre::RenderQueueListener
			{
			public:
				/** Called at the start of the queue
				 */
				void renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String &invocation, bool &skipThisInvocation) 
				{
					if ((queueGroupId == Ogre::RENDER_QUEUE_SKIES_EARLY || queueGroupId == Ogre::RENDER_QUEUE_SKIES_LATE) 
						&& mActive)
					{
						SGTMain::Instance().GetCamera()->disableCustomNearClipPlane();
						Ogre::Root::getSingleton().getRenderSystem()->_setProjectionMatrix(SGTMain::Instance().GetCamera()->getProjectionMatrixRS()); 
					}
				}

				/** Called on the end of the queue
				 */
				void renderQueueEnded(Ogre::uint8 queueGroupId, const Ogre::String &invocation, bool &skipThisInvocation) 
				{
					if ((queueGroupId == Ogre::RENDER_QUEUE_SKIES_EARLY || queueGroupId == Ogre::RENDER_QUEUE_SKIES_LATE) 
						&& mActive)
					{
						SGTMain::Instance().GetCamera()->enableCustomNearClipPlane(mReflectionPlane);
						Ogre::Root::getSingleton().getRenderSystem()->_setProjectionMatrix(SGTMain::Instance().GetCamera()->getProjectionMatrixRS()); 
					}
				}

				/// Is the reflection Rtt active?
				bool mActive;
				Ogre::MovablePlane *mReflectionPlane;
			};

private:
	Ogre::Camera *mCamera;
	Ogre::MovablePlane *mReflectionPlane;
	Ogre::SceneNode *mPlanesSceneNode;
	CReflectionQueueListener mCReflectionQueueListener;

public:

	ReflectionTextureListener(Ogre::Camera *Camera)
	{
		mCamera = Camera;
		mReflectionPlane = new Ogre::MovablePlane("ReflectionPlane");
		mReflectionPlane->normal = Ogre::Vector3::UNIT_Y;
        mReflectionPlane->d = 0;

		mPlanesSceneNode = SGTMain::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode();
		mPlanesSceneNode->attachObject(mReflectionPlane);

		mCReflectionQueueListener.mReflectionPlane = mReflectionPlane;

		SGTMain::Instance().GetOgreSceneMgr()->addRenderQueueListener(&mCReflectionQueueListener);
	}
    void preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
    {
		mCReflectionQueueListener.mActive = true;

		if (SGTMain::Instance().mWaterTestEntity != 0)
		{
			SGTMain::Instance().mWaterTestEntity->setVisible(false);
			mPlanesSceneNode->setPosition(SGTMain::Instance().mWaterTestEntity->getParentNode()->_getDerivedPosition());
		}
        mCamera->enableReflection(mReflectionPlane);
		mCamera->enableCustomNearClipPlane(mReflectionPlane);

    }
    void postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
	{

		if (SGTMain::Instance().mWaterTestEntity != 0)
		{
			SGTMain::Instance().mWaterTestEntity->setVisible(true);
		}
        mCamera->disableReflection();
		mCamera->disableCustomNearClipPlane();

		mCReflectionQueueListener.mActive = false;
    }

};


class SGTDllExport SGTSceneListener : public Ogre::SceneManager::Listener
{
public:
	SGTSceneListener(void);
	~SGTSceneListener(void);

	void shadowTextureCasterPreViewProj(Ogre::Light *light, Ogre::Camera *cam);
	void shadowTextureReceiverPreViewProj(Ogre::Light *light, Ogre::Frustum *frustum) {};
	void shadowTexturesUpdated(size_t numberOfShadowTextures) {};
	bool sortLightsAffectingFrustum(Ogre::LightList &lightList) { return true; };
	void postFindVisibleObjects(Ogre::SceneManager *source, Ogre::SceneManager::IlluminationRenderStage irs, Ogre::Viewport *v) {};
	void preFindVisibleObjects(Ogre::SceneManager *source, Ogre::SceneManager::IlluminationRenderStage irs, Ogre::Viewport *v) {};
};
