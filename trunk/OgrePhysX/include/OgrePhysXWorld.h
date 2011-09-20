#pragma once

#include "OgrePhysXClasses.h"
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
				mWorld->simulate(evt.timeSinceLastFrame);
				mWorld->syncRenderables();
				return true;
			}
		};

	private:
		World();

		PxDefaultAllocator *mAllocator;

		LogOutputStream *mErrorOutputStream;

		PxPhysics *mPxPhysics;
		PxCooking *mCookingInterface;

		std::unordered_map<Ogre::String, Scene*> mScenes;

		bool mSimulating;

		PxMaterial *mDefaultMaterial;

		std::map<Ogre::String, PxMaterial*> mMaterialBindings;

	public:
		~World();

		void init();
		void shutdown();

		OgreFrameListener* createFramelistener();

		PxPhysics* getPxPhysics();
		PxCooking* getCookingInterface();

		Scene* addScene(Ogre::String name);
		Scene* addScene(Ogre::String name, PxSceneDesc &desc);
		Scene* getScene(Ogre::String name);
		void destroyScene(Ogre::String name);
		void clearScenes();

		///Advances the simulation.
		void simulate(float time);

		//Synchronises renderables.
		void syncRenderables(); 

		/**
		OgrePhysX creates a default material at startup. 
		@returns The default material.
		*/
		PxMaterial& getDefaultMaterial();

		/**
		Binds a material name to a PhysX material.
		By default, all material names are bound to the physX default material.
		*/
		void registerOgreMaterialName(const Ogre::String &matName, PxMaterial *material);

		/**
		Returns the material index assigned to the material name if existing, or nullptr.
		*/
		PxMaterial* getOgreMaterialByName(const Ogre::String &materialName);

		///Retrieves all material bindings.
		std::map<Ogre::String, PxMaterial*>& getOgreMaterialNames()
		{
			return mMaterialBindings;
		}

		//Sets the material bindings.
		void setOgreMaterialNames(const std::map<Ogre::String, PxMaterial*> &bindings)
		{
			mMaterialBindings = bindings;
		}

		//Singleton
		static World& getSingleton();
	};
}