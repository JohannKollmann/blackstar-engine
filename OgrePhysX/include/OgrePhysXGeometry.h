
#include "Ogre.h"
#include "PxPhysics.h"
#include "PxSphereGeometry.h"
#include "PxBoxGeometry.h"
#include "PxCapsuleGeometry.h"
#include "PxConvexMeshGeometry.h"
#include "PxTriangleMeshGeometry.h"
#include "PxRigidActor.h"
#include "OgrePhysXConvert.h"
#include "OgrePhysXCooker.h"

namespace OgrePhysX
{
	/**
	Set of helper functions to create PhysX geometry from an Ogre entity
	*/

	class Geometry
	{
	public:

		static PxSphereGeometry sphereGeometry(Ogre::Entity *entity)
		{
			float sphereShapeRadius = entity->getBoundingRadius();
			return PxSphereGeometry(sphereShapeRadius);
		}

		static PxBoxGeometry boxGeometry(Ogre::Entity *entity)
		{
			return PxBoxGeometry(Convert::toPx(entity->getBoundingBox().getHalfSize()));
		}

		static PxBoxGeometry boxGeometry(const Ogre::Vector3 &dimension)
		{
			return PxBoxGeometry(Convert::toPx(dimension));
		}

		static PxConvexMeshGeometry convexMeshGeometry(Ogre::MeshPtr mesh, Cooker::Params params = Cooker::Params())
		{
			return PxConvexMeshGeometry(Cooker::getSingleton().createPxConvexMesh(mesh, params));
		}

		static PxTriangleMeshGeometry triangleMeshGeometry(Ogre::MeshPtr mesh, Cooker::Params params = Cooker::Params())
		{
			return PxTriangleMeshGeometry(Cooker::getSingleton().createPxTriangleMesh(mesh, params));
		}
	};
}