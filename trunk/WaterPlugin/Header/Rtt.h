

#pragma once

#include "Ogre.h"
#include "IceIncludes.h"
#include "IceSceneManager.h"


class __declspec(dllexport) BaseRttListener : public Ogre::RenderTargetListener
{
protected:
	Ogre::Camera *mCamera;
	Ogre::Plane mPlane;

	struct SIgnoreItem
	{
	public:
		Ogre::MovableObject *object;
		bool resetState;
	};
	std::vector<SIgnoreItem> mIgnoreList;
	void ignoreObjects()
	{
		for (auto i = mIgnoreList.begin(); i != mIgnoreList.end(); i++)
		{
			i->resetState = i->object->isVisible();
			i->object->setVisible(false);
		}
	}
	void resetObjects()
	{
		for (auto i = mIgnoreList.begin(); i != mIgnoreList.end(); i++)
			i->object->setVisible(i->resetState);
	}

public:
	BaseRttListener() : mCamera(nullptr) {}
	virtual ~BaseRttListener() {}

	void addIgnoreObject(Ogre::MovableObject *object)
	{
		SIgnoreItem item;
		item.object = object;
		item.resetState = object->isVisible();
		mIgnoreList.push_back(item);
	}
	void removeIgnoreObject(Ogre::MovableObject *object)
	{
		for (auto i = mIgnoreList.begin(); i != mIgnoreList.end(); i++)
		{
			if (i->object == object)
			{
				mIgnoreList.erase(i);
				return;
			}
		}
	}
	void clearIgnoreObjects()
	{
		mIgnoreList.clear();
	}
	void setPlane(Ogre::Plane &plane)
	{
		mPlane = plane;
	}
	void setCamera(Ogre::Camera *cam)
	{
		mCamera = cam;
	}
};

class __declspec(dllexport) ReflectionListener : public BaseRttListener
{
public:

	void preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
	{
		mCamera->enableCustomNearClipPlane(mPlane);
		mCamera->enableReflection(mPlane);
		ignoreObjects();
	}

	void postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
	{
		mCamera->disableCustomNearClipPlane();
		mCamera->disableReflection();
		resetObjects();
	}
};


class __declspec(dllexport) RefractionListener : public BaseRttListener
{
public:

	void preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
	{
		mCamera->enableCustomNearClipPlane(mPlane);

		ignoreObjects();
	}

	void postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
	{
		mCamera->disableCustomNearClipPlane();

		resetObjects();
	}
};

class __declspec(dllexport) WaterPlane
{
private:
	ReflectionListener mReflectionListener;
	Ogre::TexturePtr mReflectionTexture;

	RefractionListener mRefractionListener;
	Ogre::TexturePtr mRefractionTexture;
	Ogre::TexturePtr mRefractionDepthTexture;
	Ogre::MultiRenderTarget *mRefractMRT;

	Ogre::MaterialPtr mMaterial;

public:
	WaterPlane() : mRefractMRT(nullptr)
	{
	}
	~WaterPlane()
	{
		unloadResources();
	}

	void unloadResources()
	{
		if (!mMaterial.isNull())
		{
			Ogre::MaterialManager::getSingleton().remove(mMaterial->getName());
			mMaterial.setNull();
		}
		if (!mReflectionTexture.isNull())
		{
			mReflectionListener.clearIgnoreObjects();
			mReflectionTexture->getBuffer()->getRenderTarget()->removeAllViewports();
			mReflectionTexture->getBuffer()->getRenderTarget()->removeAllListeners();
			Ogre::TextureManager::getSingleton().remove(mReflectionTexture->getName());
			mReflectionTexture.setNull();
		}
		if (!mRefractionTexture.isNull())
		{
			mRefractionListener.clearIgnoreObjects();
			mRefractionTexture->getBuffer()->getRenderTarget()->removeAllViewports();
			mRefractionTexture->getBuffer()->getRenderTarget()->removeAllListeners();
			Ogre::TextureManager::getSingleton().remove(mRefractionTexture->getName());
		}
		if (!mRefractionDepthTexture.isNull())
		{
			mRefractionDepthTexture->getBuffer()->getRenderTarget()->removeAllViewports();
			mRefractionDepthTexture->getBuffer()->getRenderTarget()->removeAllListeners();
			Ogre::TextureManager::getSingleton().remove(mRefractionDepthTexture->getName());
		}
		if (mRefractMRT)
		{
			mRefractMRT->removeAllViewports();
			mRefractMRT->removeAllListeners();
			Ogre::Root::getSingleton().getRenderSystem()->destroyRenderTarget(mRefractMRT->getName());
			mRefractMRT = nullptr;
			mRefractionDepthTexture.setNull();
		}
	}

