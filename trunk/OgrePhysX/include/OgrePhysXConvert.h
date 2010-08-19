#pragma once

#include "NxQuat.h"
#include "NxMat34.h"
#include "NxMat33.h"
#include "NxVec3.h"
#include "NxBounds3.h"
#include "Ogre.h"
#include "OgrePhysXClasses.h"

namespace OgrePhysX
{
	class OgrePhysXClass Convert
	{
	public:
		static Ogre::Vector3 toOgre(const NxVec3 &vec3)
		{
			return Ogre::Vector3(vec3.x, vec3.y, vec3.z);
		}
		static Ogre::Quaternion toOgre(const NxQuat &q)
		{
			return Ogre::Quaternion(q.w, q.x, q.y, q.z);
		}
		static Ogre::AxisAlignedBox toOgre(const NxBounds3 &b)
		{
			return Ogre::AxisAlignedBox(toOgre(b.min), toOgre(b.max));
		}

		static NxVec3 toNx(const Ogre::Vector3 &vec3)
		{
			return NxVec3(vec3.x, vec3.y, vec3.z);
		}
		static NxQuat toNx(const Ogre::Quaternion &q)
		{
			NxQuat nxq; nxq.setWXYZ(q.w, q.x, q.y, q.z);
			return nxq;
		}
		static NxBounds3 toNx(const Ogre::AxisAlignedBox &b)
		{
			NxBounds3 nxb;
			nxb.min = toNx(b.getMinimum());
			nxb.max = toNx(b.getMaximum());
			return nxb;
		}
		static NxMat34 toNx(const Ogre::Vector3 &pos, const Ogre::Quaternion &rot)
		{
			return NxMat34(NxMat33(toNx(rot)), toNx(pos));
		}
	};
}