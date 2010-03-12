
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

	NxActor* FollowPathway::ObstacleCheck(Ogre::Vector3 motion)
	{
		if (!mSweepActor) return 0;

		int maxNumResult  = 10;
		NxSweepQueryHit *sqh_result = new NxSweepQueryHit[maxNumResult];
		NxU32 numHits = mSweepActor->linearSweep(OgrePhysX::Convert::toNx(motion), NX_SF_DYNAMICS|NX_SF_ALL_HITS, 0, maxNumResult, sqh_result, 0, mSweepCache);
		bool obstacleHit = false;
		for (NxU32 i = 0; i < numHits; i++)
		{
			NxSweepQueryHit hit = sqh_result[i];
			if (hit.hitShape->getGroup() == DEFAULT || hit.hitShape->getGroup() == CHARACTER)
			{
				return &hit.hitShape->getActor();
			}
		}
		delete sqh_result;

		return 0;
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

		computePath();
	}

	void FollowPathway::computePath()
	{
		Ogre::Vector3 pos = mAIObject->GetOwner()->GetGlobalPosition();
		mPath.clear();
		AIManager::Instance().FindPath(pos, mTargetWP, mPath);

		if (!mPath.empty())
		{
			mDirectionBlender.StartBlend(mAIObject->GetOwner()->GetGlobalOrientation() * Ogre::Vector3::UNIT_Z, mPath[0]->GetGlobalPosition()-pos);
		}
	}

	void FollowPathway::checkPath()
	{
		unsigned int max = mPath.size() < 5 ? mPath.size() : 5;
		for (unsigned int i = 0; i < max; i++)
		{
			if (mPath[i]->IsBlocked())
			{
				computePath();
				return;
			}
		}
	}

	bool FollowPathway::Update(float time)
	{
		checkPath();
		if (mPath.empty())
		{
			mAIObject->BroadcastMovementState(0);
			return true;
		}

		Ogre::Vector3 currPos = mAIObject->GetOwner()->GetGlobalPosition();

		Ogre::Vector3 myDirection = mAIObject->GetOwner()->GetGlobalOrientation() * Ogre::Vector3::UNIT_Z;

		Ogre::Vector3 currPosXZ = currPos;
		currPosXZ.y = 0;
		Ogre::Vector3 targetPosXZ = mPath[0]->GetGlobalPosition();
		targetPosXZ.y = 0;
		float dist = currPosXZ.distance(targetPosXZ);
		//Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(dist));
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

		Ogre::Vector3 direction;// = mPath[0]->GetGlobalPosition()-currPos;
		//direction.normalise();
		if (mDirectionBlender.HasNext())
		{
			direction = mDirectionBlender.Next(time);
		}
		else
		{
			direction = mPath[0]->GetGlobalPosition()-currPos;
			direction.y = 0;
			direction.normalise();
		}

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

		if (!mDirectionBlender.HasNext())
		{
			if (NxActor *obstacle = ObstacleCheck(direction * 2.0f))
			{
				if (!obstacle->isSleeping())
				{
				}
			}
		}
		/*if (ObstacleCheck(direction * 3.0f))
		{
			mAvoidingObstacle = true;
			Ogre::Quaternion q = Ogre::Quaternion(Ogre::Radian(Ogre::Degree(90)), Ogre::Vector3::UNIT_Y);
			mAvoidObstacleVector = q * direction;
			mDirectionBlender.StartBlend(direction, mAvoidObstacleVector);
		}*/

		Ogre::Quaternion quat = Ogre::Vector3::UNIT_Z.getRotationTo(direction);
		mAIObject->GetOwner()->SetGlobalOrientation(quat);
		mAIObject->BroadcastMovementState(movementstate);
		return false;
	}

}