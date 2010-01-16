
#pragma once

#include "NxControllerManager.h"
#include "IceMessageListener.h"
#include "IceGOCPhysics.h"

namespace Ice
{

enum CharacterMovement
{
	FORWARD = 1,
	BACKWARD = 2,
	LEFT = 4,
	RIGHT = 8,
	JUMP = 16,
	RUN = 32,
	CROUCH = 64,
};

class CharacterJump
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

class CharacterControllerInput : public GOComponent
{
protected:
	int mCharacterMovementState;

	CharacterControllerInput(void) {};
	virtual ~CharacterControllerInput(void) {};

	void BroadcastMovementState();

public:
	goc_id_family& GetFamilyID() const { static std::string name = "CharacterInput"; return name; }
	void BroadcastMovementState(int state);
	int GetMovementState() { return mCharacterMovementState; }
};

class DllExport GOCCharacterController : public GOCPhysics, public MessageListener, public GOCEditorInterface
{
private:
	NxController *mCharacterController;
	void Create(Ogre::Vector3 dimensions);
	CharacterJump mJump;
	Ogre::Vector3 mDirection;
	Ogre::Vector3 mDimensions;
	float mStepOffset;
	bool mFreezed;

	float mMovementSpeed;

public:
	GOCCharacterController() { mCharacterController = 0; }
	GOCCharacterController(Ogre::Vector3 dimensions);
	~GOCCharacterController(void);

	GOComponent::goc_id_type& GetComponentID() const { static std::string name = "CharacterController"; return name; }

	void UpdatePosition(Ogre::Vector3 position);
	void UpdateOrientation(Ogre::Quaternion orientation);
	void UpdateScale(Ogre::Vector3 scale);

	NxController* GetNxController() { return mCharacterController; }

	void ReceiveObjectMessage(Ogre::SharedPtr<ObjectMsg> msg);
	void ReceiveMessage(Msg &msg);

	void Freeze(bool freeze);

	void SetOwner(GameObject *go);

	void CreateFromDataMap(DataMap *parameters);
	void GetParameters(DataMap *parameters);
	static void GetDefaultParameters(DataMap *parameters);
	static GOCEditorInterface* NewEditorInterfaceInstance() { return new GOCCharacterController(); }
	void AttachToGO(GameObject *go); 
	Ogre::String GetLabel() { return "Character"; }

	void Save(LoadSave::SaveSystem& mgr);
	void Load(LoadSave::LoadSystem& mgr);
	std::string& TellName() { static std::string name = "CharacterController"; return name; };
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "CharacterController"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
	static LoadSave::Saveable* NewInstance() { return new GOCCharacterController; };
};


};