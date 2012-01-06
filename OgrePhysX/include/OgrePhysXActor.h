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

#include "OgrePhysXConvert.h"

namespace OgrePhysX
{

	/**
	Helper class that wraps an PxRigidActor to keep code clean.
	Provides set/get position/orientation methods that accept Ogre math types.
	*/

	template<class T>		//T extends PxRigidActor
	class Actor
	{
	protected:
		T *mActor;

	public:
		Actor() : mActor(nullptr) {}
		Actor(T *actor) : mActor(actor) {}
		virtual ~Actor() {}

		void setPxActor(T *actor)
		{
			mActor = actor;
		}

		inline T* getPxActor() const
		{
			return mActor;
		}

		///Retrieves the first shape of the actor.
		PxShape* getFirstShape()
		{
			PxShape *shapes[1];
			mActor->getShapes(shapes, 1);
			return shapes[0];
		}

		inline void setGlobalPosition(const Ogre::Vector3 &position)
		{
			mActor->setGlobalPose(PxTransform(Convert::toPx(position), mActor->getGlobalPose().q));
		}

		inline Ogre::Vector3 getGlobalPosition()
		{
			return Convert::toOgre(mActor->getGlobalPose().p);
		}

		inline void setGlobalOrientation(const Ogre::Quaternion &q)
		{
			mActor->setGlobalPose(PxTransform(mActor->getGlobalPose().p, Convert::toPx(q)));
		}

		inline Ogre::Quaternion getGlobalOrientation()
		{
			return Convert::toOgre(mActor->getGlobalPose().q);
		}

		inline void setGlobalTransform(const Ogre::Vector3 &position, const Ogre::Quaternion &q)
		{
			mActor->setGlobalPose(PxTransform(Convert::toPx(position), Convert::toPx(q)));
		}

		///Freezed / unfreezes the actor.
		void setFreezed(bool freezed)
		{
			unsigned int numShapes = mActor->getNbShapes();
			PxShape **shapes = new PxShape*[numShapes];
			mActor->getShapes(shapes, numShapes);
			for (unsigned int i = 0; i < numShapes; i++)
				shapes[i]->setFlag(PxShapeFlag::eSIMULATION_SHAPE, !freezed);

			delete[] shapes;

			mActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, freezed);

			//clear all forces
			mActor->clearForce(PxForceMode::eFORCE);
			mActor->clearForce(PxForceMode::eIMPULSE);
			mActor->clearForce(PxForceMode::eVELOCITY_CHANGE);
			mActor->clearForce(PxForceMode::eACCELERATION);
		}
	};
}