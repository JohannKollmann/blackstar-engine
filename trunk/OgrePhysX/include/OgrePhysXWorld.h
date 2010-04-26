#pragma once

#include "OgrePhysXClasses.h"
#include "NxPhysics.h"
#include "NxCooking.h"
#include "NxControllerManager.h"
#include <map>
#include "Ogre.h"

namespace OgrePhysX
{
	class OgrePhysXClass World
	{
		class OgrePhysXClass OgreFrameListener : public Ogre::FrameListener
		{
		private:
			World *mWorld;
		public:
			OgreFrameListener(World *world) : mWorld(world) {}
			~OgreFrameListener() {}
			bool frameStarted (const Ogre::FrameEvent &evt)
			{
				mWorld->startSimulate(evt.timeSinceLastFrame);
				mWorld->fetchSimulate();
				mWorld->syncRenderables();
				return true;
			}
		};

	private:
		World();

		NxPhysicsSDK *mSDK;
		NxControllerManager	*mControllerManager;
		NxCookingInterface *mCookingInterface;

		std::map<Ogre::String, Scene*> mScenes;

		bool mSimulating;

	public:
		~World();

		void init();
		void shutdown();

		OgreFrameListener* createFramelistener();

		NxPhysicsSDK* getSDK();
		NxCookingInterface* getCookingInterface();
		NxControllerManager* getControllerManager();

		Scene* addScene(Ogre::String name);
		Scene* addScene(Ogre::String name, NxSceneDesc &desc);
		Scene* getScene(Ogre::String name);
		void destroyScene(Ogre::String name);
		void clearScenes();

		void startSimulate(float time);
		void fetchSimulate();
		void syncRenderables(); 

		//Singleton
		static World& getSingleton();
	};
}