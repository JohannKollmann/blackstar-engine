#pragma once

#include "OgrePhysXClasses.h"
#include "Ogre.h"
#include <NxShapeDesc.h> 
#include <NxTriangleMeshShapeDesc.h> 
#include "NxBoxShapeDesc.h"
#include "NxSphereShapeDesc.h"
#include "NxCapsuleShapeDesc.h"
#include "NxPlaneShapeDesc.h"
#include "OgrePhysXCooker.h"

namespace OgrePhysX
{
	class OgrePhysXClass Shape
	{
	public:
		virtual NxShapeDesc* getDesc() = 0;
	};
	class OgrePhysXClass PrimitiveShape : public Shape
	{
	protected:
		float max(float a, float b)
		{
			return (a > b) ? a : b;
		}
	public:
		PrimitiveShape& density(float density);
		PrimitiveShape& mass(float mass);
		PrimitiveShape& setStatic();
		PrimitiveShape& setTrigger();

		PrimitiveShape& localPose(Ogre::Vector3 offset, Ogre::Quaternion rotation = Ogre::Quaternion());
		PrimitiveShape& material(NxMaterialIndex material);
		PrimitiveShape& group(NxCollisionGroup group);
		PrimitiveShape& groupsMask(NxGroupsMask mask);
	};
	class OgrePhysXClass PlaneShape : public PrimitiveShape
	{
	private:
		NxPlaneShapeDesc mDesc;
	public:
		PlaneShape(Ogre::Vector3 normal, float d);
		NxShapeDesc* getDesc() { return &mDesc; }
	};
	class OgrePhysXClass BoxShape : public PrimitiveShape
	{
	private:
		NxBoxShapeDesc mDesc;
	public:
		BoxShape(Ogre::Vector3 dims);
		BoxShape(Ogre::Entity *ent, Ogre::Vector3 scale = Ogre::Vector3(1,1,1), float minDim = 0.1f);
		NxShapeDesc* getDesc() { return &mDesc; }
	};
	class OgrePhysXClass SphereShape : public PrimitiveShape
	{
	private:
		NxSphereShapeDesc mDesc;
	public:
		SphereShape(float radius);
		SphereShape(Ogre::Entity *ent, float scale = 1.0f, float minDim = 0.05);
		NxShapeDesc* getDesc() { return &mDesc; }
	};
	class OgrePhysXClass CapsuleShape : public PrimitiveShape
	{
	private:
		NxCapsuleShapeDesc mDesc;
	public:
		CapsuleShape(float radius, float height);
		NxShapeDesc* getDesc() { return &mDesc; }
	};

	class OgrePhysXClass BaseMeshShape : public Shape
	{
	protected:
		NxTriangleMeshShapeDesc mDesc;
	public:
		virtual ~BaseMeshShape() {}

		BaseMeshShape& material(NxMaterialIndex material);
		BaseMeshShape& group(NxCollisionGroup group);
		BaseMeshShape& groupsMask(NxGroupsMask mask);
	};
	class OgrePhysXClass CookedMeshShape : public BaseMeshShape
	{
	public:
		CookedMeshShape(NxTriangleMesh *mesh);
		CookedMeshShape(Ogre::String nxsFile);
		~CookedMeshShape() {}
		NxShapeDesc* getDesc() { return &mDesc; }
	};

	class OgrePhysXClass RTMeshShape : public BaseMeshShape
	{
	private:
		Ogre::MeshPtr mOgreMesh;
		CookerParams mCookerParams;

	public:

		RTMeshShape(Ogre::MeshPtr mesh);
		~RTMeshShape() {}
		NxShapeDesc* getDesc();

		RTMeshShape& scale(Ogre::Vector3 scale);
		RTMeshShape& materials(std::map<Ogre::String, NxMaterialIndex> &materialBindings);
		RTMeshShape& backfaces(bool addBackfaces);
	};
};