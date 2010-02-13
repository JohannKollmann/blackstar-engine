	
#pragma once

#include "Ogre.h"

namespace Ice
{

	class DirectionYawBlender
	{
	private:
		Ogre::Vector3 mOldDirection;
		float mTargetRotation;
		float mInterpolationTimeStep;
		float mInterpolationStatus;

	public:
		DirectionYawBlender()
		{
			mInterpolationStatus = 1.0f;
		}
		~DirectionYawBlender() {}

		void StartBlend(Ogre::Vector3 oldDir, Ogre::Vector3 newDir, float total_time = 0.5f)
		{
			oldDir.y = 0;
			oldDir.normalise();
			newDir.y = 0;
			newDir.normalise();
			mOldDirection = oldDir;
			mInterpolationStatus = 0.0f;
			mInterpolationTimeStep = 1 / total_time;
			if ((1.0f + mOldDirection.dotProduct(newDir)) < 0.0001f)            //Workaround for 180 degree rotation                         
			{
				mTargetRotation = Ogre::Math::PI;
			}
			else
			{
				mTargetRotation = mOldDirection.getRotationTo(newDir).getYaw().valueRadians();
			}
		}
		bool HasNext()
		{
			return (mInterpolationStatus < 1.0f);
		}
		Ogre::Vector3 Next(float time)
		{
			if (HasNext())
				mInterpolationStatus += mInterpolationTimeStep * time;
			float rot = mTargetRotation * mInterpolationStatus;
			return Ogre::Quaternion(Ogre::Radian(rot), Ogre::Vector3::UNIT_Y) * mOldDirection;
		}
	};

}