#pragma once

#include "OgrePhysXClasses.h"
#include <map>
#include "Ogre.h"

namespace OgrePhysX
{
	typedef unsigned short MaterialIndex;

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
				mWorld->renderDebugVisuals();
				return true;
			}
		};

	private:
		World();

		PxDefaultAllocator *mAllocator;

		LogOutputStream *mErrorOutputStream;

		PxPhysics *mSDK;
		PxCooking *mCookingInterface;

		std::unordered_map<Ogre::String, Scene*> mScenes;

		bool mSimulating;

		PxMaterial *mDefaultMaterial;

		std::map<Ogre::String, MaterialIndex> mMaterialBindings;

	public:
		~World();

		void init();
		void shutdown();

		OgreFrameListener* createFramelistener();

		PxPhysics* getSDK();
		PxCooking* getCookingInterface();

		Scene* addScene(Ogre::String name);
		Scene* addScene(Ogre::String name, PxSceneDesc *desc);
		Scene* getScene(Ogre::String name);
		void destroyScene(Ogre::String name);
		void clearScenes();

		///Advances the simulation.
		void simulate(float time);

		///Updates the debug visuals - needs access to both Ogre and PhysX.
		void renderDebugVisuals();

		//Synchronises renderables.
		void syncRenderables(); 

		/**
		OgrePhysX creates a default material at startup. 
		@returns The default material.
		*/
		PxMaterial& getDefaultMaterial();

		/**
		Binds an ogre material to a PhysX material.
		By default, all ogre materials are bound to the physX default material.
		Setup your material binds BEFORE you cook meshes.
		*/
		void bindOgreMaterial(const Ogre::String &ogreMatName, MaterialIndex index);

		/**
		Returns the material index assigned to the Ogre material if existing, or nullptr.
		*/
		MaterialIndex getMaterialIndexByOgreMat(const Ogre::String &ogreMaterialName);

		///Retrieves all material bindings.
		std::map<Ogre::String, MaterialIndex>& getOgreMaterialBindings()
		{
			return mMaterialBindings;
		}

		//Sets the material bindings.
		void setOgreMaterialBindings(const std::map<Ogre::String, MaterialIndex> &bindings)
		{
			mMaterialBindings = bindings;
		}

		//Singleton
		static World& getSingleton();
	};
}