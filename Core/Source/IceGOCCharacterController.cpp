
#include "IceGOCCharacterController.h"

#include "IceMessageSystem.h"
#include "IceObjectMessageIDs.h"
#include "IceMain.h"
#include "IceGameObject.h"
#include "OgrePhysX.h"
#include "IceGOCPhysics.h"
#include "mmsystem.h"

#include "IceSceneManager.h"

namespace Ice
{

	void CharacterControllerInput::BroadcastMovementState()
	{
		Msg msg;
		msg.typeID = ObjectMessageIDs::UPDATE_CHARACTER_MOVEMENTSTATE;
		msg.params.AddInt("CharacterMovementState", mCharacterMovementState);
		BroadcastObjectMessage(msg);
	}

#define BROADCAST_MOVEMENTCHANGE(movementType, newState) \
{ \
	int dif = mCharacterMovementState ^ newState; \
	if (dif & movementType) \
	{ \
		Msg msg; \
		msg.typeID = ((newState & movementType) ? ObjectMessageIDs::ENTER_MOVEMENT_STATE : ObjectMessageIDs::LEAVE_MOVEMENT_STATE); \
		msg.params.AddOgreString("state", #movementType); \
		BroadcastObjectMessage(msg); \
	} \
} \

	void CharacterControllerInput::BroadcastMovementState(int state)
	{
		if (mCharacterMovementState != state)
		{
			BROADCAST_MOVEMENTCHANGE(FORWARD, state)
			BROADCAST_MOVEMENTCHANGE(BACKWARD, state)
			BROADCAST_MOVEMENTCHANGE(LEFT, state)
			BROADCAST_MOVEMENTCHANGE(RIGHT, state)
			BROADCAST_MOVEMENTCHANGE(RUN, state)
			BROADCAST_MOVEMENTCHANGE(CROUCH, state)
		}
		mCharacterMovementState = state;
		BroadcastMovementState();
	}


	GOCCharacterController::GOCCharacterController(Ogre::Vector3 dimensions)
	{
		mMovementSpeed = 2.0f;
		mActor = nullptr;
		Create(dimensions);
		mDensity = 10;
	}

	GOCCharacterController::~GOCCharacterController(void)
	{
		_clear();
	}

	void GOCCharacterController::_clear()
	{
		if (mActor.getPxActor())
		{
			Msg msg;
			msg.typeID = GlobalMessageIDs::ACTOR_ONWAKE;
			msg.rawData = mActor.getPxActor();
			MulticastMessage(msg);
			Main::Instance().GetPhysXScene()->getPxScene()->removeActor(*mActor.getPxActor());
			mActor.setPxActor(nullptr);

			//TODO: destroy sweep cache
			mSweepCache = nullptr;
		}
	}

	void GOCCharacterController::Create(Ogre::Vector3 dimensions)
	{
		QuitAllNewsgroups();
		JoinNewsgroup(GlobalMessageIDs::PHYSICS_SUBSTEP);
		JoinNewsgroup(GlobalMessageIDs::PHYSICS_END);

		if (dimensions.x == 0 || dimensions.y == 0 || dimensions.z == 0) dimensions = Ogre::Vector3(1,1,1);

		mFreezed = false;
		mJump.mJumping = false;

		mSpeedFactor = 1;
		mDirection = Ogre::Vector3(0,0,0);
		mDimensions = dimensions;
		//PxMaterialIndex nxID = SceneManager::Instance().GetSoundMaterialTable().GetMaterialID(mMaterialName);

		mRadius = mDimensions.x > mDimensions.z ? mDimensions.x : mDimensions.z;
		float offset = 0.0f;
		if (mDimensions.y - mRadius > 0.0f) offset = (mDimensions.y / mRadius) * 0.1f;
		mHeight = mDimensions.y * 0.5f + offset;
		mRadius *= 0.5f;

		PxMaterial *mat = &OgrePhysX::World::getSingleton().getDefaultMaterial();

		mActor.setPxActor(OgrePhysX::getPxPhysics()->createRigidDynamic(PxTransform()));
		
		mActor.getPxActor()->createShape(PxCapsuleGeometry(mRadius, mHeight), *mat, PxTransform(PxVec3(0, mDimensions.y * 0.5f, 0)));

		mActor.getPxActor()->setMassSpaceInertiaTensor(PxVec3(0,1,0));
		mActor.getPxActor()->setSolverIterationCounts(8);

		mSweepCache = Main::Instance().GetPhysXScene()->getPxScene()->createSweepCache();

	}

	void GOCCharacterController::SetSpeedFactor(float factor)
	{
		mSpeedFactor = factor;
	}

	void GOCCharacterController::UpdatePosition(const Ogre::Vector3 &position)
	{
		mActor.setGlobalPosition(position);//Ogre::Vector3(position.x, position.y + mDimensions.y * 0.5, position.z));
	}
	void GOCCharacterController::UpdateOrientation(const Ogre::Quaternion &orientation)
	{
		mActor.setGlobalOrientation(orientation);
	}
	void GOCCharacterController::UpdateScale(const Ogre::Vector3 &scale)
	{
	}

	void GOCCharacterController::ReceiveMessage(Msg &msg)
	{
		GOComponent::ReceiveMessage(msg);
		if (msg.typeID == GlobalMessageIDs::PHYSICS_SUBSTEP && !mFreezed)
		{
			float time = msg.params.GetFloat("TIME");
			//if (mJump.mJumping) jumpDelta = mJump.GetHeight(time);
			Ogre::Vector3 finalDir = Ogre::Vector3(0,0,0);
			Ogre::Vector3 userDir = mOwnerGO.lock()->GetGlobalOrientation() * (mDirection);

			float maxStepHeight = 0.8f;
			PxVec3 currPos = OgrePhysX::Convert::toPx(mOwnerGO.lock()->GetGlobalPosition());

			//feet capsule
			PxCapsuleGeometry feetVolume(mRadius, maxStepHeight);

			//body capsule
			float bodyHeight = mDimensions.y-maxStepHeight;
			PxCapsuleGeometry bodyVolume(mRadius, bodyHeight);

			/*Main::Instance().GetPhysXScene()->getPxScene()->sweepSingle(bodyVolume, OgrePhysX::Convert::toNx(Ogre::Vector3(userDir*time*2)), NX_SF_STATICS|NX_SF_DYNAMICS, 0, 1, sqh_result, nullptr, 1<<CollisionGroups::DEFAULT | 1<<CollisionGroups::LEVELMESH);
			bool bodyHit = (numHits > 0);
			//Log::Instance().LogMessage("Body hit: " + Ogre::StringConverter::toString(bodyHit));
			numHits = Main::Instance().GetPhysXScene()->getNxScene()->linearCapsuleSweep(feetVolume, OgrePhysX::Convert::toNx(Ogre::Vector3(userDir*time*2)), NX_SF_STATICS|NX_SF_DYNAMICS, 0, 1, sqh_result, nullptr, 1<<CollisionGroups::DEFAULT | 1<<CollisionGroups::LEVELMESH);//, mSweepCache);
			bool feetHit = (numHits > 0);
			//Log::Instance().LogMessage("Feet hit: " + Ogre::StringConverter::toString(feetHit));

			NxCapsule playerCapsule;
			playerCapsule.radius = mRadius;
			playerCapsule.p0 = currPos;
			playerCapsule.p1 = currPos + NxVec3(0, mHeight, 0);
			mTouchesGround = Main::Instance().GetPhysXScene()->getNxScene()->checkOverlapCapsule(playerCapsule, NX_ALL_SHAPES, 1<<CollisionGroups::DEFAULT | 1<<CollisionGroups::LEVELMESH);
			

			if (!bodyHit)
			{
				finalDir += userDir;	//add player movement
			}
			if (!bodyHit && feetHit)
			{
				finalDir += Ogre::Vector3(0,3,0); //climb stairs
			}*/

			if (finalDir != Ogre::Vector3(0,0,0))
				mActor.getPxActor()->setGlobalPose(PxTransform(currPos + OgrePhysX::Convert::toPx(finalDir*time)));

			//Log::Instance().LogMessage(Ogre::StringConverter::toString(mTouchesGround));
			if (mJumping && mTouchesGround && (timeGetTime() - mJumpStartTime > 400))
			{
				mJumping = false;
				Msg jump_response;
				jump_response.typeID = ObjectMessageIDs::END_JUMP;
				BroadcastObjectMessage(jump_response);
			}
			/*Msg collision_response;
			collision_response.type = "CharacterCollisionReport";
			collision_response.params.AddInt("collisionFlags", collisionFlags);
			mOwnerGO.lock()->SendMessage(collision_response);*/
		}
		if (msg.typeID == GlobalMessageIDs::PHYSICS_END && !mFreezed)
		{
			SetOwnerPosition(mActor.getGlobalPosition());
			//SetOwnerOrientation(mActor->getGlobalOrientation());
		}


		if (msg.typeID == ObjectMessageIDs::UPDATE_CHARACTER_MOVEMENTSTATE)
		{
			mDirection = Ogre::Vector3(0,0,0);
			int movementFlags = msg.params.GetInt("CharacterMovementState");
			if (movementFlags & CharacterMovement::FORWARD) mDirection.z += 1;
			if (movementFlags & CharacterMovement::BACKWARD) mDirection.z -= 1;
			if (movementFlags & CharacterMovement::LEFT) mDirection.x += 1;
			if (movementFlags & CharacterMovement::RIGHT) mDirection.x -= 1;

			mDirection.normalise();
			mDirection*=(mMovementSpeed*mSpeedFactor);
		}
		if (msg.typeID == ObjectMessageIDs::INPUT_START_JUMP)
		{
			if (mTouchesGround && !mJumping)
			{
				mJumping = true;
				mJumpStartTime = timeGetTime();
				mActor.getPxActor()->addForce(PxVec3(0, 400, 0), PxForceMode::eIMPULSE);
				Msg msg;
				msg.typeID = ObjectMessageIDs::START_JUMP;
				BroadcastObjectMessage(msg);
			}
		}
		if (msg.typeID == ObjectMessageIDs::CHARACTER_KILL)
		{
			mFreezed = true;
		}
	}


	void GOCCharacterController::SetOwner(std::weak_ptr<GameObject> go)
	{
		mOwnerGO = go;
		GameObjectPtr owner = mOwnerGO.lock();
		if (!owner.get()) return;
		if (mActor.getPxActor())
		{
			mActor.getPxActor()->userData = owner.get();
			UpdatePosition(owner->GetGlobalPosition());
		}
	}

	void GOCCharacterController::Freeze(bool freeze)
	{
		mFreezed = freeze;
		if (!mActor.getPxActor()) return;
		mActor.getPxActor()->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, freeze);
	}

