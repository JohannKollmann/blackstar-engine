
#include "OgrePhysXWorld.h"
#include "OgrePhysXLogOutputStream.h"
#include "OgrePhysXScene.h"

namespace OgrePhysX
{

	World::World()
	{
		mSDK = 0;
		mSimulating = false;
	}

	World::~World()
	{
	}

	NxPhysicsSDK* World::getSDK()
	{
		return mSDK;
	}
	NxCookingInterface* World::getCookingInterface()
	{
		return mCookingInterface;
	}
	NxControllerManager* World::getControllerManager()
	{
		return mControllerManager;
	}

	World::OgreFrameListener* World::createFramelistener()
	{
		return new World::OgreFrameListener(this);
	}

	void World::shutdown()
	{
		if (mSDK)
		{
			World::getSingleton().getCookingInterface()->NxCloseCooking();
			clearScenes();
			mSDK->release();
			mSDK = 0;
		}
	}

	void World::init()
	{
		if (!mSDK)
		{
			mSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, 0, new LogOutputStream());

			if (!mSDK)
			{
				Ogre::LogManager::getSingleton().logMessage("[OgrePhysX] Error: SDK initialisation failed.");
				return;
			}

			mSDK->setParameter(NX_SKIN_WIDTH, 0.03f);

			mCookingInterface = NxGetCookingLib(NX_PHYSICS_SDK_VERSION);
			if (!mCookingInterface)
			{
				Ogre::LogManager::getSingleton().logMessage("[OgrePhysX] Error: Cooking initialisation failed.");
				return;
			}
			mCookingInterface->NxInitCooking();

			mControllerManager = NxCreateControllerManager(&mSDK->getFoundationSDK().getAllocator());
			if (!mControllerManager)
			{
				Ogre::LogManager::getSingleton().logMessage("[OgrePhysX] Error: Controller initialisation failed.");
				return;
			}

			Ogre::LogManager::getSingleton().logMessage("[OgrePhysX] SDK created.");
		}
	}

	Scene* World::addScene(Ogre::String name)
	{
		Scene *scene = new Scene();
		mScenes.insert(std::make_pair<Ogre::String, Scene*>(name, scene));
		return scene;
	}
	Scene* World::addScene(Ogre::String name, NxSceneDesc &desc)
	{
		Scene *scene = new Scene(desc);
		mScenes.insert(std::make_pair<Ogre::String, Scene*>(name, scene));
		return scene;
	}
	Scene* World::getScene(Ogre::String name)
	{
		std::map<Ogre::String, Scene*>::iterator i = mScenes.find(name);
		if (i != mScenes.end()) return i->second;
		else
		{
			Ogre::LogManager::getSingleton().logMessage("[OgrePhysX] Error: Could not find Scene \"" + name + "\".");
			return 0;
		}
	}
	void World::destroyScene(Ogre::String name)
	{
		std::map<Ogre::String, Scene*>::iterator i = mScenes.find(name);
		if (i != mScenes.end())
		{
			delete i->second;
			mScenes.erase(i);
		}
		else
		{
			Ogre::LogManager::getSingleton().logMessage("[OgrePhysX] Error: Could not find Scene \"" + name + "\".");
		}
	}
	void World::clearScenes()
	{
		for (std::map<Ogre::String, Scene*>::iterator i = mScenes.begin(); i != mScenes.end(); i++)
		{
			delete i->second;
		}
		mScenes.clear();
	}

	void World::startSimulate(float time)
	{
		if (mSimulating)
		{
			Ogre::LogManager::getSingleton().logMessage("[OgrePhysX] Warning: startSimulate called while simulation is in progress!");
			fetchSimulate();
		}

		mSimulating = true;
		for (std::map<Ogre::String, Scene*>::iterator i = mScenes.begin(); i != mScenes.end(); i++)
		{
			i->second->getNxScene()->simulate(time);
		}
	}
	void World::fetchSimulate()
	{
		if (mSimulating)
		{
			for (std::map<Ogre::String, Scene*>::iterator i = mScenes.begin(); i != mScenes.end(); i++)
			{
				i->second->getNxScene()->flushStream();
				i->second->getNxScene()->fetchResults(NX_RIGID_BODY_FINISHED, true);
			}
			mControllerManager->updateControllers();
		}
		mSimulating = false;
	}
	void World::syncRenderables()
	{
		for (std::map<Ogre::String, Scene*>::iterator i = mScenes.begin(); i != mScenes.end(); i++)
		{
			i->second->syncRenderables();
		}
	}

	World& World::getSingleton()
	{
		static World instance;
		return instance;
	}

}