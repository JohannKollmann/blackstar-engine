
#include "ResourceGroupReloader.h"

#include "OgreResourceGroupManager.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreMovableObject.h"
#include "OgreMaterialManager.h"
#include "OgreEntity.h"
#include "OgreSubEntity.h"
#include "OgreBillBoardChain.h"
#include "OgreBillBoardSet.h"
#include "OgreOverlayElement.h"
#include "OgreOverlay.h"
#include "OgreOverlayManager.h"
#include "OgreOverlayContainer.h"
#include "Ogre.h"
#include "IceMain.h"
#include "Edit.h"

ResourceGroupReloader::UpdateMaterialRenderableVisitor::UpdateMaterialRenderableVisitor():
Ogre::Renderable::Visitor()
{
}

void ResourceGroupReloader::UpdateMaterialRenderableVisitor::visit(
	Ogre::Renderable *rend, Ogre::ushort lodIndex, bool isDebug, Ogre::Any *pAny)
{
	const Ogre::MaterialPtr mat = rend->getMaterial();
	if(!mat.isNull())
	{
		std::string newMatName = mat->getName();
		Ogre::MaterialPtr newMat = Ogre::MaterialManager::getSingleton().getByName(newMatName);
		if(newMat.isNull())
		{
			// this can happen if there was error during the reloading of the material.
			// in that case, we keep the ancient one.
			// Ice::Log::Instance().LogMessage(newMatName+" : new material is null!");
			return;
		}

		// unfortunately, the renderable gives access only to a const MaterialPtr.
		// and there is no 'setMaterial' or 'setMaterialName' method on renderables.
		// so I have to try to down cast with known classes...
		{   
			Ogre::SubEntity* lRend = dynamic_cast<Ogre::SubEntity*>(rend);
			if(lRend){lRend->setMaterialName(newMatName);return;} 
		}
		{
			Ogre::SimpleRenderable* lRend = dynamic_cast<Ogre::SimpleRenderable*>(rend);
			if(lRend){lRend->setMaterial(newMatName);return;} 
		}
		{
			Ogre::ShadowRenderable* lRend = dynamic_cast<Ogre::ShadowRenderable*>(rend);
			if(lRend){lRend->setMaterial(newMat);return;} 
		}
		{   
			Ogre::BillboardChain* lRend = dynamic_cast<Ogre::BillboardChain*>(rend);
			if(lRend){lRend->setMaterialName(newMatName);return;} 
		}
		{   
			Ogre::BillboardSet* lRend = dynamic_cast<Ogre::BillboardSet*>(rend);
			if(lRend){lRend->setMaterialName(newMatName);return;} 
		}
		{   
			Ogre::OverlayElement* lRend = dynamic_cast<Ogre::OverlayElement*>(rend);
			if(lRend){lRend->setMaterialName(newMatName);return;} 
		}
	}else{
		// was there for debug...
		// Ice::Log::Instance().LogMessage("material of renderable is null!");
	}
}

ResourceGroupReloader::ResourceGroupReloader(void)
{
}

ResourceGroupReloader::~ResourceGroupReloader(void)
{
}

bool ResourceGroupReloader::reloadAResourceGroupWithoutDestroyingIt(const std::string& pResourceGroupName)
{
	if(!resourceGroupExist(pResourceGroupName))
	{
		// not present. something wrong.
		return false;
	}
	Ogre::ResourceGroupManager& resGroupMgr = Ogre::ResourceGroupManager::getSingleton();
	resGroupMgr.clearResourceGroup(pResourceGroupName);
	resGroupMgr.initialiseResourceGroup(pResourceGroupName);
	return true;
}


bool ResourceGroupReloader::resourceGroupExist(const std::string& pResourceGroupName)
{
	bool lIsPresent = false;
	Ogre::ResourceGroupManager& resGroupMgr = Ogre::ResourceGroupManager::getSingleton();
	Ogre::StringVector lAllResourceGroups = resGroupMgr.getResourceGroups();
	Ogre::StringVector::iterator iter = lAllResourceGroups.begin();
	Ogre::StringVector::iterator iterEnd = lAllResourceGroups.end();
	for(;iter!=iterEnd;iter++)
	{
		if((*iter) == pResourceGroupName)
		{
			lIsPresent = true;
		}
	}
	return lIsPresent;
}


