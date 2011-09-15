
#include "OgrePhysXWorld.h"
#include "OgrePhysXLogOutputStream.h"
#include "OgrePhysXScene.h"
#include "PxPhysicsAPI.h"
#include "cooking/PxCooking.h"
#include "PxScene.h"
#include "extensions/PxDefaultAllocator.h"
#include "extensions/PxExtensionsAPI.h"

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

	PxPhysics* World::getSDK()
	{
		return mSDK;
	}
	PxCooking* World::getCookingInterface()
	{
		return mCookingInterface;
	}

	World::OgreFrameListener* World::createFramelistener()
	{
		return new World::OgreFrameListener(this);
	}

	void World::shutdown()
	{
		if (mSDK)
		{
			getCookingInterface()->release();
			clearScenes();
			mSDK->release();
			mSDK = nullptr;
			delete mAllocator;
			mAllocator = nullptr;
			delete mErrorOutputStream;
			mErrorOutputStream = nullptr;
		}
	}

	void World::init()
	{
		if (!mSDK)
		{
			mAllocator = new PxDefaultAllocator();
			mErrorOutputStream = new LogOutputStream();

			mSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *mAllocator, *mErrorOutputStream, PxTolerancesScale());

			if (!mSDK)
			{
				Ogre::LogManager::getSingleton().logMessage("[OgrePhysX] Error: SDK initialisation failed.");
				return;
			}

			PxCookingParams params;
			mCookingInterface = PxCreateCooking(PX_PHYSICS_VERSION, &mSDK->getFoundation(), params);
			if (!mCookingInterface)
			{
				Ogre::LogManager::getSingleton().logMessage("[OgrePhysX] Error: Cooking initialisation failed.");
				return;
			}

			if (!PxInitExtensions(*mSDK))
			{
				Ogre::LogManager::getSingleton().logMessage("[OgrePhysX] Error: Extension initialisation failed.");
				return;
			}

			if (mSDK->getPvdConnectionManager())
				PxExtensionVisualDebugger::connect(mSDK->getPvdConnectionManager(), "localhost", 5425, 500, true);

			//create default material
			mDefaultMaterial = mSDK->createMaterial(0.5f, 0.5f, 0.5f);

			Ogre::LogManager::getSingleton().logMessage("[OgrePhysX] SDK created.");
		}
	}

	Scene* World::addScene(Ogre::String name)
	{
		Scene *scene = new Scene();
		mScenes.insert(std::make_pair<Ogre::String, Scene*>(name, scene));
		return scene;
	}
	Scene* World::addScene(Ogre::String name, PxSceneDesc *desc)
	{
		Scene *scene = new Scene(*desc);
		mScenes.insert(std::make_pair<Ogre::String, Scene*>(name, scene));
		return scene;
	}
	Scene* World::getScene(Ogre::String name)
	{
		auto i = mScenes.find(name);
		if (i != mScenes.end()) return i->second;
		else
		{
			Ogre::LogManager::getSingleton().logMessage("[OgrePhysX] Error: Could not find Scene \"" + name + "\".");
			return 0;
		}
	}
	void World::destroyScene(Ogre::String name)
	{
		auto i = mScenes.find(name);
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
		for (auto i = mScenes.begin(); i != mScenes.end(); i++)
		{
			delete i->second;
		}
		mScenes.clear();
	}

	void World::simulate(float time)
	{
		if (mSimulating)
		{
			Ogre::LogManager::getSingleton().logMessage("[OgrePhysX] Warning: startSimulate called while simulation is in progress!");
		}

		mSimulating = true;
		for (auto i = mScenes.begin(); i != mScenes.end(); i++)
		{
			i->second->simulate(time);
		}
		mSimulating = false;
	}

	void World::renderDebugVisuals()
	{
		for (auto i = mScenes.begin(); i != mScenes.end(); i++)
		{
			i->second->renderDebugGeometry();
		}
	}

	void World::syncRenderables()
	{
		for (auto i = mScenes.begin(); i != mScenes.end(); i++)
		{
			i->second->syncRenderables();
		}
	}

	World& World::getSingleton()
	{
		static World instance;
		return instance;
	}

	PxMaterial& World::getDefaultMaterial()
	{
		return *mDefaultMaterial;
	}

	void World::registerOgreMaterialName(const Ogre::String &matName, PxMaterial *pxMat)
	{
		auto i = mMaterialBindings.find(matName);
		if (i != mMaterialBindings.end()) i->second = pxMat;
		else 
			mMaterialBindings.insert(std::make_pair<Ogre::String, PxMaterial*>(matName, pxMat));
	}

	PxMaterial* World::getOgreMaterialByName(const Ogre::String &matName)
	{
		auto i  = mMaterialBindings.find(matName);
		if (i != mMaterialBindings.end()) return i->second;
		return mDefaultMaterial;
	}



}