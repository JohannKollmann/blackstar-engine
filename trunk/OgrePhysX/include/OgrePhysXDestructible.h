
#pragma once

#include "OgrePhysX.h"
#include "OgrePhysXRenderableBinding.h"
#include "Ogre.h"
#include "PxFixedJoint.h"
#include "OgrePhysXScene.h"
#include <unordered_map>
#include "OgrePhysXWorld.h"

/**
This is a runtime for 1nsanes Mesh splitter tool: http://www.ogre3d.org/forums/viewtopic.php?f=11&t=68189
It creates a destructible mesh with pre-defined split parts.
*/

namespace OgrePhysX
{
	class OgrePhysXClass Destructible : public RenderableBinding
	{
	protected:
		std::vector<PxFixedJoint*> mJoints;

		class OgrePhysXClass SplitPart
		{
		private:
			Destructible *mDestructible;
			RenderedActorBinding *mRenderedActor;
			Actor<PxRigidDynamic> mActor;

		public:
			SplitPart(Destructible *destructible, const Ogre::String &meshName);
			~SplitPart();

			std::vector<SplitPart*> mEdges;

			RenderedActorBinding* getRenderedActor() { return mRenderedActor; }
			Actor<PxRigidDynamic>& getActor() { return mActor; }
		};
		std::unordered_map<Ogre::String, SplitPart*> mParts;

		float mBreakForce;
		float mBreakTorque;

		float mDensity;

		Ogre::Vector3 mScale;

		PxMaterial *mMaterial;

		Scene *mScene;

		Ogre::Vector3 mPosition;
		Ogre::Quaternion mOrientation;

		PointRenderable *mPointRenderable;

		SplitPart* getOrCreatePart(const Ogre::String &meshName);

		void createJoint(SplitPart *part1, SplitPart *part2, float breakForce, float breakTorque);

	public:
		Destructible(Scene *scene, const Ogre::String &meshSplitConfigFile, float breakForce = 1000.0f, float breakTorque = 1000.0f, float density = 100.0f, PxMaterial &material = World::getSingleton().getDefaultMaterial(), const Ogre::Vector3 &scale = Ogre::Vector3(1,1,1));
		virtual ~Destructible();

		/**
		Sets the position of the destructible.
		This will reset the broken parts to their original position, so do not use it at runtime.
		*/
		void setPosition(const Ogre::Vector3 &position);

		/**
		Retrieves the position of the destructible.
		*/
		Ogre::Vector3 getPosition() { return mPosition; }

		/**
		Sets the orientation of the destructible.
		This will reset the broken parts to their original orientation, so do not use it at runtime.
		*/
		void setOrientation(const Ogre::Quaternion &orientation);

		/**
		Retrieves the orientation of the destructible.
		*/
		Ogre::Quaternion getOrientation() { return mOrientation; }

		/**
		This is not necessary but useful for listening to object movements / rotations invoked by PhysX.
		*/
		void setPointRenderable(PointRenderable *renderable) { mPointRenderable = renderable; }

		/**
		Freezed or unfreezed all actors.
		*/
		void setFreezed(bool freezed);

		/**
		Sets the user data of every PhysX actor.
		*/
		template<class T>
		void setUserData(T *userData)
		{
			for (auto i = mParts.begin(); i != mParts.end(); i++)
			{
				((NodeRenderable*)i->second->getRenderedActor()->getPointRenderable())->getOgreNode()->getAttachedObject(0)->setUserAny(Ogre::Any(userData));
				i->second->getActor().getPxActor()->userData = userData;
			}
		}

		void setSimulationFilterData(PxFilterData &data);

		void setQueryFilterData(PxFilterData &data);

		void sync();
	};
}