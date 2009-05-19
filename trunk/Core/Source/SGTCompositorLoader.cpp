
#include "SGTCompositorLoader.h"
#include "SGTMain.h"
#include "SGTSceneManager.h"	//Caelum hack
#include "SGTWeatherController.h"


SGTCompositorLoader::SGTCompositorLoader()
{
	Ogre::CompositorManager::ResourceMapIterator resourceIterator =
    Ogre::CompositorManager::getSingleton().getResourceIterator();

    // add all compositor resources to the view container
    while (resourceIterator.hasMoreElements())
    {
		Ogre::ResourcePtr resource = resourceIterator.getNext();
        const Ogre::String& compositorName = resource->getName();
        // Don't add base Ogre/Scene compositor to view
        if (compositorName == "Ogre/Scene") continue;
		Ogre::LogManager::getSingleton().logMessage("Adding Compositor: " + compositorName);
		Ogre::CompositorInstance *instance = Ogre::CompositorManager::getSingleton().addCompositor(SGTMain::Instance().GetViewport(), compositorName, -1);
        Ogre::CompositorManager::getSingleton().setCompositorEnabled(SGTMain::Instance().GetViewport(), compositorName, false);
		mCompositorInstances.insert(std::make_pair<Ogre::String, Ogre::CompositorInstance*>(compositorName, instance));
	}
};

SGTCompositorLoader& SGTCompositorLoader::Instance()
{
	static SGTCompositorLoader TheOneAndOnly;
	return TheOneAndOnly;
};

Ogre::CompositorInstance* SGTCompositorLoader::AddListener(Ogre::String compositorName, Ogre::CompositorInstance::Listener *listener)
{
	mCompositorInstances[compositorName]->addListener(listener);
	return mCompositorInstances[compositorName];
};

void SGTCompositorLoader::EnableCompositor(Ogre::String compositorName)
{
	Ogre::CompositorManager::getSingleton().setCompositorEnabled(SGTMain::Instance().GetViewport(), compositorName, true);
};

void SGTCompositorLoader::DisableCompositor(Ogre::String compositorName)
{
	Ogre::CompositorManager::getSingleton().setCompositorEnabled(SGTMain::Instance().GetViewport(), compositorName, false);
};