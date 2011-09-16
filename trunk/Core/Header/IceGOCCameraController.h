#pragma once

#include "IceGOComponent.h"
#include "IceIncludes.h"
#include "IceMessageListener.h"
#include "IceGOCCharacterController.h"
#include "IceGOCOgreNode.h"
#include "IceCameraController.h"

namespace Ice
{

	class DllExport GOCSimpleCameraController : public CameraController, public GOCOgreNodeUserEditable
	{
	protected:
		Ogre::Camera *mCamera;

	public:
		GOCSimpleCameraController();
		virtual ~GOCSimpleCameraController();

		virtual void AttachCamera(Ogre::Camera *camera);
		virtual void DetachCamera();

		virtual void ReceiveMessage(Msg &msg);

		GOComponent::FamilyID& GetFamilyID() const { static std::string name = "Camera"; return name; }
		virtual GOComponent::TypeID& GetComponentID() const { static std::string name = "SimpleCamera"; return name; }

		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "SimpleCamera"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCSimpleCameraController; };

		GOCEditorInterface* New() { return new GOCSimpleCameraController(); }
		Ogre::String GetLabel() { return "Simple Camera"; }
	};
	
	class DllExport GOCCameraController : public GOCSimpleCameraController
	{
	private:
		Ogre::Camera *mCamera;
		Ogre::SceneNode *mCameraNode;
		Ogre::SceneNode *mPitchNode;
		Ogre::SceneNode *mCameraCenterNode;
		float mTightness;
		float mInterpolationFactor;
		double mMaxPitch;

		float mfZoom;

	public:
		GOCCameraController();
		~GOCCameraController(void);

		void CreateNodes();
		void AttachCamera(Ogre::Camera *camera);
		void DetachCamera();

		void SetOwner(std::weak_ptr<GameObject> go);

		AccessPermissionID GetAccessPermissionID() { return AccessPermissions::ACCESS_VIEW; }

		void ReceiveMessage(Msg &msg);

		void UpdateOrientation(const Ogre::Quaternion &orientation);

		GOComponent::TypeID& GetComponentID() const { static std::string name = "CameraController"; return name; }

		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "CameraController"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCCameraController; };

		//Editor
		void SetParameters(DataMap *parameters) { };
		void GetDefaultParameters(DataMap *parameters) {};
		GOCEditorInterface* New() { return new GOCCameraController(); }
		Ogre::String GetLabel() { return "Camera Contoller"; }
	};

};