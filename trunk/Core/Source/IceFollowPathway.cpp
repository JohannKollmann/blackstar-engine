
#include "IceFollowPathway.h"
#include "IceNavigationMesh.h"
#include "IceGOCAI.h"
#include "IceGOCCharacterController.h"
#include "IceGameObject.h"
#include "IceGOCCharacterController.h"
#include "NxController.h"
#include "IceMain.h"
#include "IceAIManager.h"

namespace Ice
{

	FollowPathway::FollowPathway(GOCAI *ai, Ogre::String targetWP, float radius)
	{
		mAIObject = ai;
		mRadius = radius;
		mTargetWP = targetWP;
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
		AIManager::Instance().FindPath(pos, mTargetWP, mPath);

		if (!mPath.empty())
		{
			mDirectionBlender.StartBlend(mAIObject->GetOwner()->GetGlobalOrientation() * Ogre::Vector3::UNIT_Z, mPath[0]->GetGlobalPosition()-pos);
		}
	}

	bool FollowPathway::Update(float time)
	{
		if (mPath.empty())
		{
			mAIObject->BroadcastMovementState(0);
			return true;
		}

		Ogre::Vector3 currPos = mAIObject->GetOwner()->GetGlobalPosition();

		Ogre::Vector3 myDirection = mAIObject->GetOwner()->GetGlobalOrientation() * Ogre::Vector3::UNIT_Z;

		float dist = currPos.distance(mPath[0]->GetGlobalPosition());
		if (dist < mRadius)
		{
			Ogre::Vector3 oldDir = mPath[0]->GetGlobalPosition()-currPos;
			mPath.erase(mPath.begin());
			if (mPath.empty())
			{
				mAIObject->BroadcastMovementState(0);
				return true;
			}
			Ogre::Vector3 targetDir = mPath[0]->GetGlobalPosition()-currPos;
			mDirectionBlender.StartBlend(myDirection, targetDir, 0.2f);
		}

		Ogre::Vector3 direction = mPath[0]->GetGlobalPosition()-currPos;
		direction.normalise();

		/*if (mAvoidingObstacle)
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
		}*/

		int movementstate = CharacterMovement::FORWARD;
		/*if (ObstacleCheck(direction * 3.0f))
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
		else*/
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