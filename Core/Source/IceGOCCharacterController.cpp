
#include "IceGOCCharacterController.h"

#include "NxBoxController.h"
#include "NxCapsuleController.h"
#include "IceMessageSystem.h"
#include "IceMain.h"
#include "IceGameObject.h"
#include "OgrePhysX.h"
#include "IceGOCPhysics.h"

#include "IceSceneManager.h"

namespace Ice
{

	void CharacterControllerInput::BroadcastMovementState()
	{
		Msg objmsg;
		objmsg.type = "UpdateCharacterMovementState";
		objmsg.params.AddInt("CharacterMovementState", mCharacterMovementState);
		mOwnerGO->SendInstantMessage(objmsg);
	}
	void CharacterControllerInput::BroadcastMovementState(int state)
	{
		mCharacterMovementState = state;
		BroadcastMovementState();
	}


	GOCCharacterController::GOCCharacterController(Ogre::Vector3 dimensions)
	{
		Create(dimensions);
	}

	GOCCharacterController::~GOCCharacterController(void)
	{
		_clear();
	}

	void GOCCharacterController::_clear()
	{
		if (mActor)
		{
			Msg msg;
			msg.type = "ACOTR_ONWAKE";
			msg.rawData = mActor;
			MessageSystem::Instance().SendInstantMessage(msg);
			Main::Instance().GetPhysXScene()->destroyActor(mActor);
			mActor = nullptr;
			Main::Instance().GetPhysXScene()->destroyActor(mSweepActor);
			mSweepActor = nullptr;
			Main::Instance().GetPhysXScene()->getNxScene()->releaseSweepCache(mSweepCache);
			mSweepCache = nullptr;
		}
	}

	void GOCCharacterController::Create(Ogre::Vector3 dimensions)
	{
		MessageSystem::Instance().QuitAllNewsgroups(this);
		MessageSystem::Instance().JoinNewsgroup(this, "START_PHYSICS");
		MessageSystem::Instance().JoinNewsgroup(this, "END_PHYSICS");

		if (dimensions.x == 0 || dimensions.y == 0 || dimensions.z == 0) dimensions = Ogre::Vector3(1,1,1);

		mFreezed = false;

		mMovementSpeed = 2.0f;
		mSpeedFactor = 1;
		mDirection = Ogre::Vector3(0,0,0);
		mDimensions = dimensions;
		NxMaterialIndex nxID = SceneManager::Instance().GetSoundMaterialTable().GetMaterialID(mMaterialName);

		mRadius = mDimensions.x > mDimensions.z ? mDimensions.x : mDimensions.z;
		float offset = 0.0f;
		if (mDimensions.y - mRadius > 0.0f) offset = (mDimensions.y / mRadius) * 0.1f;
		mHeight = mDimensions.y * 0.5f + offset;
		mRadius *= 0.5f;
		mActor = Main::Instance().GetPhysXScene()->createActor(
			OgrePhysX::CapsuleShape(capsule_radius * 0.5f, mDimensions.y * 0.5f + offset).density(10).group(CollisionGroups::CHARACTER).localPose(Ogre::Vector3(0, mDimensions.y * 0.5f, 0)).material(nxID));
		//mActor->getNxActor()->raiseBodyFlag(NxBodyFlag::NX_BF_DISABLE_GRAVITY);
		mActor->getNxActor()->setMassSpaceInertiaTensor(NxVec3(0,1,0));

		float maxStepHeight = 0.3f;
		mSweepActor = Main::Instance().GetPhysXScene()->createActor(
			OgrePhysX::CapsuleShape(mRadius, mDimensions.y * 0.5f + offset - maxStepHeight).group(CollisionGroups::AI).localPose(Ogre::Vector3(0, (mDimensions.y * 0.5f) + maxStepHeight, 0)));
		mSweepActor->getNxActor()->raiseBodyFlag(NxBodyFlag::NX_BF_KINEMATIC);
		mSweepActor->getNxActor()->setMassSpaceInertiaTensor(NxVec3(0,1,0));
		mSweepCache = Main::Instance().GetPhysXScene()->getNxScene()->createSweepCache();

	}

