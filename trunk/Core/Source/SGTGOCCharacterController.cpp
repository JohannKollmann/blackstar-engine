
#include "SGTGOCCharacterController.h"

#include "NxBoxController.h"
#include "NxCapsuleController.h"
#include "SGTMessageSystem.h"
#include "SGTMain.h"
#include "SGTGameObject.h"

void SGTCharacterControllerInput::BroadcastMovementState()
{
	SGTObjectMsg *objmsg = new SGTObjectMsg();
	objmsg->mName = "UpdateCharacterMovementState";
	objmsg->mData.AddInt("CharacterMovementState", mCharacterMovementState);
	mOwnerGO->SendInstantMessage("", Ogre::SharedPtr<SGTObjectMsg>(objmsg));
}
void SGTCharacterControllerInput::BroadcastMovementState(int state)
{
	mCharacterMovementState = state;
	BroadcastMovementState();
}


SGTGOCCharacterController::SGTGOCCharacterController(Ogre::Vector3 dimensions)
{
	Create(dimensions);
}

SGTGOCCharacterController::~SGTGOCCharacterController(void)
{
	SGTMain::Instance().GetNxCharacterManager()->releaseController(*mCharacterController);
	SGTMessageSystem::Instance().QuitNewsgroup(this, "UPDATE_PER_FRAME");
}

void SGTGOCCharacterController::Create(Ogre::Vector3 dimensions)
{
	SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");

	mMovementSpeed = 10.0f;
	mDirection = Ogre::Vector3(0,0,0);
	//mDimensions = dimensions;
	NxCapsuleControllerDesc desc;
	desc.position.x		= 0;
	desc.position.y		= 0;
	desc.position.z		= 0;
	desc.radius			= dimensions.x;
	desc.height			= dimensions.y;
	desc.upDirection	= NX_Y;
	//		desc.slopeLimit		= cosf(NxMath::degToRad(45.0f));
	desc.slopeLimit		= 0;
	desc.skinWidth		= 0.2f;
	mStepOffset			= desc.radius * 0.5;
	desc.stepOffset		= mStepOffset;
	bool test = desc.isValid();
	mCharacterController = SGTMain::Instance().GetNxCharacterManager()->createController(SGTMain::Instance().GetNxScene()->getNxScene(), desc);
}

void SGTGOCCharacterController::UpdatePosition(Ogre::Vector3 position)
{
	if (!mOwnerGO->GetTranformingComponents()) mCharacterController->setPosition(NxExtendedVec3(position.x, position.y, position.z));
}
void SGTGOCCharacterController::UpdateOrientation(Ogre::Quaternion orientation)
{
}
void SGTGOCCharacterController::UpdateScale(Ogre::Vector3 scale)
{
}

void SGTGOCCharacterController::ReceiveMessage(SGTMsg &msg)
{
	if (msg.mNewsgroup == "UPDATE_PER_FRAME")
	{
		float time = msg.mData.GetFloat("TIME");
		float jumpDelta = 0.0f;
		if (mJump.mJumping) jumpDelta = mJump.GetHeight(time);
		Ogre::Vector3 dir_rotated = mOwnerGO->GetGlobalOrientation() * (mDirection + Ogre::Vector3(0, jumpDelta, 0));
		Ogre::Vector3 dir = (dir_rotated + Ogre::Vector3(0.0f, -9.81f, 0.0f)) * time;
		NxU32 collisionFlags;
		float minDist = 0.000001f;
		mCharacterController->move(NxVec3(dir.x, dir.y, dir.z), 1<<SGTMain::Instance().GetNxScene()->getShapeGroup("Collidable")->getGroupID(), minDist, collisionFlags);
		SGTObjectMsg *position_response = new SGTObjectMsg;
		NxExtendedVec3 nxPos = mCharacterController->getDebugPosition();
		mOwnerGO->UpdateTransform(Ogre::Vector3(nxPos.x, nxPos.y-2.0f, nxPos.z), mOwnerGO->GetGlobalOrientation());

		if(collisionFlags &  NxControllerFlag::NXCC_COLLISION_DOWN && mJump.mJumping && mJump.GetHeight(0) <= 0.0f)
		{
			mCharacterController->setStepOffset(mStepOffset);
			mJump.StopJump();
			SGTObjectMsg *jump_response = new SGTObjectMsg;
			jump_response->mName = "CharacterJumpEnded";
			mOwnerGO->SendMessage(Ogre::SharedPtr<SGTObjectMsg>(jump_response));
		}
		SGTObjectMsg *collision_response = new SGTObjectMsg;
		collision_response->mName = "CharacterCollisionReport";
		collision_response->mData.AddFloat("collisionFlags", collisionFlags);
		mOwnerGO->SendMessage(Ogre::SharedPtr<SGTObjectMsg>(collision_response));

	}
}

void SGTGOCCharacterController::ReceiveObjectMessage(Ogre::SharedPtr<SGTObjectMsg> msg)
{
	if (msg->mName == "UpdateCharacterMovementState")
	{
		mDirection = Ogre::Vector3(0,0,0);
		int movementFlags = msg->mData.GetInt("CharacterMovementState");
		if (movementFlags & SGTCharacterMovement::FORWARD) mDirection.z += mMovementSpeed;
		if (movementFlags & SGTCharacterMovement::BACKWARD) mDirection.z -= mMovementSpeed;
		if (movementFlags & SGTCharacterMovement::LEFT) mDirection.x += mMovementSpeed;
		if (movementFlags & SGTCharacterMovement::RIGHT) mDirection.x -= mMovementSpeed;

		if (movementFlags & SGTCharacterMovement::JUMP)
		{
			if (!mJump.mJumping)
			{
				mCharacterController->setStepOffset(0.0f);
				mJump.StartJump(1.0f);
			}
		}
	}
}

void SGTGOCCharacterController::SetOwner(SGTGameObject *go)
{
	mOwnerGO = go;
	UpdatePosition(mOwnerGO->GetGlobalPosition());
}

void SGTGOCCharacterController::CreateFromDataMap(SGTDataMap *parameters)
{
	Create(Ogre::Vector3(1,2,1));
}
void SGTGOCCharacterController::GetParameters(SGTDataMap *parameters)
{
}
void SGTGOCCharacterController::GetDefaultParameters(SGTDataMap *parameters)
{
}

void SGTGOCCharacterController::Save(SGTSaveSystem& mgr)
{
}
void SGTGOCCharacterController::Load(SGTLoadSystem& mgr)
{
	Create(Ogre::Vector3(1,2,1));
}