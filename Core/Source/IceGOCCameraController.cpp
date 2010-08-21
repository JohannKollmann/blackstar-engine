#include "IceGOCCameraController.h"
#include "OIS/OIS.h"
#include "IceMessageSystem.h"
#include "IceGameObject.h"
#include "IceMain.h"
#include "IceCameraController.h"
#include "IceSceneManager.h"
#include "IceGOCOgreNode.h"

namespace Ice
{

	GOCSimpleCameraController::GOCSimpleCameraController()
	{
		mCamera = nullptr;
	}

	GOCSimpleCameraController::~GOCSimpleCameraController()
	{
		if (mCamera) DetachCamera();
	}

	void GOCSimpleCameraController::AttachCamera(Ogre::Camera *camera)
	{
		mCamera = camera;
		mCamera->setPosition(0,0,0);
		mCamera->lookAt(0,0,1);
		GetNode()->attachObject(mCamera);
	}
	void GOCSimpleCameraController::DetachCamera()
	{
		if (!mCamera) return;
		GetNode()->detachObject(mCamera);
		mCamera = nullptr;
	}	
	void GOCSimpleCameraController::ReceiveObjectMessage(Msg &msg)
	{
		if (msg.type == "MOVER_START")
		{
			SceneManager::Instance().AcquireCamera(this);
		}
		else if (msg.type == "MOVER_END")
		{
			SceneManager::Instance().TerminateCurrentCameraController();
		}
	}


	GOCCameraController::GOCCameraController()
	{
		mCamera = nullptr;
		mCameraCenterNode = nullptr;
		mCameraNode = nullptr;
		mTargetNode = nullptr;
	}

	GOCCameraController::~GOCCameraController()
	{
		if (mCamera) DetachCamera();
		if (mCameraCenterNode) Main::Instance().GetOgreSceneMgr()->destroySceneNode(mCameraCenterNode);
		if (mCameraNode) Main::Instance().GetOgreSceneMgr()->destroySceneNode(mCameraNode);
		if (mTargetNode) Main::Instance().GetOgreSceneMgr()->destroySceneNode(mTargetNode);
	}

	void GOCCameraController::CreateNodes()
	{
		mCameraCenterNode = GetNode()->createChildSceneNode(Ogre::Vector3(0, 2, 0));
		mCameraCenterNode->setInheritOrientation(false);
		mTargetNode = mCameraCenterNode->createChildSceneNode(Ogre::Vector3(0,0,10));
		mCameraNode = mCameraCenterNode->createChildSceneNode(Ogre::Vector3(0,0,0));

		mTightness = 0.003f;
		mfRefCharacterAngle = 0;
		mfZoom=-1.0;

		mCameraNode->setFixedYawAxis (true);
	
		mCameraNode->setAutoTracking(true, mTargetNode);
	}

	void GOCCameraController::AttachCamera(Ogre::Camera *camera)
	{
		if (!mCameraCenterNode) CreateNodes();

		mCameraCenterNode->resetOrientation();
		mCamera = camera;
		mCameraNode->attachObject(mCamera);
		mCamera->setPosition(0,0,0);
		mCamera->setOrientation(Ogre::Vector3::UNIT_Z.getRotationTo(Ogre::Vector3(0,0,1)));

		MessageSystem::Instance().JoinNewsgroup(this, "MOUSE_MOVE");
		MessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
		MessageSystem::Instance().JoinNewsgroup(this, "KEY_DOWN");
	}
	void GOCCameraController::DetachCamera()
	{
		Msg msg;
		msg.type = "LEAVE_FPS_MODE";
		if (mOwnerGO) mOwnerGO->SendInstantMessage(msg);
		if (!mCamera) return;
		mCameraNode->detachObject(mCamera);
		MessageSystem::Instance().QuitAllNewsgroups(this);
		mCamera = nullptr;
	}	

	void GOCCameraController::SetOwner(GameObject *go)
	{
		mOwnerGO = go;
		CreateNodes();
		UpdatePosition(go->GetGlobalPosition());
		UpdateOrientation(go->GetGlobalOrientation());
	}

