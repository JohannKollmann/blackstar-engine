#pragma once

#include "PxPhysics.h"
#include "PxQuat.h"
#include "PxMat44.h"
#include "PxMat33.h"
#include "PxVec3.h"
#include "foundation/PxBounds3.h"
#include "Ogre.h"
#include "OgrePhysXClasses.h"

namespace OgrePhysX
{
	namespace Convert
	{
		static Ogre::Vector3 toOgre(const PxVec3 &vec3)
		{
			return Ogre::Vector3(vec3.x, vec3.y, vec3.z);
		}
		static Ogre::Quaternion toOgre(const PxQuat &q)
		{
			return Ogre::Quaternion(q.w, q.x, q.y, q.z);
		}
		static Ogre::AxisAlignedBox toOgre(const PxBounds3 &b)
		{
			return Ogre::AxisAlignedBox(toOgre(b.minimum), toOgre(b.maximum));
		}

		static PxVec3 toPx(const Ogre::Vector3 &vec3)
		{
			return PxVec3(vec3.x, vec3.y, vec3.z);
		}
		static PxQuat toPx(const Ogre::Quaternion &q)
		{
			PxQuat pxq; pxq. w = q.w; pxq.x = q.x; pxq.y = q.y; pxq.z = q.z;
			return pxq;
		}
		static PxBounds3 toPx(const Ogre::AxisAlignedBox &b)
		{
			PxBounds3 Pxb;
			Pxb.minimum = toPx(b.getMinimum());
			Pxb.maximum = toPx(b.getMaximum());
			return Pxb;
		}
		static PxMat44 toPx(const Ogre::Vector3 &pos, const Ogre::Quaternion &rot)
		{
			return PxMat44(PxMat33(toPx(rot)), toPx(pos));
		}
	};
}