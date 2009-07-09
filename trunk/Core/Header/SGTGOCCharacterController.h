
#pragma once

#include "NxControllerManager.h"
#include "SGTMessageListener.h"
#include "SGTGOCPhysics.h"

enum SGTCharacterMovement
{
	FORWARD = 1,
	BACKWARD = 2,
	LEFT = 4,
	RIGHT = 8,
	JUMP = 16,
	RUN = 32,
	CROUCH = 64,
};

class SGTCharacterJump
{
private:
	float mV0;
	float mJumpTime;
public:
	SGTCharacterJump() : mV0(0), mJumping(false), mJumpTime(0) {}
	~SGTCharacterJump() {}

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

class SGTCharacterControllerInput : public SGTGOComponent
{
protected:
	int mCharacterMovementState;

	SGTCharacterControllerInput(void) {};
	virtual ~SGTCharacterControllerInput(void) {};

	void BroadcastMovementState();

public:
	goc_id_family& GetFamilyID() const { static std::string name = "GOCCharacterInput"; return name; }
};

class SGTDllExport SGTGOCCharacterController : public SGTGOCPhysics, public SGTMessageListener
{
private:
	NxController *mCharacterController;
	void Create(Ogre::Vector3 dimensions);
	SGTCharacterJump mJump;
	Ogre::Vector3 mDirection;
	float mStepOffset;

	float mMovementSpeed;

public:
	SGTGOCCharacterController() { mCharacterController = 0; }
	SGTGOCCharacterController(Ogre::Vector3 dimensions);
	~SGTGOCCharacterController(void);

	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "CharacterController"; return name; }

	void UpdatePosition(Ogre::Vector3 position);
	void UpdateOrientation(Ogre::Quaternion orientation);
	void UpdateScale(Ogre::Vector3 scale);

	void ReceiveObjectMessage(Ogre::SharedPtr<SGTObjectMsg> msg);
	void ReceiveMessage(SGTMsg &msg);

	void SetOwner(SGTGameObject *go);

	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	std::string& TellName() { static std::string name = "CharacterController"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "CharacterController"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTSaveable* NewInstance() { return new SGTGOCCharacterController; };
};
