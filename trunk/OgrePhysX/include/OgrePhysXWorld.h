/*
This source file is part of OgrePhysX.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

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

		OgreFrameListener* setupOgreFramelistener();

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