	void GOCCharacterController::SetSpeedFactor(float factor)
	{
		mSpeedFactor = factor;
	}

	void GOCCharacterController::UpdatePosition(Ogre::Vector3 position)
	{
		mActor->setGlobalPosition(position);//Ogre::Vector3(position.x, position.y + mDimensions.y * 0.5, position.z));
		mSweepActor->setGlobalPosition(position);
	}
	void GOCCharacterController::UpdateOrientation(Ogre::Quaternion orientation)
	{
	}
	void GOCCharacterController::UpdateScale(Ogre::Vector3 scale)
	{
	}

	void GOCCharacterController::ReceiveMessage(Msg &msg)
	{
		if (msg.type == "START_PHYSICS" && !mFreezed)
		{
			float time = msg.params.GetFloat("TIME");
			float jumpDelta = 0.0f;
			if (mJump.mJumping) jumpDelta = mJump.GetHeight(time);
			Ogre::Vector3 dir = mOwnerGO->GetGlobalOrientation() * (mDirection + Ogre::Vector3(0, jumpDelta, 0));

			float maxStepHeight = 0.6f;
			NxVec3 currPos = OgrePhysX::Convert::toNx(mOwnerGO->GetGlobalPosition());
			//feet capsule
			NxBox feetBox;
			feetBox.center = currPos + NxVec3(0, maxStepHeight*0.5f - mRadius + 0.2f, 0);
			feetBox.extents = NxVec3(mRadius, maxStepHeight*0.5f, mRadius);
			feetBox.rot.fromQuat(OgrePhysX::Convert::toNx(Ogre::Quaternion()));

			//body capsule
			NxBox bodyBox;
			float bodyHeight = mDimensions.y-maxStepHeight;
			bodyBox.center = currPos + NxVec3(0, maxStepHeight+(bodyHeight*0.5f), 0);
			bodyBox.extents = NxVec3(mRadius, bodyHeight*0.5f, mRadius);
			bodyBox.rot.fromQuat(OgrePhysX::Convert::toNx(Ogre::Quaternion()));
			NxSweepQueryHit sqh_result[1];

			NxU32 numHits = Main::Instance().GetPhysXScene()->getNxScene()->linearOBBSweep(bodyBox, OgrePhysX::Convert::toNx(Ogre::Vector3(dir*time)), NX_SF_STATICS, 0, 1, sqh_result, 0);//1<<CollisionGroups::DEFAULT | 1<<CollisionGroups::LEVELMESH, mSweepCache);
 			bool bodyHit = (numHits > 0);
			Ogre::LogManager::getSingleton().logMessage("Body hit: " + Ogre::StringConverter::toString(bodyHit));
			numHits = Main::Instance().GetPhysXScene()->getNxScene()->linearOBBSweep(feetBox, OgrePhysX::Convert::toNx(Ogre::Vector3(dir*time)), NX_SF_STATICS, 0, 1, sqh_result, 0);//, mSweepCache);
 			bool feetHit = (numHits > 0);
			Ogre::LogManager::getSingleton().logMessage("Feet hit: " + Ogre::StringConverter::toString(feetHit));

			if (!bodyHit && !feetHit)
			{
				Freeze(true);
				mActor->setGlobalPosition(mActor->getGlobalPosition() + dir*time);
				Freeze(false);
			}
			else if (!bodyHit && feetHit)
			{
				Freeze(true);
				dir+=Ogre::Vector3(0,1,0);
				mActor->setGlobalPosition(mActor->getGlobalPosition() + dir*time);
				Freeze(false);
				float forceFactor = 1 - NxVec3(0,1,0).dot(sqh_result[0].normal);
				if (forceFactor > 0.9)
					mActor->getNxActor()->addForce(NxVec3(0, 500*forceFactor,0));
			}

			if(mJump.mJumping && mJump.GetHeight(0) <= 0.0f)
			{
				mJump.StopJump();
				Msg jump_response;
				jump_response.type = "CharacterJumpEnded";
				mOwnerGO->SendInstantMessage(jump_response);
			}
			/*Msg collision_response;
			collision_response.type = "CharacterCollisionReport";
			collision_response.params.AddInt("collisionFlags", collisionFlags);
			mOwnerGO->SendMessage(collision_response);*/
		}
		if (msg.type == "END_PHYSICS" && !mFreezed)
		{
			Ogre::Vector3 pos = mActor->getGlobalPosition();
			SetOwnerPosition(Ogre::Vector3(pos.x, pos.y, pos.z));
		}
	}

