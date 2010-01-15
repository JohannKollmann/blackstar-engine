
#include "SGTFollowPathway.h"
#include "SGTPathfinder.h"
#include "SGTGOCAI.h"
#include "SGTGOCCharacterController.h"
#include "SGTGameObject.h"
#include "SGTGOCCharacterController.h"
#include "NxController.h"
#include "SGTMain.h"

SGTFollowPathway::SGTFollowPathway(SGTGOCAI *ai, Ogre::String targetWP, float radius)
{
	mAIObject = ai;
	mRadius = radius;
	mTargetWP = targetWP;
	mCurrentTarget = mPath.begin();
	mBlendFactor = 0.0f;
	mTargetBlendYaw = 0.0f;

	//Sweep cache for dynamic obstacle avoiding
	mSweepCache = SGTMain::Instance().GetPhysXScene()->getNxScene()->createSweepCache(); 
};

SGTFollowPathway::~SGTFollowPathway()
{
	mPath.clear();
	SGTMain::Instance().GetPhysXScene()->getNxScene()->releaseSweepCache(mSweepCache);
};

bool SGTFollowPathway::ObstacleCheck(Ogre::Vector3 motion)
{
	if (!mSweepActor) return false;

	int maxNumResult  = 1;
	NxSweepQueryHit *sqh_result = new NxSweepQueryHit[maxNumResult];
	NxU32 numHits = mSweepActor->linearSweep(OgrePhysX::Convert::toNx(motion), NX_SF_DYNAMICS, 0, maxNumResult, sqh_result, 0, mSweepCache);
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

void SGTFollowPathway::OnEnter()
{
	SGTGOCCharacterController *character = (SGTGOCCharacterController*)mAIObject->GetOwner()->GetComponent("Physics", "CharacterController");
	mSweepActor = 0;
	if (character) mSweepActor = character->GetNxController()->getActor();

	Ogre::Vector3 pos = mAIObject->GetOwner()->GetGlobalPosition();
	SGTPathfinder::Instance().FindPath(pos, mTargetWP, &mPath, mSweepActor);
	mCurrentTarget = mPath.begin();

	if (mCurrentTarget != mPath.end())
	{
		StartBlend(mAIObject->GetOwner()->GetGlobalOrientation() * Ogre::Vector3::UNIT_Z, (*mCurrentTarget)-pos);
	}
}

bool SGTFollowPathway::OnUpdate(float time)
{
	if (mCurrentTarget == mPath.end())
	{
		mAIObject->BroadcastMovementState(0);
		return true;
	}

	Ogre::Vector3 currPos = mAIObject->GetOwner()->GetGlobalPosition();

	float dist = currPos.distance(*mCurrentTarget);
	if (dist < mRadius)
	{
		mBlendDirection = (*mCurrentTarget)-currPos;
		mCurrentTarget++;
		if (mCurrentTarget == mPath.end())
		{
			mAIObject->BroadcastMovementState(0);
			return true;
		}
		Ogre::Vector3 targetBlend = (*mCurrentTarget)-currPos;
		StartBlend(mBlendDirection, targetBlend);

	}

	Ogre::Vector3 direction = (*mCurrentTarget)-currPos;
	direction.normalise();

	int movementstate = 0;
	if (ObstacleCheck(direction * 3.0f))
	{
		movementstate = SGTCharacterMovement::RIGHT;
	}
	else
	{
		movementstate = SGTCharacterMovement::FORWARD;
	}

	if (mBlendFactor > 0.0f)
	{
		Ogre::Quaternion q;
		float yaw = mTargetBlendYaw * (1-mBlendFactor);
		direction = Ogre::Quaternion(Ogre::Radian(yaw), Ogre::Vector3::UNIT_Y) * mBlendDirection;//(mBlendDirection * mBlendFactor) + ((q * mBlendDirection) * (1 - mBlendFactor));
		mBlendFactor -= 2 * time;
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

void SGTFollowPathway::StartBlend(Ogre::Vector3 oldDir, Ogre::Vector3 newDir)
{
	oldDir.y = 0;
	oldDir.normalise();
	newDir.y = 0;
	newDir.normalise();
	mBlendFactor = 1.0f;
	mBlendDirection = oldDir;
	if ((1.0f + mBlendDirection.dotProduct(newDir)) < 0.0001f)            // Work around 180 degree quaternion rotation quirk                         
	{
		mTargetBlendYaw = Ogre::Math::PI;
	}
	else
	{
		mTargetBlendYaw = mBlendDirection.getRotationTo(newDir).getYaw().valueRadians();
	}
}