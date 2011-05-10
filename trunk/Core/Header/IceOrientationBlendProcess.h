#pragma once

#include "IceProcessNode.h"
#include "IceMessageSystem.h"
#include "Ice3D.h"

namespace Ice
{

	class OrientationBlendProcess : public ProcessNode, public PhysicsMessageListener
	{
	private:
		Transformable3D *mRotationObject;
		Ogre::Quaternion mObjectInitialOrientation;
		Ogre::Quaternion mTargetOrientation;
		float mCurrentBlendPosition;
		float mTotalDuration;

	protected:
		void OnSetActive(bool active) {}

	public:
		OrientationBlendProcess(Transformable3D *object, Ogre::Quaternion target, float duration);
		~OrientationBlendProcess() {}

		void ReceiveMessage(Msg &msg);
	};

}