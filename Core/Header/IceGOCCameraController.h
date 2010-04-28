#pragma once

#include "IceGOComponent.h"
#include "IceIncludes.h"
#include "IceMessageListener.h"
#include "IceGOCCharacterController.h"

namespace Ice
{
	
class DllExport GOCCameraController : public GOComponent, public MessageListener, public GOCEditorInterface
{
private:
	Ogre::Camera *mCamera;
	Ogre::SceneNode *mTargetNode;
	Ogre::SceneNode *mCameraNode;
	Ogre::SceneNode *mCharacterCenterNode;
	Ogre::SceneNode *mCameraCenterNode;
	float mTightness;
	Ogre::Quaternion mCharacterOrientation;

	float //mfCameraAngle,
		  //mfLastCharacterAngle,
		  mfRefCharacterAngle,
		  mfZoom;

public:
	GOCCameraController();
	GOCCameraController(Ogre::Camera *camera);
	~GOCCameraController(void);

	void CreateNodes();
	void AttachCamera(Ogre::Camera *camera);
	void DetachCamera();

	void ReceiveMessage(Msg &msg);

	void ReceiveObjectMessage(const Msg &msg);
	void UpdatePosition(Ogre::Vector3 position);
	void UpdateOrientation(Ogre::Quaternion orientation);

	goc_id_family& GetFamilyID() const { static std::string name = "Camera"; return name; }
	GOComponent::goc_id_type& GetComponentID() const { static std::string name = "CameraController"; return name; }

	std::string& TellName() { static std::string name = "CameraController"; return name; };
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "CameraController"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
	static LoadSave::Saveable* NewInstance() { return new GOCCameraController; };
	virtual void Save(LoadSave::SaveSystem& mgr) {};
	virtual void Load(LoadSave::LoadSystem& mgr) {};

	//Editor
	void CreateFromDataMap(DataMap *parameters) {};
	void GetParameters(DataMap *parameters) {};
	static void GetDefaultParameters(DataMap *parameters) {};
	static GOCEditorInterface* NewEditorInterfaceInstance() { return new GOCCameraController(); }
	void AttachToGO(GameObject *go); 
	Ogre::String GetLabel() { return "Camera"; }
};

};