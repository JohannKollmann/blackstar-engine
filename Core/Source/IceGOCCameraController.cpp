#include "IceGOCCameraController.h"
#include "OIS/OIS.h"
#include "IceMessageSystem.h"
#include "IceGameObject.h"
#include "IceMain.h"
#include "IceCameraController.h"
#include "IceSceneManager.h"
#include "IceGOCOgreNode.h"
#include "IceObjectMessageIDs.h"

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
	void GOCSimpleCameraController::ReceiveMessage(Msg &msg)
	{
		if (msg.typeID == ObjectMessageIDs::MOVER_START)
		{
			SceneManager::Instance().AcquireCamera(this);
		}
		else if (msg.typeID ==ObjectMessageIDs::MOVER_END)
		{
			SceneManager::Instance().TerminateCurrentCameraController();
		}
	}


	GOCCameraController::GOCCameraController()
	{
		mCamera = nullptr;
		mCameraCenterNode = nullptr;
		mCameraNode = nullptr;
		mPitchNode = nullptr;
		mMaxPitch = 0.4;
	}

	GOCCameraController::~GOCCameraController()
	{
		if (mCamera) DetachCamera();
		if (mCameraCenterNode) Main::Instance().GetOgreSceneMgr()->destroySceneNode(mCameraCenterNode);
		if (mPitchNode) Main::Instance().GetOgreSceneMgr()->destroySceneNode(mPitchNode);
		if (mCameraNode) Main::Instance().GetOgreSceneMgr()->destroySceneNode(mCameraNode);
	}

	void GOCCameraController::CreateNodes()
	{
		mCameraCenterNode = GetNode()->createChildSceneNode(Ogre::Vector3(0, 2.0f, 0) / GetNode()->_getDerivedScale());
		mCameraCenterNode->setInheritScale(false);
		mCameraCenterNode->setInheritOrientation(false);
		mPitchNode = mCameraCenterNode->createChildSceneNode(Ogre::Vector3(0,0,0));
		mCameraNode = mPitchNode->createChildSceneNode(Ogre::Vector3(0,0,0));

		mTightness = 0.003f;
		mfZoom=-1.0;

		mCameraNode->setFixedYawAxis (true);
	}

	void GOCCameraController::AttachCamera(Ogre::Camera *camera)
	{
		if (!mCameraCenterNode) CreateNodes();

		mCameraCenterNode->resetOrientation();
		mCamera = camera;
		mCamera->setDirection(0,0,1);
		mCameraNode->attachObject(mCamera);
		mCamera->setPosition(0,0,0);

		JoinNewsgroup(GlobalMessageIDs::MOUSE_MOVE);
		JoinNewsgroup(GlobalMessageIDs::UPDATE_PER_FRAME);
		JoinNewsgroup(GlobalMessageIDs::KEY_DOWN);
	}
	void GOCCameraController::DetachCamera()
	{
		Msg msg;
		msg.typeID = ObjectMessageIDs::LEAVE_FPS_MODE;
		BroadcastObjectMessage(msg);
		if (!mCamera) return;
		mCameraNode->detachObject(mCamera);
		QuitAllNewsgroups();
		mCamera = nullptr;
	}	

	void GOCCameraController::SetOwner(std::weak_ptr<GameObject> go)
	{
		mOwnerGO = go;
		GameObjectPtr owner = mOwnerGO.lock();
		if (!owner.get()) return;
		CreateNodes();
		UpdatePosition(owner->GetGlobalPosition());
		UpdateOrientation(owner->GetGlobalOrientation());
	}

	void GOCCameraController::ReceiveMessage(Msg &msg)
	{
		static double sfAbsRefPitch=0.0;
		static double sfAbsCamPitch=0.0;
		static double sfActualZoom=0.0;
	
		if (msg.typeID == GlobalMessageIDs::KEY_DOWN)
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
		if (msg.typeID == GlobalMessageIDs::MOUSE_MOVE)
		{
			float oldZoom = mfZoom;
			mfZoom -= (mfZoom+0.1f)*(msg.params.GetInt("ROT_Z_REL")*0.001f);

			if(mfZoom>1.5)
				mfZoom=1.5f;
			if(mfZoom<0.0f)
				mfZoom=0.0f;
		
			double pitch = msg.params.GetInt("ROT_Y_REL")*mTightness;
			sfAbsRefPitch+=pitch;
			if(sfAbsRefPitch>=Ogre::Math::PI*mMaxPitch)
				sfAbsRefPitch=Ogre::Math::PI*mMaxPitch;
			if(sfAbsRefPitch<=-Ogre::Math::PI*mMaxPitch)
				sfAbsRefPitch=-Ogre::Math::PI*mMaxPitch;
			if(mfZoom>0.0f)
			{
				if (oldZoom == 0.0f)
				{
					Msg leaveMsg;
					leaveMsg.typeID = ObjectMessageIDs::LEAVE_FPS_MODE;
					BroadcastObjectMessage(leaveMsg);
					mMaxPitch = 0.4f;
				}
				mCameraNode->setPosition(Ogre::Vector3(0.0f,0.0f,-6.0f)*static_cast<float>(sfActualZoom));
			}
			else
			{
				if (oldZoom != 0.0f)
				{
					Msg enterMsg;
					enterMsg.typeID = ObjectMessageIDs::ENTER_FPS_MODE;
					BroadcastObjectMessage(enterMsg);
					mMaxPitch = 0.6f;
				}
				sfActualZoom=0.0f;
				mCameraNode->setPosition(0,0,0);
				sfAbsCamPitch=sfAbsRefPitch;
			}
		
		}
		if (msg.typeID == GlobalMessageIDs::UPDATE_PER_FRAME)
		{
			float time = msg.params.GetFloat("TIME");
			if(mfZoom>0.0)
			{
				const double cfSpeed=7.0f;
				if(time>0.5f/cfSpeed)
					time=0.5f/static_cast<float>(cfSpeed);

				double fRelPitch=(sfAbsCamPitch-sfAbsRefPitch)*time*cfSpeed;
				sfAbsCamPitch-=fRelPitch;

				sfActualZoom-=(sfActualZoom-mfZoom)*time*cfSpeed*0.3f;
				mCameraNode->setPosition(Ogre::Vector3(0.0f,0.0f,-6.0f)*static_cast<float>(sfActualZoom));

				mCameraCenterNode->setOrientation(Ogre::Quaternion::Slerp(time*5, mCameraCenterNode->getOrientation(), mOwnerGO.lock()->GetGlobalOrientation(), true));
			}
			else mCameraCenterNode->setOrientation(mOwnerGO.lock()->GetGlobalOrientation());
		}

		mPitchNode->resetOrientation();
		mPitchNode->pitch(Ogre::Radian(sfAbsCamPitch));
	}

	void GOCCameraController::UpdateOrientation(const Ogre::Quaternion &orientation)
	{
	}

};