	void GOCCharacterController::SetParameters(DataMap *parameters)
	{
		_clear();
		mDimensions = parameters->GetValue("Dimensions", Ogre::Vector3(1,1,1));
		mMovementSpeed = parameters->GetFloat("MaxSpeed");
		mMaterialName = parameters->GetValue<Ogre::String>("MaterialName", "Wood");
		mDensity = parameters->GetValue<float>("Density", 10);
		Create(mDimensions);
		GameObjectPtr owner = mOwnerGO.lock();
		if (owner.get()) mActor.getPxActor()->userData = owner.get();
	}
	void GOCCharacterController::GetParameters(DataMap *parameters)
	{
		parameters->AddOgreVec3("Dimensions", mDimensions);
		parameters->AddFloat("MaxSpeed", mMovementSpeed);
		parameters->AddOgreString("MaterialName", mMaterialName);
		parameters->AddFloat("Density", mDensity);
	}
	void GOCCharacterController::GetDefaultParameters(DataMap *parameters)
	{
		parameters->AddOgreVec3("Dimensions", Ogre::Vector3(0.5, 1.8, 0.5));
		parameters->AddFloat("MaxSpeed", 2.0f);
		parameters->AddOgreString("MaterialName", "Wood");
		parameters->AddFloat("Density", 10);
	}

