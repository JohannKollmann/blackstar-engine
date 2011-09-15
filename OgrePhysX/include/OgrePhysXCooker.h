#pragma once

#include "OgrePhysXClasses.h"
#include "Ogre.h"
#include "PxVec3.h"
#include "PxPhysics.h" 
#include "common/PxCoreUtilityTypes.h"

namespace OgrePhysX
{
	class OgrePhysXClass Cooker
	{
	private:
		Ogre::String mOgreResourceGroup;

	public:

		class OgrePhysXClass Params
		{
		public:
			Ogre::Vector3 mScale;
			std::map<Ogre::String, PxMaterial*> mMaterialBindings;
			bool mAddBackfaces;

			Params() : mScale(Ogre::Vector3(1,1,1)), mAddBackfaces(false) {}
			~Params() {}

			Params& scale(Ogre::Vector3 &scale) { mScale = scale; return *this; }
			Params& materials(std::map<Ogre::String, PxMaterial*> &bindings) { mMaterialBindings = bindings; return *this; }
			Params& backfaces(bool addBackfaces) { mAddBackfaces = addBackfaces; return *this; }
		};

		class AddedMaterials
		{
		public:
			PxMaterial **materials;
			PxU32  materialCount; 

			AddedMaterials() : materials(nullptr) {}
			~AddedMaterials() { if (materials) delete materials; }
		};

		struct MeshInfo
		{
			//vertex buffer
			std::vector<Ogre::Vector3> vertices;

			//index buffer
			std::vector<int> indices;

			//assigns a material to each triangle.
			std::vector<Ogre::String> materials;
		};

		void getMeshInfo(Ogre::MeshPtr mesh, Params &params, MeshInfo &outInfo);
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
		out_addedMaterials can be passed to obtain information about the used materials (for per triangle materials).
		@see PxShape::setMaterials
		*/
		void cookPxTriangleMesh(Ogre::MeshPtr mesh, PxStream& outputStream, Params &params = Params(), AddedMaterials *out_addedMaterials = nullptr);
		void cookPxTriangleMeshToFile(Ogre::MeshPtr mesh, Ogre::String PxsOutputFile, Params &params = Params(), AddedMaterials *out_addedMaterials = nullptr);

		void cookPxConvexMesh(Ogre::MeshPtr mesh, PxStream& outputStream, Params &params = Params());

		void cookPxCCDSkeleton(Ogre::MeshPtr mesh, PxStream& outputStream, Params &params = Params());

		/*
		createPxTriangleMesh
		Cooks an Px mesh from an ogre mesh and returns it, does not save to file.
		*/
		PxTriangleMesh* createPxTriangleMesh(Ogre::MeshPtr mesh, Params &params = Params(), AddedMaterials *out_addedMaterials = nullptr);

		PxConvexMesh* createPxConvexMesh(Ogre::MeshPtr mesh, Params &params = Params());

		//Singleton
		static Cooker& getSingleton();
	};

}