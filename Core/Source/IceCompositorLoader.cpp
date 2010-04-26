
#include "IceCompositorLoader.h"
#include "IceMain.h"
#include "IceSceneManager.h"	//Caelum hack
#include "IceWeatherController.h"

namespace Ice
{

CompositorLoader::CompositorLoader()
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
		Ogre::CompositorInstance *instance = Ogre::CompositorManager::getSingleton().addCompositor(Main::Instance().GetViewport(), compositorName, -1);
        Ogre::CompositorManager::getSingleton().setCompositorEnabled(Main::Instance().GetViewport(), compositorName, false);
		mCompositorInstances.insert(std::make_pair<Ogre::String, Ogre::CompositorInstance*>(compositorName, instance));
	}
};

CompositorLoader& CompositorLoader::Instance()
{
	static CompositorLoader TheOneAndOnly;
	return TheOneAndOnly;
};

Ogre::CompositorInstance* CompositorLoader::AddListener(Ogre::String compositorName, Ogre::CompositorInstance::Listener *listener)
{
	mCompositorInstances[compositorName]->addListener(listener);
	return mCompositorInstances[compositorName];
};

void CompositorLoader::EnableCompositor(Ogre::String compositorName)
{
	Ogre::CompositorManager::getSingleton().setCompositorEnabled(Main::Instance().GetViewport(), compositorName, true);
};

void CompositorLoader::DisableCompositor(Ogre::String compositorName)
{
	Ogre::CompositorManager::getSingleton().setCompositorEnabled(Main::Instance().GetViewport(), compositorName, false);
};

};