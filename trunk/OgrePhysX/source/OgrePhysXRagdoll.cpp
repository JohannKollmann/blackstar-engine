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

#include "OgrePhysXRagdoll.h"
#include <fstream>
#include "OgrePhysXConvert.h"

#pragma warning (disable:4482)

namespace OgrePhysX
{

	Ragdoll::~Ragdoll(void)
	{
	}

	Ragdoll::Ragdoll(PxScene *scene, Ogre::Entity *ent, Ogre::SceneNode *node)
	{
		mPxScene = scene;
		mNode = node;
		Ogre::Vector3 scale = mNode->getScale();
		mEntity = ent;
		if (!mEntity->getSkeleton()) OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR, "Entity " + mEntity->getName() + " has no skeleton!", "OgrePhysX::Ragdoll::Ragdoll");
	}

	void Ragdoll::addBoneActorBinding(Ogre::Bone *bone, PxRigidBody *body, const Ogre::Vector3 &offset)
	{
		BoneActorBinding binding;
		binding.actor.setPxActor(body);
		binding.bone = bone;
		binding.offset = offset;
	}

	void Ragdoll::updateBoneActors()
	{
		Ogre::Vector3 scale = mNode->_getDerivedScale();
		for (std::vector<BoneActorBinding>::iterator i = mSkeletonBinding.begin(); i != mSkeletonBinding.end(); ++i)
		{
			Ogre::Vector3 position = (mNode->_getDerivedOrientation() * ((*i).bone->_getDerivedPosition()+(*i).offset) * scale) + mNode->_getDerivedPosition();
			Ogre::Quaternion orientation = mNode->_getDerivedOrientation() * (*i).bone->_getDerivedOrientation();
			(*i).actor.setGlobalTransform(position, orientation);
		}
	}

	void Ragdoll::updateVisualBones()
	{
		for (std::vector<BoneActorBinding>::iterator i = mSkeletonBinding.begin(); i != mSkeletonBinding.end(); i++)
		{
			Ogre::Bone *bone = (*i).bone;
			if (!(*i).bone->getParent())
			{
				Ogre::Vector3 newPos = (*i).actor.getGlobalPosition() - (mNode->_getDerivedOrientation() * (bone->_getDerivedPosition() * mNode->getScale()));
				mNode->setPosition(newPos);
			}
			bone->setOrientation((*i).actor.getGlobalOrientation());	//bone inherit orientation is set to false
		}
	}

	void Ragdoll::setAllBonesToManualControl(bool manual)
	{
	   Ogre::SkeletonInstance* skeletonInst = mEntity->getSkeleton();
	   Ogre::Skeleton::BoneIterator boneI = skeletonInst->getBoneIterator();

	   while(boneI.hasMoreElements())
	   {
		   Ogre::Bone *bone = boneI.getNext();
		   bone->setManuallyControlled(manual); 
	   }

	}

	void Ragdoll::resetBones()
	{
	   Ogre::SkeletonInstance* skeletonInst = mEntity->getSkeleton();
		Ogre::Skeleton::BoneIterator boneI = skeletonInst->getBoneIterator();

		while(boneI.hasMoreElements())
			boneI.getNext()->reset();
	}

	void Ragdoll::setControlToActors()
	{
		mControlledByActors = true;
		Ogre::AnimationStateSet* set = mEntity->getAllAnimationStates();
		Ogre::AnimationStateIterator it = set->getAnimationStateIterator();
		Ogre::AnimationState *anim = 0;   
		while(it.hasMoreElements())
		{
			anim = it.getNext();
			anim->setLoop(false);
			anim->setEnabled(false);
			anim->setWeight(0);
		 }

		setAllBonesToManualControl(true);
		for (std::vector<BoneActorBinding>::iterator i = mSkeletonBinding.begin(); i != mSkeletonBinding.end(); ++i)
		{
			(*i).bone->setInheritOrientation(false);

			//TODO: unfreeze actor
			//i->actor.getPxActor()->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, mIsKinematic);
		}
	}

	void Ragdoll::setControlToBones()
	{
		mControlledByActors = false;
		setAllBonesToManualControl(false);
		for (std::vector<BoneActorBinding>::iterator i = mSkeletonBinding.begin(); i != mSkeletonBinding.end(); ++i)
		{
			//TODO: freeze actor
		}
	}

	void Ragdoll::sync()
	{
		if (mControlledByActors)
			updateVisualBones();
	}

}