	void setPlane(Ogre::Plane &plane)
	{
		plane.normalise();
		mReflectionListener.setPlane(plane);
		Ogre::Plane refractPlane(-plane.normal, plane.d);
		mRefractionListener.setPlane(refractPlane);
	}

	void create(Ogre::Entity *ent, Ogre::Camera *cam)
	{
		Ogre::MaterialPtr baseMat = Ogre::MaterialManager::getSingleton().getByName("WaterReflectionRefraction");
		IceAssert(!baseMat.isNull());

		Ogre::String id = Ice::SceneManager::Instance().RequestIDStr();
		mMaterial = baseMat->clone("WaterMaterial_" + id);
		ent->setMaterial(mMaterial);

		ent->setRenderQueueGroup(Ogre::RENDER_QUEUE_6);	//water surface group

		//setup rtts
		mReflectionListener.setCamera(cam);
		mReflectionListener.addIgnoreObject(ent);
		mRefractionListener.setCamera(cam);
		mRefractionListener.addIgnoreObject(ent);

		mReflectionTexture = Ogre::TextureManager::getSingleton().createManual("Reflection_" + id,
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, 1024, 1024, 0, Ogre::PF_FLOAT16_RGB, Ogre::TU_RENDERTARGET);
		Ogre::RenderTarget* reflection_rtt = mReflectionTexture->getBuffer()->getRenderTarget();
		reflection_rtt->addViewport(cam)->setOverlaysEnabled(false);
		reflection_rtt->getViewport(0)->setShadowsEnabled(false);
		reflection_rtt->getViewport(0)->setMaterialScheme("LowQuality");
		reflection_rtt->addListener(&mReflectionListener);

		//setup refraction MRT
		mRefractionTexture = Ogre::TextureManager::getSingleton().createManual("Refraction" + id,
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, 512, 512, 0, Ogre::PF_FLOAT16_RGB, Ogre::TU_RENDERTARGET);

		mRefractionDepthTexture = Ogre::TextureManager::getSingleton().createManual("RefractionDepth" + id,
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, 512, 512, 0, Ogre::PF_FLOAT16_R, Ogre::TU_RENDERTARGET);

		mRefractMRT = Ogre::Root::getSingleton().getRenderSystem()->createMultiRenderTarget("RefractionMRT" + id);
 
		mRefractionTexture->getBuffer()->getRenderTarget()->setAutoUpdated(false);
		mRefractionDepthTexture->getBuffer()->getRenderTarget()->setAutoUpdated(false);
 
		mRefractMRT->bindSurface(0, mRefractionTexture->getBuffer()->getRenderTarget());
		mRefractMRT->bindSurface(1, mRefractionDepthTexture->getBuffer()->getRenderTarget());
		mRefractMRT->setAutoUpdated(true);
 
		mRefractMRT->addViewport(cam)->setMaterialScheme("Refraction");
		mRefractMRT->getViewport(0)->setOverlaysEnabled(false);
		mRefractMRT->getViewport(0)->setShadowsEnabled(false);
		mRefractMRT->addListener(&mRefractionListener);


		mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState("Reflection")->setTextureName(mReflectionTexture->getName());
		mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState("Refraction")->setTextureName(mRefractionTexture->getName());
		mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState("RefractionDepth")->setTextureName(mRefractionDepthTexture->getName());
	}
};