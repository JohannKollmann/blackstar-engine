
#include "IceFollowPathway.h"
#include "IceNavigationMesh.h"
#include "IceGOCAI.h"
#include "IceGOCCharacterController.h"
#include "IceGameObject.h"
#include "IceGOCCharacterController.h"
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
		//mSweepCache = Main::Instance().GetPhysXScene()->getPxScene()->createSweepCache(); 

		JoinNewsgroup(GlobalMessageIDs::PHYSICS_BEGIN);
	};

	FollowPathway::~FollowPathway()
	{
		mPath.clear();
		//Main::Instance().GetPhysXScene()->getNxScene()->releaseSweepCache(mSweepCache);
	};

	PxRigidDynamic* FollowPathway::ObstacleCheck(Ogre::Vector3 motion)
	{
		//TODO: implement

		return nullptr;
	}
	
	void FollowPathway::OnSetActive(bool active)
	{
		if (active)
		{
			GOCCharacterController *character = (GOCCharacterController*)mAIObject->GetOwner()->GetComponent("Physics", "CharacterController");

			computePath();
		}
		else
		{
			mAIObject->BroadcastMovementState(0);
		}
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

	void FollowPathway::verifyPath()
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

	void FollowPathway::optimizePath()
	{
		Ogre::Vector3 currPos = mAIObject->GetOwner()->GetGlobalPosition();

		unsigned int max = mPath.size() < 5 ? mPath.size() : 5;
		auto endErase = mPath.begin();
		bool erase = false;
		for (unsigned int i = 1; i < max; i++)
		{
			if (!AIManager::Instance().GetNavigationMesh()->TestLinearPath(currPos, mPath[i]->GetGlobalPosition()))
				break;
			else
			{
				if (erase) endErase++;
				else erase = true;
			}
		}
		if (erase) mPath.erase(mPath.begin(), endErase);
	}

	void FollowPathway::OnReceiveMessage(Msg &msg)
	{
		if (msg.typeID == GlobalMessageIDs::PHYSICS_BEGIN)
			Update(msg.params.GetValue<float>(0));
	}

	void FollowPathway::Update(float time)
	{
		verifyPath();
		if (mPath.empty())
		{
			mAIObject->BroadcastMovementState(0);
			TerminateProcess();
			return;
		}

		Ogre::Vector3 currPos = mAIObject->GetOwner()->GetGlobalPosition();

		Ogre::Vector3 myDirection = mAIObject->GetOwner()->GetGlobalOrientation() * Ogre::Vector3::UNIT_Z;

		Ogre::Vector3 currPosXZ = currPos;
		currPosXZ.y = 0;
		Ogre::Vector3 targetPosXZ = mPath[0]->GetGlobalPosition();
		targetPosXZ.y = 0;
		float dist = currPosXZ.distance(targetPosXZ);
		//Log::Instance().LogMessage(Ogre::StringConverter::toString(dist));
		if (dist < mRadius)
		{
			Ogre::Vector3 oldDir = mPath[0]->GetGlobalPosition()-currPos;
			mPath.erase(mPath.begin());
			if (mPath.empty())
			{
				mAIObject->BroadcastMovementState(0);
				TerminateProcess();
				return;
			}
			//optimizePath();
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
		mAIObject->GetOwner()->GetComponent<GOCCharacterController>()->SetSpeedFactor(1);

		if (PxRigidDynamic *obstacle = ObstacleCheck(direction * 4.0f))
		{
			GameObject *go = (GameObject*)obstacle->userData;
			dist = go->GetGlobalPosition().distance(currPos);

			mAIObject->GetOwner()->GetComponent<GOCCharacterController>()->SetSpeedFactor(dist < 3 ? dist / 3 : 1);
				/*
				Bei < 4 Metern Entfernung:
					- Versuchen nach rechts auszuweichen, AStar von dort neu anwenden
					- Wenn das nicht geht geradeaus weiterlaufen, Speed drosseln
				Bei < 2 Metern Entfernung:
					- Wenn Hindernis Npc ist: Speed stark drosseln und soweit rechts steuern, wie möglich.
					- kein Npc: AStar, vorher blocker temporär setzen
				*/
				/*if (go->GetGlobalPosition().distance(currPos) < 2)
				{
					if (GOCCharacterController *character = go->GetComponent<GOCCharacterController>())
					{
					}
					else
					{
					}
				}
				else*/
			Ogre::Quaternion q = Ogre::Vector3::UNIT_X.getRotationTo(Ogre::Vector3::UNIT_Z);
			Ogre::Vector3 ortDir = q * myDirection;
			Ogre::Vector3 target = currPos + (myDirection * dist) + (ortDir * Ogre::Vector3(1.5f, 0, 1.5f));
			Ogre::Vector3 avoidDir = (target - currPos).normalisedCopy();
			Ogre::Vector3 velocity = OgrePhysX::Convert::toOgre(obstacle->getLinearVelocity());
			if (velocity.normalisedCopy().dotProduct(myDirection) < 0 || velocity.length() < 0.5f)
			{
				if (AIManager::Instance().GetNavigationMesh()->TestLinearPath(currPos, target))
				{
					mTempObstacleAvoidNode.SetGlobalPosition(target);
					mPath.erase(mPath.begin());
					mPath.insert(mPath.begin(), &mTempObstacleAvoidNode);
					Ogre::Vector3 targetDir = mPath[0]->GetGlobalPosition()-currPos;
					mDirectionBlender.StartBlend(direction, targetDir, 0.2f);
				}
			}
		}	

		Ogre::Quaternion quat = Ogre::Vector3::UNIT_Z.getRotationTo(direction);
		mAIObject->GetOwner()->SetGlobalOrientation(quat);
		mAIObject->BroadcastMovementState(movementstate);
	}

}