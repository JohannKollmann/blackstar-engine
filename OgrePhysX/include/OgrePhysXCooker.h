#pragma once

#include "OgrePhysXClasses.h"
#include "Ogre.h"
#include "PxVec3.h"
#include "PxPhysics.h" 
#include "common/PxCoreUtilityTypes.h"

namespace OgrePhysX
{
	class OgrePhysXClass CookerParams
	{
	public:
		Ogre::Vector3 mScale;
		std::map<Ogre::String, PxMaterialTableIndex> mMaterialBindings;
		bool mAddBackfaces;

		CookerParams() : mScale(Ogre::Vector3(1,1,1)), mAddBackfaces(false) {}
		~CookerParams() {}

		CookerParams& scale(Ogre::Vector3 &scale) { mScale = scale; return *this; }
		CookerParams& materials(std::map<Ogre::String, PxMaterialTableIndex> &bindings) { mMaterialBindings = bindings; return *this; }
		CookerParams& backfaces(bool addBackfaces) { mAddBackfaces = addBackfaces; return *this; }
	};

	class OgrePhysXClass Cooker
	{
	private:
		Ogre::String mOgreResourceGroup;

	public:

		struct MeshInfo
		{
			//vertex buffer
			std::vector<Ogre::Vector3> vertices;

			//index buffer
			std::vector<int> indices;

			//assigns a material to each triangle.
			std::vector<Ogre::String> materials;
		};

		void getMeshInfo(Ogre::MeshPtr mesh, CookerParams &params, MeshInfo &outInfo);
		void mergeVertices(MeshInfo &outInfo, float fMergeDist = 1e-3f);
		void insetMesh(MeshInfo &outInfo, float fAmount);
		void cutMesh(MeshInfo &outInfo, physx::pubfnd::PxVec3 vPlanePos, physx::pubfnd::PxVec3 vPlaneDir);

		Cooker(void);
		~Cooker(void);

		/*
		setOgreResourceGroup
		Tells OgrePhysX in which ogre resource group it is supposed to look for resources.
		By default, this is set to "General".
		*/
		void setOgreResourceGroup(Ogre::String group);

		/*
		hasPxMesh
		Checks whether the Ogre Resource System has a resource PxsFile.
		*/
		bool hasPxMesh(Ogre::String PxsFile);
		/*
		loadPxMeshFromFile
		Loads a PhysX triangle mesh from a Pxs file. Throws an exception if the file is not found in the Ogre resource system.
		*/
		PxTriangleMesh* loadPxTriangleMeshFromFile(Ogre::String PxsFile);

		/*
		cookPxTriangleMesh
		cooks an PhysX triangle mesh from an Ogre mesh.
		*/
		void cookPxTriangleMesh(Ogre::MeshPtr mesh, PxStream& outputStream, CookerParams &params = CookerParams());
		void cookPxTriangleMeshToFile(Ogre::MeshPtr mesh, Ogre::String PxsOutputFile, CookerParams &params = CookerParams());

		void cookPxConvexMesh(Ogre::MeshPtr mesh, PxStream& outputStream, CookerParams &params = CookerParams());

		void cookPxCCDSkeleton(Ogre::MeshPtr mesh, PxStream& outputStream, CookerParams &params = CookerParams());

		/*
		createPxTriangleMesh
		Cooks an Px mesh from an ogre mesh and returns it, does not save to file.
		This is faster than getPxMesh(Ogre::MeshPtr, Ogre::String) if the mesh is not cooked yet, otherwise it is much slower.
		Example use case: This method is useful, if you want to rescale an actor with a triangle mesh shape in realtime
		*/
		PxTriangleMesh* createPxTriangleMesh(Ogre::MeshPtr mesh, CookerParams &params = CookerParams());

		PxConvexMesh* createPxConvexMesh(Ogre::MeshPtr mesh, CookerParams &params = CookerParams());

		//Singleton
		static Cooker& getSingleton();
	};

}