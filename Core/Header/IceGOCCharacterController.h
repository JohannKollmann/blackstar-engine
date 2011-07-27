
#pragma once


#include "NxControllerManager.h"
#include "IceMessageListener.h"
#include "IceGOCPhysics.h"
#include <NxController.h> 
#include "IceGOCScriptMakros.h"

namespace Ice
{

	enum DllExport CharacterMovement
	{
		FORWARD = 1,
		BACKWARD = 2,
		LEFT = 4,
		RIGHT = 8,
		RUN = 32,
		CROUCH = 64,
	};

	class DllExport CharacterJump
	{
	private:
		float mV0;
		float mJumpTime;
	public:
		CharacterJump() : mV0(0), mJumping(false), mJumpTime(0) {}
		~CharacterJump() {}

		bool mJumping;

		void StopJump () { mJumping = false; }
		void StartJump(float v0)
		{
			if (mJumping) return;
			mJumpTime = 0.0f;
			mV0 = v0;
			mJumping = true;
		}
		float GetHeight(float elapsedTime)
		{
			if(!mJumping)	return 0.0f;
			mJumpTime += elapsedTime;
			float h = (-15*mJumpTime*mJumpTime) + mV0*mJumpTime + 15;
			if (h <= 0) return 0.0f;
			return h;
		}
	};

	class DllExport CharacterControllerInput : public GOComponent
	{
	protected:
		int mCharacterMovementState;

		CharacterControllerInput(void) {};
		virtual ~CharacterControllerInput(void) {};

		void BroadcastMovementState();

	public:
		GOComponent::FamilyID& GetFamilyID() const { static std::string name = "CharacterInput"; return name; }
		void BroadcastMovementState(int state);
		int GetMovementState() { return mCharacterMovementState; }
	};

	class DllExport GOCCharacterController : public GOCPhysics, public GOCEditorInterface
	{
	private:
		OgrePhysX::Actor *mActor;
		OgrePhysX::Actor *mSweepActor;
		NxSweepCache *mSweepCache;
		void Create(Ogre::Vector3 dimensions);
		CharacterJump mJump;
		Ogre::Vector3 mDirection;
		float mRadius;
		float mHeight;
		Ogre::Vector3 mDimensions;
		float mStepOffset;
		bool mFreezed;
		bool mTouchesGround;
		bool mJumping;
		unsigned long mJumpStartTime;
		unsigned long mTakeoffTime;

		float mMovementSpeed;
		float mSpeedFactor;
		float mDensity;

		Ogre::String mMaterialName;


		void _clear();

	public:
		GOCCharacterController() : mMovementSpeed(2.0f), mActor(nullptr), mDensity(10) {}
		GOCCharacterController(Ogre::Vector3 dimensions);
		~GOCCharacterController(void);

		AccessPermissionID GetAccessPermissionID() { return AccessPermissions::ACCESS_PHYSICS; }

		GOComponent::TypeID& GetComponentID() const { static std::string name = "CharacterController"; return name; }

		void UpdatePosition(Ogre::Vector3 position);
		void UpdateOrientation(Ogre::Quaternion orientation);
		void UpdateScale(Ogre::Vector3 scale);

		void SetSpeedFactor(float factor);

		void ReceiveMessage(Msg &msg);

		void Freeze(bool freeze);

		void SetOwner(std::weak_ptr<GameObject> go);

		OgrePhysX::Actor* GetActor() { return mActor; }

		void SetParameters(DataMap *parameters);
		void GetParameters(DataMap *parameters);
		void GetDefaultParameters(DataMap *parameters);
		GOCEditorInterface* New() { return new GOCCharacterController(); }
		Ogre::String GetLabel() { return "Character"; }
		GOComponent* GetGOComponent() { return this; }

		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		std::string& TellName() { static std::string name = "CharacterController"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "CharacterController"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCCharacterController; };

		//Scripting
		std::vector<ScriptParam> Character_GetGroundMaterial(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> Character_SetSpeed(Script& caller, std::vector<ScriptParam> &vParams);

		DEFINE_GOCLUAMETHOD(GOCCharacterController, Character_GetGroundMaterial)
		DEFINE_TYPEDGOCLUAMETHOD(GOCCharacterController, Character_SetSpeed, "float")
	};


};