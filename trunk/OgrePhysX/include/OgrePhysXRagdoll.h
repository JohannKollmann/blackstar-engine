
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