	void GOCCharacterController::ReceiveObjectMessage(Msg &msg)
	{
		if (msg.type == "UpdateCharacterMovementState")
		{
			mDirection = Ogre::Vector3(0,0,0);
			int movementFlags = msg.params.GetInt("CharacterMovementState");
			if (movementFlags & CharacterMovement::FORWARD) mDirection.z += (mMovementSpeed*mSpeedFactor);
			if (movementFlags & CharacterMovement::BACKWARD) mDirection.z -= (mMovementSpeed*mSpeedFactor);
			if (movementFlags & CharacterMovement::LEFT) mDirection.x += (mMovementSpeed*mSpeedFactor);
			if (movementFlags & CharacterMovement::RIGHT) mDirection.x -= (mMovementSpeed*mSpeedFactor);

			if (movementFlags & CharacterMovement::JUMP)
			{
				if (!mJump.mJumping)
				{
					mJump.StartJump(1.0f);
				}
			}
		}
		if (msg.type == "KillCharacter")
		{
			mFreezed = true;
		}
	}

	void GOCCharacterController::SetOwner(GameObject *go)
	{
		mOwnerGO = go;
		if (mActor)
		{
			mActor->getNxActor()->userData = mOwnerGO;
			UpdatePosition(mOwnerGO->GetGlobalPosition());
		}
	}

	void GOCCharacterController::Freeze(bool freeze)
	{
		mFreezed = freeze;
		if (mFreezed)
		{
			mActor->getNxActor()->raiseBodyFlag(NX_BF_FROZEN);	
		}
		else
		{
			mActor->getNxActor()->clearBodyFlag(NX_BF_FROZEN);	
			mActor->getNxActor()->wakeUp();
		}
	}

	void GOCCharacterController::SetParameters(DataMap *parameters)
	{
		_clear();
		mDimensions = parameters->GetValue("Dimensions", Ogre::Vector3(1,1,1));
		Create(mDimensions);
		if (mOwnerGO) mActor->getNxActor()->userData = mOwnerGO;
		mMovementSpeed = parameters->GetFloat("MaxSpeed");
		mMaterialName = parameters->GetValue<Ogre::String>("mMaterialName", "Wood");
	}
	void GOCCharacterController::GetParameters(DataMap *parameters)
	{
		parameters->AddOgreVec3("Dimensions", mDimensions);
		parameters->AddFloat("MaxSpeed", mMovementSpeed);
		parameters->AddOgreString("mMaterialName", mMaterialName);
	}
	void GOCCharacterController::GetDefaultParameters(DataMap *parameters)
	{
		parameters->AddOgreVec3("Dimensions", Ogre::Vector3(0.5, 1.8, 0.5));
		parameters->AddFloat("MaxSpeed", 2.0f);
		parameters->AddOgreString("mMaterialName", "Wood");
	}

	void GOCCharacterController::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("Ogre::Vector3", &mDimensions, "Dimensions");
		mgr.SaveAtom("float", &mMovementSpeed, "MaxSpeed");
		mgr.SaveAtom("Ogre::String", &mMaterialName, "mMaterialName");
	}
	void GOCCharacterController::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("Ogre::Vector3", &mDimensions);
		Create(mDimensions);
		mgr.LoadAtom("float", &mMovementSpeed);		//Load Save: Todo!
		mgr.LoadAtom("Ogre::String", &mMaterialName);
	}

};