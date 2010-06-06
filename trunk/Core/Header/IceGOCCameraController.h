#pragma once

#include "IceGOComponent.h"
#include "IceIncludes.h"
#include "IceMessageListener.h"
#include "IceGOCCharacterController.h"
#include "IceGOCOgreNode.h"

namespace Ice
{

	class DllExport GOCSimpleCameraController : public GOCOgreNodeUserEditable
	{
	protected:
		Ogre::Camera *mCamera;

	public:
		GOCSimpleCameraController();
		virtual ~GOCSimpleCameraController();

		virtual void AttachCamera(Ogre::Camera *camera);
		virtual void DetachCamera();

		virtual void ReceiveObjectMessage(const Msg &msg);

		goc_id_family& GetFamilyID() const { static std::string name = "Camera"; return name; }
		virtual GOComponent::goc_id_type& GetComponentID() const { static std::string name = "SimpleCamera"; return name; }

		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "CameraController"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCSimpleCameraController; };

		GOCEditorInterface* New() { return new GOCSimpleCameraController(); }
		Ogre::String GetLabel() { return "Camera"; }
	};
	
	class DllExport GOCCameraController : public GOCSimpleCameraController, public MessageListener
	{
	private:
		Ogre::Camera *mCamera;
		Ogre::SceneNode *mTargetNode;
		Ogre::SceneNode *mCameraNode;
		Ogre::SceneNode *mCameraCenterNode;
		float mTightness;
		Ogre::Quaternion mCharacterOrientation;

		float //mfCameraAngle,
			  //mfLastCharacterAngle,
			  mfRefCharacterAngle,
			  mfZoom;

	public:
		GOCCameraController();
		~GOCCameraController(void);

		void CreateNodes();
		void AttachCamera(Ogre::Camera *camera);
		void DetachCamera();

		void ReceiveMessage(Msg &msg);

		void UpdateOrientation(Ogre::Quaternion orientation);

		GOComponent::goc_id_type& GetComponentID() const { static std::string name = "CameraController"; return name; }

		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "CameraController"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCCameraController; };

		//Editor
		void SetParameters(DataMap *parameters) { CreateNodes(); };
		void GetDefaultParameters(DataMap *parameters) {};
		GOCEditorInterface* New() { return new GOCCameraController(); }
		Ogre::String GetLabel() { return "Camera Contoller"; }
	};

};