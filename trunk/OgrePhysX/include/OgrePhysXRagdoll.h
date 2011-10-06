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
#include "OgrePhysX.h"
#include "OgrePhysXRenderableBinding.h"
#include "PxPhysicsAPI.h"

/**
This class only manages Ogre bone <-> PhysX actor synchronisation, it does not create bone actors or joints.
*/

namespace OgrePhysX
{

	class OgrePhysXClass Ragdoll : public RenderableBinding
	{
		friend class Scene;

	public:

		struct BoneActorBinding
		{
			Actor<PxRigidBody> actor;
			Ogre::Bone *bone;
			Ogre::Vector3 offset;
		};

	private:
		PxScene *mPxScene;
		Ogre::Entity *mEntity;
		Ogre::SceneNode *mNode;
		bool mControlledByActors;
		std::vector<BoneActorBinding> mSkeletonBinding;

		Ragdoll(PxScene* scene, Ogre::Entity *ent, Ogre::SceneNode *node);
		~Ragdoll(void);

	public:

		void addBoneActorBinding(Ogre::Bone *bone, PxRigidBody *body, const Ogre::Vector3 &offset);

		void setControlToActors();
		void setControlToBones();
		bool isControlledByActors() { return mControlledByActors; }
		void resetBones();

		void updateBoneActors();
		void updateVisualBones();
		void setAllBonesToManualControl(bool manual);

		Ogre::Entity* getEntity() { return mEntity; }

		void sync();
	};

}