	void GOCCameraController::ReceiveMessage(Msg &msg)
	{
		static double sfAbsRefPitch=0.0;
		static double sfAbsCamPitch=0.0;
		static double sfAbsCamYaw=0.0;
		static double sfActualZoom=0.0;
	
		if (msg.type == "KEY_DOWN")
		{
			OIS::KeyCode kc = (OIS::KeyCode)msg.params.GetInt("KEY_ID_OIS");
			if (kc == OIS::KC_PGDOWN)
				mfZoom += (mfZoom+0.1f)*0.2f;
			if (kc == OIS::KC_PGUP)
				mfZoom -= (mfZoom+0.1f)*0.2f;
			if(mfZoom>1.5)
				mfZoom=1.5;
			if(mfZoom<0.0)
				mfZoom=0.0;
		}
		if (msg.type == "MOUSE_MOVE")
		{
			float oldZoom = mfZoom;
			mfZoom -= (mfZoom+0.1f)*(msg.params.GetInt("ROT_Z_REL")*0.001f);

			if(mfZoom>1.5f)
				mfZoom=1.5f;
			if(mfZoom<0.0f)
				mfZoom=0.0f;
		
			double pitch = msg.params.GetInt("ROT_Y_REL")*mTightness;
			sfAbsRefPitch+=pitch;
			if(sfAbsRefPitch>=Ogre::Math::PI*0.4f)
				sfAbsRefPitch=Ogre::Math::PI*0.4f;
			if(sfAbsRefPitch<=-Ogre::Math::PI*0.4f)
				sfAbsRefPitch=-Ogre::Math::PI*0.4f;
			if(mfZoom>0.0f)
			{
				if (oldZoom == 0.0f)
				{
					Msg msg;
					msg.type = "LEAVE_FPS_MODE";
					mOwnerGO->SendInstantMessage(msg);
				}
				mCameraNode->setPosition(Ogre::Vector3(0.0f,0.0f,-6.0f)*static_cast<float>(sfActualZoom));
				mCameraNode->setAutoTracking(true, mTargetNode);
			}
			else
			{
				if (oldZoom != 0.0f)
				{
					Msg msg;
					msg.type = "ENTER_FPS_MODE";
					mOwnerGO->SendInstantMessage(msg);
				}
				sfActualZoom=0.0f;
				mCameraNode->setPosition(0,0,0);
				mCameraNode->setAutoTracking(false);
				sfAbsCamPitch=sfAbsRefPitch;
				sfAbsCamYaw=mfRefCharacterAngle;
			}
		
		}
		if (msg.type == "UPDATE_PER_FRAME")
		{
			float time = msg.params.GetFloat("TIME");
			if(mfZoom>0.0)
			{
				const double cfSpeed=7.0f;
				if(time>0.5f/cfSpeed)
					time=0.5f/static_cast<float>(cfSpeed);

				double fRelPitch=(sfAbsCamPitch-sfAbsRefPitch)*time*cfSpeed;
				sfAbsCamPitch-=fRelPitch;

				double fRelAngle=(((mfRefCharacterAngle-sfAbsCamYaw)/(2.0f*Ogre::Math::PI)-Ogre::Math::Floor((mfRefCharacterAngle-sfAbsCamYaw)/(2.0f*Ogre::Math::PI)+0.5)+0.5f)*2.0f-1.0f)*Ogre::Math::PI;
				double fRelYaw=fRelAngle*time*cfSpeed;
				sfAbsCamYaw+=fRelYaw;

				sfActualZoom-=(sfActualZoom-mfZoom)*time*cfSpeed*0.3f;
				mCameraNode->setPosition(Ogre::Vector3(0.0f,0.0f,-6.0f)*static_cast<float>(sfActualZoom));
			}
		}
		Ogre::Matrix3 mat3;
		mat3.FromEulerAnglesYXZ(-Ogre::Radian(sfAbsCamYaw), Ogre::Radian(sfAbsCamPitch), Ogre::Radian(0.0f));
		Ogre::Quaternion q;
		q.FromRotationMatrix(mat3);
		mCameraCenterNode->setOrientation(q);

	}

	void GOCCameraController::UpdateOrientation(Ogre::Quaternion orientation)
	{
		mfRefCharacterAngle=-orientation.getYaw().valueRadians();
	}

};