#pragma once

#include "OgrePhysXClasses.h"
#include "Ogre.h"
#include "NxCooking.h"
#include <NxMaterial.h> 

namespace OgrePhysX
{
	class OgrePhysXClass CookerParams
	{
	public:
		Ogre::Vector3 mScale;
		std::map<Ogre::String, NxMaterialIndex> mMaterialBindings;
		bool mAddBackfaces;

		CookerParams() : mScale(Ogre::Vector3(1,1,1)), mAddBackfaces(false) {}
		~CookerParams() {}

		CookerParams& scale(Ogre::Vector3 &scale) { mScale = scale; return *this; }
		CookerParams& materials(std::map<Ogre::String, NxMaterialIndex> &bindings) { mMaterialBindings = bindings; return *this; }
		CookerParams& backfaces(bool addBackfaces) { mAddBackfaces = addBackfaces; return *this; }
	};

	class OgrePhysXClass Cooker
	{
	private:
		Ogre::String mOgreResourceGroup;

	public:
		Cooker(void);
		~Cooker(void);

		/*
		setOgreResourceGroup
		Tells OgrePhysX in which ogre resource group it is supposed to look for resources.
		By default, this is set to "General".
		*/
		void setOgreResourceGroup(Ogre::String group);

		/*
		hasNxMesh
		Checks whether the Ogre Resource System has a resource nxsFile.
		*/
		bool hasNxMesh(Ogre::String nxsFile);
		/*
		loadNxMeshFromFile
		Loads a PhysX mesh from a nxs file. Throws an exception if the file is not found in the Ogre resource system.
		*/
		NxTriangleMesh* loadNxMeshFromFile(Ogre::String nxsFile);

		/*
		cookNxMesh
		cooks an PhysX mesh from an Ogre mesh.
		*/
		void cookNxMesh(Ogre::MeshPtr mesh, NxStream& outputStream, CookerParams &params = CookerParams());				//Ogre::Vector3 scale = Ogre::Vector3(1,1,1), std::map<Ogre::String, NxMaterialIndex> &materialBindings = std::map<Ogre::String, NxMaterialIndex>());
		void cookNxMeshToFile(Ogre::MeshPtr mesh, Ogre::String nxsOutputFile, CookerParams &params = CookerParams());	//Ogre::Vector3 scale = Ogre::Vector3(1,1,1), std::map<Ogre::String, NxMaterialIndex> &materialBindings = std::map<Ogre::String, NxMaterialIndex>());

		/*
		getNxMesh
		Cooks an nx mesh from an ogre mesh and returns it, does not save to file.
		This is faster than getNxMesh(Ogre::MeshPtr, Ogre::String) if the mesh is not cooked yet, otherwise it is much slower.
		Example use case: This method is useful, if you want to rescale an actor with a triangle mesh shape in realtime
		*/
		NxTriangleMesh* getNxMesh(Ogre::MeshPtr mesh, CookerParams &params = CookerParams());	//Ogre::Vector3 scale = Ogre::Vector3(1,1,1), std::map<Ogre::String, NxMaterialIndex> &materialBindings = std::map<Ogre::String, NxMaterialIndex>());

		//Singleton
		static Cooker& getSingleton();
	};

}