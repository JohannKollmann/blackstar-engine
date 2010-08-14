
#include "OgrePhysXShapes.h"
#include "OgrePhysXConvert.h"
#include "NxTriangleMeshDesc.h"
#include "OgrePhysXWorld.h"

#pragma warning (disable:4482)

namespace OgrePhysX
{
	PrimitiveShape& PrimitiveShape::density(float density)
	{
		getDesc()->density = density;
		if (density != 0) getDesc()->mass = 0;
		return (*this);
	}
	PrimitiveShape& PrimitiveShape::mass(float mass)
	{
		getDesc()->mass = mass;
		if (mass != 0) getDesc()->density = 0;
		return (*this);
	}
	PrimitiveShape& PrimitiveShape::setStatic()
	{
		getDesc()->mass = 0;
		getDesc()->density = 0;
		return (*this);
	}
	PrimitiveShape& PrimitiveShape::localPose(Ogre::Vector3 offset, Ogre::Quaternion rotation)
	{
		getDesc()->localPose = NxMat34(NxMat33(Convert::toNx(rotation)), Convert::toNx(offset));
		return (*this);
	}
	PrimitiveShape& PrimitiveShape::group(NxCollisionGroup group)
	{
		getDesc()->group = group;
		return (*this);
	}
	PrimitiveShape& PrimitiveShape::groupsMask(NxGroupsMask mask)
	{
		getDesc()->groupsMask = mask;
		return (*this);
	}
	PrimitiveShape& PrimitiveShape::material(NxMaterialIndex material)
	{
		getDesc()->materialIndex = material;
		return (*this);
	}
	PrimitiveShape& PrimitiveShape::setTrigger()
	{
		getDesc()->shapeFlags |= NX_TRIGGER_ENABLE;
		return setStatic();
	}
	PlaneShape::PlaneShape(Ogre::Vector3 normal, float d)
	{
		mDesc.setToDefault();
		mDesc.mass = 0;
		mDesc.density = 0;
		mDesc.normal = Convert::toNx(normal);
		mDesc.d = d;
		mDesc.shapeFlags |= NX_SF_POINT_CONTACT_FORCE;
	}
	BoxShape::BoxShape(Ogre::Vector3 dims)
	{
		mDesc.setToDefault();
		mDesc.dimensions.set(dims.x * 0.5f, dims.y * 0.5f, dims.z * 0.5f);
		mDesc.shapeFlags |= NX_SF_POINT_CONTACT_FORCE;
	}
	BoxShape::BoxShape(Ogre::Entity *ent, Ogre::Vector3 scale, float minDim)
	{
		mDesc.setToDefault();
		Ogre::Vector3 size = ent->getBoundingBox().getSize() * scale;
		mDesc.dimensions.set(max(size.x * 0.5f, minDim), max(size.y * 0.5f, minDim), max(size.z * 0.5f, minDim));
		mDesc.shapeFlags |= NX_SF_POINT_CONTACT_FORCE;
	}
	SphereShape::SphereShape(float radius)
	{
		mDesc.setToDefault();
		mDesc.radius = radius;
		mDesc.shapeFlags |= NX_SF_POINT_CONTACT_FORCE;
	}
	SphereShape::SphereShape(Ogre::Entity *ent, float scale, float minDim)
	{
		mDesc.setToDefault();
		mDesc.radius = max(ent->getBoundingRadius() * scale, minDim);
		mDesc.shapeFlags |= NX_SF_POINT_CONTACT_FORCE;
	}
	CapsuleShape::CapsuleShape(float radius, float height)
	{
		mDesc.setToDefault();
		mDesc.radius = radius;
		mDesc.height = height;
		mDesc.shapeFlags |= NX_SF_POINT_CONTACT_FORCE;
	}

	CookedMeshShape::CookedMeshShape(NxTriangleMesh *mesh)
	{
		mDesc.meshData = mesh;
		mDesc.shapeFlags |= NX_SF_FEATURE_INDICES;
		mDesc.shapeFlags |= NX_SF_POINT_CONTACT_FORCE;
	}
	CookedMeshShape::CookedMeshShape(Ogre::String nxsFile)
	{
		mDesc.meshData = Cooker::getSingleton().loadNxTriangleMeshFromFile(nxsFile);
		mDesc.shapeFlags |= NX_SF_FEATURE_INDICES;
	}

	RTMeshShape::RTMeshShape(Ogre::MeshPtr mesh)
	{ 
		mOgreMesh = mesh;
		mDesc.shapeFlags |= NX_SF_FEATURE_INDICES;
		mDesc.shapeFlags |= NX_SF_POINT_CONTACT_FORCE;
	}
	NxShapeDesc* RTMeshShape::getDesc()
	{
		mDesc.meshData = Cooker::getSingleton().createNxTriangleMesh(mOgreMesh, mCookerParams);
		return &mDesc;
	}
	RTMeshShape& RTMeshShape::scale(Ogre::Vector3 scale)
	{
		mCookerParams.scale(scale);
		return (*this);
	}
	RTMeshShape& RTMeshShape::materials(std::map<Ogre::String, NxMaterialIndex> &materialBindings)
	{
		mCookerParams.materials(materialBindings);
		return (*this);
	}

	RTMeshShape& RTMeshShape::backfaces(bool addBackfaces)
	{
		mCookerParams.backfaces(addBackfaces);
		return (*this);
	}

	RTConvexMeshShape::RTConvexMeshShape(Ogre::MeshPtr mesh)
	{ 
		mOgreMesh = mesh;
		mDesc.shapeFlags |= NX_SF_POINT_CONTACT_FORCE;
	}
	RTConvexMeshShape& RTConvexMeshShape::scale(Ogre::Vector3 scale)
	{
		mCookerParams.scale(scale);
		return (*this);
	}
	NxShapeDesc* RTConvexMeshShape::getDesc()
	{
		mDesc.meshData = Cooker::getSingleton().createNxConvexMesh(mOgreMesh, mCookerParams);
		return &mDesc;
	}

}