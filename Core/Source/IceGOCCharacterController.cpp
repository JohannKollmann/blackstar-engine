
#include "IceGOCCharacterController.h"

#include "NxBoxController.h"
#include "NxCapsuleController.h"
#include "IceMessageSystem.h"
#include "IceMain.h"
#include "IceGameObject.h"
#include "OgrePhysX.h"

#include "IceGOCPhysics.h"

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
		if (mCharacterController)
		{
			Msg msg;
			msg.type = "ACOTR_ONWAKE";
			msg.rawData = mCharacterController->getActor();
			MessageSystem::Instance().SendInstantMessage(msg);
			OgrePhysX::World::getSingleton().getControllerManager()->releaseController(*mCharacterController);
			mCharacterController = nullptr;
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
		NxBoxControllerDesc desc;
		desc.skinWidth		= 0.1f;
		desc.extents.set(dimensions.x * 0.5 - desc.skinWidth, dimensions.y * 0.5 - desc.skinWidth, dimensions.z * 0.5 - desc.skinWidth);
		desc.upDirection	= NX_Y;
		desc.slopeLimit		= cosf(NxMath::degToRad(45.0f));
		mStepOffset			= 0.8;
		desc.stepOffset		= mStepOffset;
		bool test = desc.isValid();
		mCharacterController = OgrePhysX::World::getSingleton().getControllerManager()->createController(Main::Instance().GetPhysXScene()->getNxScene(), desc);
		mCharacterController->getActor()->getShapes()[0]->setGroup(CollisionGroups::CHARACTER);
	}

	void GOCCharacterController::SetSpeedFactor(float factor)
	{
		mSpeedFactor = factor;
	}

	void GOCCharacterController::UpdatePosition(Ogre::Vector3 position)
	{
		mCharacterController->setPosition(NxExtendedVec3(position.x, position.y + mDimensions.y * 0.5, position.z));
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
			Ogre::Vector3 dir_rotated = mOwnerGO->GetGlobalOrientation() * (mDirection + Ogre::Vector3(0, jumpDelta, 0));
			Ogre::Vector3 dir = (dir_rotated + Ogre::Vector3(0.0f, -9.81f, 0.0f)) * time;
			NxU32 collisionFlags;
			float minDist = 0.005f;
			mCharacterController->move(NxVec3(dir.x, dir.y, dir.z), 1<<CollisionGroups::DEFAULT | 1<<CollisionGroups::LEVELMESH, minDist, collisionFlags, 0.5f);
			if(collisionFlags &  NxControllerFlag::NXCC_COLLISION_DOWN && mJump.mJumping && mJump.GetHeight(0) <= 0.0f)
			{
				mCharacterController->setStepOffset(mStepOffset);
				mJump.StopJump();
				Msg jump_response;
				jump_response.type = "CharacterJumpEnded";
				mOwnerGO->SendInstantMessage(jump_response);
			}
			Msg collision_response;
			collision_response.type = "CharacterCollisionReport";
			collision_response.params.AddInt("collisionFlags", collisionFlags);
			mOwnerGO->SendMessage(collision_response);
		}
		if (msg.type == "END_PHYSICS" && !mFreezed)
		{
			NxExtendedVec3 nxPos = mCharacterController->getFilteredPosition();
			SetOwnerPosition(Ogre::Vector3(nxPos.x, nxPos.y - mDimensions.y * 0.5, nxPos.z));
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
					mCharacterController->setStepOffset(0.0f);
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
		if (mCharacterController)
		{
			mCharacterController->getActor()->userData = mOwnerGO;
			UpdatePosition(mOwnerGO->GetGlobalPosition());
		}
	}

	void GOCCharacterController::Freeze(bool freeze)
	{
		mFreezed = freeze;
		if (mFreezed)
		{
			mCharacterController->getActor()->raiseBodyFlag(NX_BF_DISABLE_GRAVITY);	
			mCharacterController->getActor()->raiseBodyFlag(NX_BF_FROZEN);	
		}
		else
		{
			mCharacterController->getActor()->clearBodyFlag(NX_BF_DISABLE_GRAVITY);	
			mCharacterController->getActor()->clearBodyFlag(NX_BF_FROZEN);	
			mCharacterController->getActor()->wakeUp();
		}
	}

	void GOCCharacterController::SetParameters(DataMap *parameters)
	{
		_clear();
		mDimensions = parameters->GetValue("Dimensions", Ogre::Vector3(1,1,1));
		Create(mDimensions);
		if (mOwnerGO) mCharacterController->getActor()->userData = mOwnerGO;
		mMovementSpeed = parameters->GetFloat("MaxSpeed");
	}
	void GOCCharacterController::GetParameters(DataMap *parameters)
	{
		parameters->AddOgreVec3("Dimensions", mDimensions);
		parameters->AddFloat("MaxSpeed", mMovementSpeed);
	}
	void GOCCharacterController::GetDefaultParameters(DataMap *parameters)
	{
		parameters->AddOgreVec3("Dimensions", Ogre::Vector3(0.5, 1.8, 0.5));
		parameters->AddFloat("MaxSpeed", 2.0f);
	}

	void GOCCharacterController::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("Ogre::Vector3", &mDimensions, "Dimensions");
		mgr.SaveAtom("float", &mMovementSpeed, "MaxSpeed");
	}
	void GOCCharacterController::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("Ogre::Vector3", &mDimensions);
		Create(mDimensions);
		mgr.LoadAtom("float", &mMovementSpeed);		//Load Save: Todo!
	}

};