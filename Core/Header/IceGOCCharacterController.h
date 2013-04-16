
#pragma once


#include "IceMessageListener.h"
#include "IceGOCPhysics.h"
#include "IceGOCScriptMakros.h"
#include "PxPhysicsAPI.h"

namespace Ice
{
	using namespace physx;

	enum DllExport CharacterMovement
	{
		FORWARD = 1,
		BACKWARD = 2,
		LEFT = 4,
		RIGHT = 8,
		RUN = 32,
		CROUCH = 64,
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

	class DllExport GOCCharacterController : public GOCPhysics, public GOCEditorInterface, public SeeSense::VisualObject
	{
	private:
		OgrePhysX::Actor<PxRigidDynamic> mActor;
		void Create(Ogre::Vector3 dimensions);
		Ogre::Vector3 mDirection;
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

		void UpdatePosition(const Ogre::Vector3 &position);
		void UpdateOrientation(const Ogre::Quaternion &orientation);
		void UpdateScale(const Ogre::Vector3 &scale);

		void SetSpeedFactor(float factor);

		void ReceiveMessage(Msg &msg);

		void Freeze(bool freeze);

		void SetOwner(std::weak_ptr<GameObject> go);

		OgrePhysX::Actor<PxRigidDynamic>* GetActor() { return &mActor; }

		Ogre::String GetVisualObjectDescription();
		void GetTrackPoints(std::vector<Ogre::Vector3> &outPoints);

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