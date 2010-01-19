
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
	ObjectMsg *objmsg = new ObjectMsg();
	objmsg->mName = "UpdateCharacterMovementState";
	objmsg->mData.AddInt("CharacterMovementState", mCharacterMovementState);
	mOwnerGO->SendInstantMessage("", Ogre::SharedPtr<ObjectMsg>(objmsg));
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
	OgrePhysX::World::getSingleton().getControllerManager()->releaseController(*mCharacterController);
	MessageSystem::Instance().QuitNewsgroup(this, "START_PHYSICS");
	MessageSystem::Instance().QuitNewsgroup(this, "END_PHYSICS");
}

void GOCCharacterController::Create(Ogre::Vector3 dimensions)
{
	MessageSystem::Instance().JoinNewsgroup(this, "START_PHYSICS");
	MessageSystem::Instance().JoinNewsgroup(this, "END_PHYSICS");

	if (dimensions.x == 0 || dimensions.y == 0 || dimensions.z == 0) dimensions = Ogre::Vector3(1,1,1);

	mFreezed = false;

	mMovementSpeed = 2.0f;
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
	mCharacterController = OgrePhysX::World::getSingleton().getControllerManager()->createController(Main::Instance().GetPhysXScene()->getNxScene(), desc);
	mCharacterController->getActor()->getShapes()[0]->setGroup(CollisionGroups::CHARACTER);
}

void GOCCharacterController::UpdatePosition(Ogre::Vector3 position)
{
	if (!mOwnerGO->GetTranformingComponents()) mCharacterController->setPosition(NxExtendedVec3(position.x, position.y + mDimensions.y * 0.5, position.z));
}
void GOCCharacterController::UpdateOrientation(Ogre::Quaternion orientation)
{
}
void GOCCharacterController::UpdateScale(Ogre::Vector3 scale)
{
}

void GOCCharacterController::ReceiveMessage(Msg &msg)
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
		mCharacterController->move(NxVec3(dir.x, dir.y, dir.z), 1<<CollisionGroups::DEFAULT | 1<<CollisionGroups::LEVELMESH, minDist, collisionFlags);
		if(collisionFlags &  NxControllerFlag::NXCC_COLLISION_DOWN && mJump.mJumping && mJump.GetHeight(0) <= 0.0f)
		{
			mCharacterController->setStepOffset(mStepOffset);
			mJump.StopJump();
			ObjectMsg *jump_response = new ObjectMsg;
			jump_response->mName = "CharacterJumpEnded";
			mOwnerGO->SendMessage(Ogre::SharedPtr<ObjectMsg>(jump_response));
		}
		ObjectMsg *collision_response = new ObjectMsg;
		collision_response->mName = "CharacterCollisionReport";
		collision_response->mData.AddFloat("collisionFlags", collisionFlags);
		mOwnerGO->SendMessage(Ogre::SharedPtr<ObjectMsg>(collision_response));
	}
	if (msg.mNewsgroup == "END_PHYSICS" && !mFreezed)
	{
		NxExtendedVec3 nxPos = mCharacterController->getFilteredPosition();
		mOwnerGO->UpdateTransform(Ogre::Vector3(nxPos.x, nxPos.y - mDimensions.y * 0.5, nxPos.z), mOwnerGO->GetGlobalOrientation());
	}
}

void GOCCharacterController::ReceiveObjectMessage(Ogre::SharedPtr<ObjectMsg> msg)
{
	if (msg->mName == "UpdateCharacterMovementState")
	{
		mDirection = Ogre::Vector3(0,0,0);
		int movementFlags = msg->mData.GetInt("CharacterMovementState");
		if (movementFlags & CharacterMovement::FORWARD) mDirection.z += mMovementSpeed;
		if (movementFlags & CharacterMovement::BACKWARD) mDirection.z -= mMovementSpeed;
		if (movementFlags & CharacterMovement::LEFT) mDirection.x += mMovementSpeed;
		if (movementFlags & CharacterMovement::RIGHT) mDirection.x -= mMovementSpeed;

		if (movementFlags & CharacterMovement::JUMP)
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

void GOCCharacterController::SetOwner(GameObject *go)
{
	mOwnerGO = go;
	mCharacterController->getActor()->userData = mOwnerGO;
	UpdatePosition(mOwnerGO->GetGlobalPosition());
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

void GOCCharacterController::CreateFromDataMap(DataMap *parameters)
{
	try {
		mDimensions = parameters->GetOgreVec3("Dimensions");
	} catch (Ogre::Exception) { mDimensions = Ogre::Vector3(1,1,1); }
	Create(mDimensions);
}
void GOCCharacterController::GetParameters(DataMap *parameters)
{
	parameters->AddOgreVec3("Dimensions", mDimensions);
}
void GOCCharacterController::GetDefaultParameters(DataMap *parameters)
{
	parameters->AddOgreVec3("Dimensions", Ogre::Vector3(0.5, 1.8, 0.5));
}

void GOCCharacterController::Save(LoadSave::SaveSystem& mgr)
{
	mgr.SaveAtom("Ogre::Vector3", &mDimensions, "Dimensions");
}
void GOCCharacterController::Load(LoadSave::LoadSystem& mgr)
{
	mgr.LoadAtom("Ogre::Vector3", &mDimensions);
	Create(mDimensions);
}

void GOCCharacterController::AttachToGO(GameObject *go)
{
	go->RemoveComponent(GetFamilyID());
	go->AddComponent(this);
}

};