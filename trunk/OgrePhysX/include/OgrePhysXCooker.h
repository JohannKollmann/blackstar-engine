#pragma once

#include "OgrePhysXClasses.h"
#include "Ogre.h"
#include "NxCooking.h"
#include <NxMaterial.h> 
#include <NxConvexMesh.h> 

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

		struct MeshInfo
		{
			unsigned int numVertices;
			NxArray<NxVec3> vertices;

			NxArray<NxU32> indices;
			unsigned int numTriangles;

			NxArray<NxMaterialIndex> materialIndices;
		};

		void getMeshInfo(Ogre::MeshPtr mesh, CookerParams &params, MeshInfo &outInfo);
		void mergeVertices(MeshInfo &outInfo);
		void insetMesh(MeshInfo &outInfo, float fAmount);
		void cutMesh(MeshInfo &outInfo, NxVec3 vPlanePos, NxVec3 vPlaneDir);

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
		Loads a PhysX triangle mesh from a nxs file. Throws an exception if the file is not found in the Ogre resource system.
		*/
		NxTriangleMesh* loadNxTriangleMeshFromFile(Ogre::String nxsFile);

		/*
		cookNxTriangleMesh
		cooks an PhysX triangle mesh from an Ogre mesh.
		*/
		void cookNxTriangleMesh(Ogre::MeshPtr mesh, NxStream& outputStream, CookerParams &params = CookerParams());
		void cookNxTriangleMeshToFile(Ogre::MeshPtr mesh, Ogre::String nxsOutputFile, CookerParams &params = CookerParams());

		void cookNxConvexMesh(Ogre::MeshPtr mesh, NxStream& outputStream, CookerParams &params = CookerParams());

		void cookNxCCDSkeleton(Ogre::MeshPtr mesh, NxStream& outputStream, CookerParams &params = CookerParams());

		/*
		createNxTriangleMesh
		Cooks an nx mesh from an ogre mesh and returns it, does not save to file.
		This is faster than getNxMesh(Ogre::MeshPtr, Ogre::String) if the mesh is not cooked yet, otherwise it is much slower.
		Example use case: This method is useful, if you want to rescale an actor with a triangle mesh shape in realtime
		*/
		NxTriangleMesh* createNxTriangleMesh(Ogre::MeshPtr mesh, CookerParams &params = CookerParams());

		NxConvexMesh* createNxConvexMesh(Ogre::MeshPtr mesh, CookerParams &params = CookerParams());

		//Singleton
		static Cooker& getSingleton();
	};

}