void ResourceGroupReloader::updateOnEveryRenderable()
{

	//1/ get all the available object type (entity, light, user defined types ...)
	std::vector<std::string> allAvailableTypes; 
	Ogre::Root::MovableObjectFactoryIterator iterFactory = Ogre::Root::getSingleton().getMovableObjectFactoryIterator();
	for(;iterFactory.hasMoreElements();)
	{
		Ogre::MovableObjectFactory* factory = iterFactory.getNext();
		allAvailableTypes.push_back(factory->getType());
	}

	UpdateMaterialRenderableVisitor lRenderableVisitor;

	//2/ for each scene manager, lets visit renderables!
	// unfortunately that does not cover all renderables type... (overlays...)
	Ogre::SceneManagerEnumerator::SceneManagerIterator iterSceneManager = Ogre::Root::getSingleton().getSceneManagerIterator();
	for(;iterSceneManager.hasMoreElements();)
	{
		Ogre::SceneManager * scMgr = iterSceneManager.getNext();

		std::vector<std::string>::iterator iterMovableType = allAvailableTypes.begin();
		std::vector<std::string>::iterator iterMovableTypeEnd = allAvailableTypes.end();
		for(;iterMovableType!=iterMovableTypeEnd;iterMovableType++)
		{
			Ogre::SceneManager::MovableObjectIterator iterMovable = scMgr->getMovableObjectIterator(*iterMovableType);
			for(;iterMovable.hasMoreElements();)
			{
				Ogre::MovableObject * movable = iterMovable.getNext();
				movable->visitRenderables(&lRenderableVisitor,false);
			}
		}
	}

	// 3 / visit overlays!
	{
		Ogre::OverlayManager::OverlayMapIterator iterOverlay = Ogre::OverlayManager::getSingleton().getOverlayIterator();
		for(;iterOverlay.hasMoreElements();)
		{
			Ogre::Overlay* lOverlay = iterOverlay.getNext();
			// get the first level of OverlayContainer in the Overlay
			Ogre::Overlay::Overlay2DElementsIterator iterOverlayElem = lOverlay->get2DElementsIterator();
			for(;iterOverlayElem.hasMoreElements();)
			{
				Ogre::OverlayContainer * lOverlayCont = iterOverlayElem.getNext();
				visitRecursivelyRenderablesFrom(lOverlayCont,lRenderableVisitor, false);
			}
		}
	}
}


void ResourceGroupReloader::visitRecursivelyRenderablesFrom(Ogre::OverlayContainer* pOverlayContainer, Ogre::Renderable::Visitor& pVisitor, bool debugRenderable)
{
	// call on 'this'
	pOverlayContainer->visitRenderables(&pVisitor,false);

	// call on 'leaf' (cf composite pattern)
	{
		Ogre::OverlayContainer::ChildIterator childIter = pOverlayContainer->getChildIterator();
		for(;childIter.hasMoreElements();)
		{
			Ogre::OverlayElement* lOverElem = childIter.getNext();
			lOverElem->visitRenderables(&pVisitor,false);
		}
	}

	// call on 'not-leaf' (cf composite pattern)
	{
		Ogre::OverlayContainer::ChildContainerIterator childContainerIter = pOverlayContainer->getChildContainerIterator();
		for(;childContainerIter.hasMoreElements();)
		{
			Ogre::OverlayContainer * childContainer = childContainerIter.getNext();
			visitRecursivelyRenderablesFrom(childContainer, pVisitor,debugRenderable);
		}
	}
}

void ResourceGroupReloader::reloadResourceGroup(const std::string& resourceGroupName)
{
	STOP_MAINLOOP
	Ogre::GpuProgramManager::getSingleton().clearSharedParameterSets();
	Ogre::CompositorManager::getSingleton().setCompositorEnabled(Ice::Main::Instance().GetViewport(), "RenderHDRScene", false);
	Ogre::CompositorManager::getSingleton().setCompositorEnabled(Ice::Main::Instance().GetViewport(), "DownsampleHDR1", false);
	Ogre::ParticleSystemManager::getSingleton().removeAllTemplates();
	Ogre::CompositorManager::getSingleton().removeAll();
	reloadAResourceGroupWithoutDestroyingIt(resourceGroupName);
	updateOnEveryRenderable();
	Ice::Main::Instance().GetOgreSceneMgr()->setShadowTextureCasterMaterial("shadow_caster");
	Ogre::CompositorManager::getSingleton().initialise();
	Ice::Main::Instance().InitCompositor();
	RESUME_MAINLOOP
}