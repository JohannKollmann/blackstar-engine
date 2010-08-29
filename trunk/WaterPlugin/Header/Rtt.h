

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


/*class __declspec(dllexport) RefractionListener : public BaseRttListener
{
public:

	void preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
	{
		//mCamera->enableCustomNearClipPlane(mPlane);

		ignoreObjects();
	}

	void postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
	{
		//mCamera->disableCustomNearClipPlane();

		resetObjects();
	}
};

class RefractionManager
{
private:
	int mUseCounter;

	RefractionListener mRefractionListener;
	Ogre::TexturePtr mRefractionTexture;		//Rtt for the refraction color map
	Ogre::TexturePtr mRefractionDepthTexture;	//Refration depth map
	Ogre::TexturePtr mRefractionOccludersDepthTexture;	//Depth map of refraction occluders
	Ogre::MultiRenderTarget *mRefractMRT;

	void _clear()
	{
		if (!mRefractionTexture.isNull())
		{
			mRefractionListener.clearIgnoreObjects();
			Ogre::TextureManager::getSingleton().remove(mRefractionTexture->getName());
		}
		if (!mRefractionDepthTexture.isNull())
		{
			Ogre::TextureManager::getSingleton().remove(mRefractionDepthTexture->getName());
		}
		if (!mRefractionOccludersDepthTexture.isNull())
		{
			mRefractionOccludersDepthTexture->getBuffer()->getRenderTarget()->removeAllViewports();
			Ogre::TextureManager::getSingleton().remove(mRefractionOccludersDepthTexture->getName());
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
	void _create()
	{
		int width = Ice::Main::Instance().GetViewport()->getActualWidth();
		int height = Ice::Main::Instance().GetViewport()->getActualHeight();
		
		mRefractionOccludersDepthTexture = Ogre::TextureManager::getSingleton().createManual("RefractionOccludersDepth",
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, width, height, 0, Ogre::PF_FLOAT16_R, Ogre::TU_RENDERTARGET);
		mRefractionOccludersDepthTexture->getBuffer()->getRenderTarget()->addViewport(Ice::Main::Instance().GetCamera())->setMaterialScheme("depth");
		mRefractionOccludersDepthTexture->getBuffer()->getRenderTarget()->getViewport(0)->setBackgroundColour(Ogre::ColourValue::Black);
		mRefractionOccludersDepthTexture->getBuffer()->getRenderTarget()->getViewport(0)->setClearEveryFrame(true);
		mRefractionOccludersDepthTexture->getBuffer()->getRenderTarget()->getViewport(0)->setShadowsEnabled(false);
		mRefractionOccludersDepthTexture->getBuffer()->getRenderTarget()->getViewport(0)->setRenderQueueInvocationSequenceName("RefractionSurfacesOnly");

		auto matIter = Ogre::MaterialManager::getSingleton().getResourceIterator();
		while (matIter.hasMoreElements())
		{
			Ogre::MaterialPtr mat = matIter.getNext();
			if (mat->getTechnique("Refraction"))
			{
				mat->getTechnique("Refraction")->getPass(0)->getTextureUnitState("RefractionOccludersDepth")->setTextureName("RefractionOccludersDepth");
			}
		}
		//setup refraction MRT
		mRefractionTexture = Ogre::TextureManager::getSingleton().createManual("Refraction",
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, width, height, 0, Ogre::PF_FLOAT16_RGBA, Ogre::TU_RENDERTARGET);

		mRefractionDepthTexture = Ogre::TextureManager::getSingleton().createManual("RefractionDepth",
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, width, height, 0, Ogre::PF_FLOAT16_R, Ogre::TU_RENDERTARGET);

		mRefractMRT = Ogre::Root::getSingleton().getRenderSystem()->createMultiRenderTarget("RefractionMRT");
 
		mRefractionTexture->getBuffer()->getRenderTarget()->setAutoUpdated(false);
		mRefractionDepthTexture->getBuffer()->getRenderTarget()->setAutoUpdated(false);
 
		mRefractMRT->bindSurface(0, mRefractionTexture->getBuffer()->getRenderTarget());
		mRefractMRT->bindSurface(1, mRefractionDepthTexture->getBuffer()->getRenderTarget());
		mRefractMRT->setAutoUpdated(true);
 
		mRefractMRT->addViewport(Ice::Main::Instance().GetCamera())->setMaterialScheme("Refraction");
		mRefractMRT->getViewport(0)->setOverlaysEnabled(false);
		mRefractMRT->getViewport(0)->setShadowsEnabled(false);
		mRefractMRT->addListener(&mRefractionListener);
	}

public:
	RefractionManager() : mUseCounter(0), mRefractMRT(nullptr)
	{
	}
	~RefractionManager()
	{
		_clear();
	}
	void AddRefractionObject(Ogre::Entity *object)
	{
		if (mUseCounter == 0)
			_create();
		mRefractionListener.addIgnoreObject(object);
		mUseCounter++;
	}
	void RemoveRefractionObject(Ogre::Entity *object)
	{
		IceAssert(mUseCounter > 0);
		mUseCounter--;
		mRefractionListener.removeIgnoreObject(object);
		if (mUseCounter == 0)
			_clear();
	}

	static RefractionManager& Instance()
	{
		static RefractionManager instance;
		return instance;
	}
};*/

