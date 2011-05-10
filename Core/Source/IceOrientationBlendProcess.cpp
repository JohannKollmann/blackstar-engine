
#include "IceOrientationBlendProcess.h"

namespace Ice
{

	OrientationBlendProcess::OrientationBlendProcess(Transformable3D *object, Ogre::Quaternion target, float duration)
		: mRotationObject(object), mTargetOrientation(target), mTotalDuration(duration), mObjectInitialOrientation(object->GetGlobalOrientation()), mCurrentBlendPosition(0)
	{
		JoinNewsgroup(GlobalMessageIDs::UPDATE_PER_FRAME);
	}

	void OrientationBlendProcess::ReceiveMessage(Msg &msg)
	{
		if (GetIsActive() && msg.typeID == GlobalMessageIDs::UPDATE_PER_FRAME)
		{
			mRotationObject->SetGlobalOrientation(Ogre::Quaternion::Slerp(mCurrentBlendPosition, mObjectInitialOrientation, mTargetOrientation, true));
			if (mCurrentBlendPosition >= 1) TerminateProcess();
			float time = msg.params.GetFloat("TIME");
			mCurrentBlendPosition += time / mTotalDuration;
		}
	}

}