	void GOCCharacterController::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("Ogre::Vector3", &mDimensions, "Dimensions");
		mgr.SaveAtom("float", &mMovementSpeed, "MaxSpeed");
		mgr.SaveAtom("Ogre::String", &mMaterialName, "MaterialName");
		mgr.SaveAtom("float", &mDensity, "Density");
	}
	void GOCCharacterController::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("Ogre::Vector3", &mDimensions);
		mgr.LoadAtom("float", &mMovementSpeed);		//Load Save: Todo!
		mgr.LoadAtom("Ogre::String", &mMaterialName);
		mgr.LoadAtom("float", &mDensity);
		Create(mDimensions);
	}

	std::vector<ScriptParam> GOCCharacterController::Character_GetGroundMaterial(Script& caller, std::vector<ScriptParam> &vParams)
	{
		//TODO: Use code from IceCollisionCallback.cpp for per-triangle materials!
		std::string mat = "None";
		/*NxRaycastHit hit;
		if (Main::Instance().GetPhysXScene()->getNxScene()->raycastClosestShape(NxRay(OgrePhysX::Convert::toNx(mOwnerGO.lock()->GetGlobalPosition() + Ogre::Vector3(0,1,0)), NxVec3(0,-1,0)), NX_ALL_SHAPES, hit, 1<<CollisionGroups::LEVELMESH|1<<CollisionGroups::DEFAULT, 1.5f, NX_RAYCAST_MATERIAL))
		{
			mat = SceneManager::Instance().GetSoundMaterialTable().GetMaterialName(hit.materialIndex);
		}*/
		std::vector<ScriptParam> out;
		out.push_back(mat);
		return out;
	}
	std::vector<ScriptParam> GOCCharacterController::Character_SetSpeed(Script& caller, std::vector<ScriptParam> &vParams)
	{
		SetSpeedFactor(vParams[0].getFloat());
		SCRIPT_RETURN()
	}

};