class __declspec(dllexport) WaterPlane
{
private:
	ReflectionListener mReflectionListener;
	Ogre::TexturePtr mReflectionTexture;

	Ogre::MaterialPtr mMaterial;

	Ogre::Entity *mEntity;

public:
	WaterPlane() : mEntity(nullptr)
	{
	}
	~WaterPlane()
	{
		unloadResources();
	}

	void unloadResources()
	{
		if (mEntity)
		{
			//RefractionManager::Instance().RemoveRefractionObject(mEntity);
			mEntity = nullptr;
		}
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
	}

	void setPlane(Ogre::Plane &plane)
	{
		plane.normalise();
		mReflectionListener.setPlane(plane);
	}

	void create(Ogre::Entity *ent, Ogre::Camera *cam)
	{
		Ogre::MaterialPtr baseMat = Ogre::MaterialManager::getSingleton().getByName("WaterReflectionRefraction");
		IceAssert(!baseMat.isNull());

		mEntity = ent;

		Ogre::String id = Ice::SceneManager::Instance().RequestIDStr();
		mMaterial = baseMat->clone("WaterMaterial_" + id);
		mEntity->setMaterial(mMaterial);

		mEntity->setVisibilityFlags(1);

		//setup rtts
		mReflectionListener.setCamera(cam);
		mReflectionListener.addIgnoreObject(ent);

		mReflectionTexture = Ogre::TextureManager::getSingleton().createManual("Reflection_" + id,
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, 1024, 1024, 0, Ogre::PF_FLOAT16_RGB, Ogre::TU_RENDERTARGET);
		Ogre::RenderTarget* reflection_rtt = mReflectionTexture->getBuffer()->getRenderTarget();
		reflection_rtt->addViewport(cam)->setOverlaysEnabled(false);
		reflection_rtt->getViewport(0)->setShadowsEnabled(false);
		reflection_rtt->getViewport(0)->setMaterialScheme("LowQuality");
		reflection_rtt->addListener(&mReflectionListener);

		//RefractionManager::Instance().AddRefractionObject(mEntity);

		mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState("Reflection")->setTextureName(mReflectionTexture->getName());
		mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState("Refraction")->setTextureName(Ice::Main::Instance().GetSceneRenderCompositor()->getTextureInstanceName("rt_SceneNoRefractiveObjects", 0));
		mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState("RefractionMask")->setTextureName(Ice::Main::Instance().GetSceneRenderCompositor()->getTextureInstanceName("rt_DepthBuffer", 0));
		//mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState("RefractionDepth")->setTextureName(Ice::Main::Instance().GetSceneRenderCompositor()->getTextureInstanceName("rt_DepthNoRefractiveObjects", 0));
	}
};