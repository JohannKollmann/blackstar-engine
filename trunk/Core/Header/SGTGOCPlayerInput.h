#pragma once

#include "SGTGOComponent.h"
#include "SGTIncludes.h"
#include "SGTMessageListener.h"

class SGTGOCPlayerInput : public SGTGOComponent, public SGTMessageListener
{
private:
	Ogre::Vector3 mDirection;

public:
	SGTGOCPlayerInput(void);
	~SGTGOCPlayerInput(void);

	void ReceiveMessage(SGTMsg &msg);

	goc_id_family& GetFamilyID() const { static std::string name = "GOCInput"; return name; }
	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "PlayerInput"; return name; }

	std::string& TellName() { static std::string name = "PlayerInput"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "PlayerInput"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTGOCPlayerInput* NewInstance() { return new SGTGOCPlayerInput; };
};


class SGTGOCCameraController : public SGTGOComponent, public SGTMessageListener
{
private:
	Ogre::Camera *mCamera;
	Ogre::SceneNode *mCenterNode;
	Ogre::Vector3 mTargetOffset;
	Ogre::Vector3 mPositionOffset;

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
	static SGTGOCCameraController* NewInstance() { return new SGTGOCCameraController; };
};