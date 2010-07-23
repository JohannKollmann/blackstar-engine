
#pragma once

#include "IceIncludes.h"
#include "Ogre.h"
#include "IceMessageListener.h"

namespace Ice
{

	class DllExport CameraController
	{
	public:
		virtual ~CameraController() {}
		virtual void AttachCamera(Ogre::Camera *camera) = 0;
		virtual void DetachCamera() = 0;
	};

	class DllExport FreeFlightCameraController : public CameraController, public MessageListener
	{
	private:
		Ogre::Camera *mCamera;
		void Reset();

	public:
		FreeFlightCameraController(void);
		~FreeFlightCameraController(void);

		Ogre::Vector3 mMovementVec;
		bool mActive;
		bool mYRot;
		bool mXRot;
		bool mMove;
		float mRotSpeed;
		float mMoveSpeed;

		Ogre::Camera* GetCamera() { return mCamera; };
		void AttachToNode(Ogre::SceneNode *node, Ogre::Vector3 offset);

		void AttachCamera(Ogre::Camera *camera);
		void DetachCamera();

		void ReceiveMessage(Msg &msg);

	};

};