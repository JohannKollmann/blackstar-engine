#pragma once

#include "SGTGOComponent.h"
#include "SGTIncludes.h"
#include "SGTMessageListener.h"
#include "SGTGOCCharacterController.h"

class SGTGOCPlayerInput : public SGTCharacterControllerInput, public SGTMessageListener
{

public:
	SGTGOCPlayerInput(void);
	~SGTGOCPlayerInput(void);

	void ReceiveMessage(SGTMsg &msg);

	void ReceiveObjectMessage(Ogre::SharedPtr<SGTObjectMsg> msg);
	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "PlayerInput"; return name; }

	std::string& TellName() { static std::string name = "PlayerInput"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "PlayerInput"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTSaveable* NewInstance() { return new SGTGOCPlayerInput; };
	virtual void Save(SGTSaveSystem& mgr) {};
	virtual void Load(SGTLoadSystem& mgr) {};
};


class SGTGOCCameraController : public SGTGOComponent, public SGTMessageListener
{
private:
	Ogre::Camera *mCamera;
	Ogre::SceneNode *mTargetNode;
	Ogre::SceneNode *mCameraNode;
	Ogre::SceneNode *mCharacterCenterNode;
	Ogre::SceneNode *mCameraCenterNode;
	float mTightness;

public:
	SGTGOCCameraController() {};
	SGTGOCCameraController(Ogre::Camera *camera);
	~SGTGOCCameraController(void);

	void ReceiveMessage(SGTMsg &msg);

	void ReceiveObjectMessage(Ogre::SharedPtr<SGTObjectMsg> msg);
	void UpdatePosition(Ogre::Vector3 position);
	void UpdateOrientation(Ogre::Quaternion orientation);

	goc_id_family& GetFamilyID() const { static std::string name = "GOCCamera"; return name; }
	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "CameraController"; return name; }

	std::string& TellName() { static std::string name = "CameraController"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "CameraController"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTSaveable* NewInstance() { return new SGTGOCCameraController; };
	virtual void Save(SGTSaveSystem& mgr) {};
	virtual void Load(SGTLoadSystem& mgr) {};
};