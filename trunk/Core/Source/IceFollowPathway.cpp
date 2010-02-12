
#include "IceFollowPathway.h"
#include "IcePathfinder.h"
#include "IceGOCAI.h"
#include "IceGOCCharacterController.h"
#include "IceGameObject.h"
#include "IceGOCCharacterController.h"
#include "NxController.h"
#include "IceMain.h"

namespace Ice
{

	FollowPathway::FollowPathway(GOCAI *ai, Ogre::String targetWP, float radius)
	{
		mAIObject = ai;
		mRadius = radius;
		mTargetWP = targetWP;
		mCurrentTarget = mPath.begin();
		mAvoidingObstacle = false;

		//Sweep cache for dynamic obstacle avoiding
		mSweepCache = Main::Instance().GetPhysXScene()->getNxScene()->createSweepCache(); 
	};

	FollowPathway::~FollowPathway()
	{
		mPath.clear();
		Main::Instance().GetPhysXScene()->getNxScene()->releaseSweepCache(mSweepCache);
	};

	bool FollowPathway::ObstacleCheck(Ogre::Vector3 motion)
	{
		if (!mSweepActor) return false;

		int maxNumResult  = 10;
		NxSweepQueryHit *sqh_result = new NxSweepQueryHit[maxNumResult];
		NxU32 numHits = mSweepActor->linearSweep(OgrePhysX::Convert::toNx(motion), NX_SF_DYNAMICS|NX_SF_ALL_HITS, 0, maxNumResult, sqh_result, 0, mSweepCache);
		bool obstacleHit = false;
		for (NxU32 i = 0; i < numHits; i++)
		{
			NxSweepQueryHit hit = sqh_result[i];
			if (hit.hitShape->getGroup() == DEFAULT || hit.hitShape->getGroup() == CHARACTER)
			{
				return true;
			}
		}
		delete sqh_result;

		return false;
	}
	
	void FollowPathway::Leave()
	{
		Pause();
	}
	void FollowPathway::Pause()
	{
		mAIObject->BroadcastMovementState(0);
	}

	void FollowPathway::OnEnter()
	{
		GOCCharacterController *character = (GOCCharacterController*)mAIObject->GetOwner()->GetComponent("Physics", "CharacterController");
		mSweepActor = 0;
		if (character) mSweepActor = character->GetNxController()->getActor();

		Ogre::Vector3 pos = mAIObject->GetOwner()->GetGlobalPosition();
		Pathfinder::Instance().FindPath(pos, mTargetWP, &mPath, mSweepActor);
		mCurrentTarget = mPath.begin();

		if (mCurrentTarget != mPath.end())
		{
			mDirectionBlender.StartBlend(mAIObject->GetOwner()->GetGlobalOrientation() * Ogre::Vector3::UNIT_Z, (*mCurrentTarget)-pos);
		}
	}

	bool FollowPathway::Update(float time)
	{
		if (mCurrentTarget == mPath.end())
		{
			mAIObject->BroadcastMovementState(0);
			return true;
		}

		Ogre::Vector3 currPos = mAIObject->GetOwner()->GetGlobalPosition();

		Ogre::Vector3 direction = (*mCurrentTarget)-currPos;
		direction.normalise();
		float dist = currPos.distance(*mCurrentTarget);
		if (dist < mRadius)
		{
			Ogre::Vector3 oldDir = (*mCurrentTarget)-currPos;
			mCurrentTarget++;
			if (mCurrentTarget == mPath.end())
			{
				mAIObject->BroadcastMovementState(0);
				return true;
			}
			Ogre::Vector3 targetDir = (*mCurrentTarget)-currPos;
			mDirectionBlender.StartBlend(oldDir, targetDir);
		}

		if (mAvoidingObstacle)
		{
			if (ObstacleCheck(direction * 3.0f)) direction = mAvoidObstacleVector;
			else
			{
				mAvoidingObstacle = false;
				mDirectionBlender.StartBlend(mAvoidObstacleVector, direction);
			}
		}
		else
		{
		}

		int movementstate = CharacterMovement::FORWARD;
		if (ObstacleCheck(direction * 3.0f))
		{
			mAvoidingObstacle = true;
			Ogre::Quaternion q = Ogre::Quaternion(Ogre::Radian(Ogre::Degree(90)), Ogre::Vector3::UNIT_Y);
			mAvoidObstacleVector = q * direction;
			mDirectionBlender.StartBlend(direction, mAvoidObstacleVector);
		}

		if (mDirectionBlender.HasNext())
		{
			direction = mDirectionBlender.Next(time);
		}
		else
		{
			direction.y = 0;
			direction.normalise();
		}
		Ogre::Quaternion quat = Ogre::Vector3::UNIT_Z.getRotationTo(direction);
		mAIObject->GetOwner()->SetGlobalOrientation(quat);
		mAIObject->BroadcastMovementState(movementstate);
		return false;
	}

}