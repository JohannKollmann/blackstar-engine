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

#include "PxPhysics.h"
#include "PxQuat.h"
#include "PxMat44.h"
#include "PxMat33.h"
#include "PxVec3.h"
#include "PxBounds3.h"
#include "Ogre.h"
#include "OgrePhysXClasses.h"

namespace OgrePhysX
{
	using namespace physx;

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