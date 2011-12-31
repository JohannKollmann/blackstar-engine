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

		static PxSphereGeometry sphereGeometry(Ogre::Entity *entity, PxTransform &outShapeOffset, float scale = 1.0f)
		{
			outShapeOffset.p = Convert::toPx(entity->getBoundingBox().getCenter());
			float sphereShapeRadius = entity->getBoundingRadius() * scale;
			return PxSphereGeometry(sphereShapeRadius);
		}

		static PxBoxGeometry boxGeometry(Ogre::Entity *entity, PxTransform &outShapeOffset, const Ogre::Vector3 &scale = Ogre::Vector3(1,1,1))
		{
			outShapeOffset.p = Convert::toPx(entity->getBoundingBox().getCenter());
			return PxBoxGeometry(Convert::toPx(entity->getBoundingBox().getHalfSize() * scale));
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