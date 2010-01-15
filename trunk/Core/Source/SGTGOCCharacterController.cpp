
#include "SGTGOCCharacterController.h"

#include "NxBoxController.h"
#include "NxCapsuleController.h"
#include "SGTMessageSystem.h"
#include "SGTMain.h"
#include "SGTGameObject.h"
#include "OgrePhysX.h"

#include "SGTGOCPhysics.h"

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
	OgrePhysX::World::getSingleton().getControllerManager()->releaseController(*mCharacterController);
	SGTMessageSystem::Instance().QuitNewsgroup(this, "START_PHYSICS");
	SGTMessageSystem::Instance().QuitNewsgroup(this, "END_PHYSICS");
}

void SGTGOCCharacterController::Create(Ogre::Vector3 dimensions)
{
	SGTMessageSystem::Instance().JoinNewsgroup(this, "START_PHYSICS");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "END_PHYSICS");

	if (dimensions.x == 0 || dimensions.y == 0 || dimensions.z == 0) dimensions = Ogre::Vector3(1,1,1);

	mFreezed = false;

	mMovementSpeed = 4.0f;
	mDirection = Ogre::Vector3(0,0,0);
	mDimensions = dimensions;
	NxBoxControllerDesc desc;
	desc.skinWidth		= 0.1f;
	desc.extents.set(dimensions.x * 0.5 - desc.skinWidth, dimensions.y * 0.5 - desc.skinWidth, dimensions.z * 0.5 - desc.skinWidth);
	desc.upDirection	= NX_Y;
	//		desc.slopeLimit		= cosf(NxMath::degToRad(45.0f));
	desc.slopeLimit		= 0;
	mStepOffset			= dimensions.z * 0.5;
	desc.stepOffset		= mStepOffset;
	bool test = desc.isValid();
	mCharacterController = OgrePhysX::World::getSingleton().getControllerManager()->createController(SGTMain::Instance().GetPhysXScene()->getNxScene(), desc);
	mCharacterController->getActor()->getShapes()[0]->setGroup(SGTCollisionGroups::CHARACTER);
}

void SGTGOCCharacterController::UpdatePosition(Ogre::Vector3 position)
{
	if (!mOwnerGO->GetTranformingComponents()) mCharacterController->setPosition(NxExtendedVec3(position.x, position.y + mDimensions.y * 0.5, position.z));
}
void SGTGOCCharacterController::UpdateOrientation(Ogre::Quaternion orientation)
{
}
void SGTGOCCharacterController::UpdateScale(Ogre::Vector3 scale)
{
}

void SGTGOCCharacterController::ReceiveMessage(SGTMsg &msg)
{
	if (msg.mNewsgroup == "START_PHYSICS" && !mFreezed)
	{
		float time = msg.mData.GetFloat("TIME");
		float jumpDelta = 0.0f;
		if (mJump.mJumping) jumpDelta = mJump.GetHeight(time);
		Ogre::Vector3 dir_rotated = mOwnerGO->GetGlobalOrientation() * (mDirection + Ogre::Vector3(0, jumpDelta, 0));
		Ogre::Vector3 dir = (dir_rotated + Ogre::Vector3(0.0f, -9.81f, 0.0f)) * time;
		NxU32 collisionFlags;
		float minDist = 0.000001f;
		mCharacterController->move(NxVec3(dir.x, dir.y, dir.z), 1<<SGTCollisionGroups::DEFAULT | 1<<SGTCollisionGroups::LEVELMESH, minDist, collisionFlags);
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
	if (msg.mNewsgroup == "END_PHYSICS" && !mFreezed)
	{
		NxExtendedVec3 nxPos = mCharacterController->getFilteredPosition();
		mOwnerGO->UpdateTransform(Ogre::Vector3(nxPos.x, nxPos.y - mDimensions.y * 0.5, nxPos.z), mOwnerGO->GetGlobalOrientation());
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
	if (msg->mName == "KillCharacter")
	{
		mFreezed = true;
	}
}

void SGTGOCCharacterController::SetOwner(SGTGameObject *go)
{
	mOwnerGO = go;
	UpdatePosition(mOwnerGO->GetGlobalPosition());
}

void SGTGOCCharacterController::Freeze(bool freeze)
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

void SGTGOCCharacterController::CreateFromDataMap(SGTDataMap *parameters)
{
	try {
		mDimensions = parameters->GetOgreVec3("Dimensions");
	} catch (Ogre::Exception) { mDimensions = Ogre::Vector3(1,1,1); }
	Create(mDimensions);
}
void SGTGOCCharacterController::GetParameters(SGTDataMap *parameters)
{
	parameters->AddOgreVec3("Dimensions", mDimensions);
}
void SGTGOCCharacterController::GetDefaultParameters(SGTDataMap *parameters)
{
	parameters->AddOgreVec3("Dimensions", Ogre::Vector3(0.5, 1.8, 0.5));
}

void SGTGOCCharacterController::Save(SGTSaveSystem& mgr)
{
	mgr.SaveAtom("Ogre::Vector3", &mDimensions, "Dimensions");
}
void SGTGOCCharacterController::Load(SGTLoadSystem& mgr)
{
	mgr.LoadAtom("Ogre::Vector3", &mDimensions);
	Create(mDimensions);
}

void SGTGOCCharacterController::AttachToGO(SGTGameObject *go)
{
	go->RemoveComponent(GetFamilyID());
	go->AddComponent(this);
}