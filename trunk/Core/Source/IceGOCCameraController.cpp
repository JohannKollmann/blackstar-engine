#include "IceGOCCameraController.h"
#include "OIS/OIS.h"
#include "IceMessageSystem.h"
#include "IceGameObject.h"
#include "IceMain.h"
#include "IceCameraController.h"

namespace Ice
{

GOCCameraController::GOCCameraController()
{
	CreateNodes();
	mCamera = 0;
}
GOCCameraController::GOCCameraController(Ogre::Camera *camera)
{
	CreateNodes();
	AttachCamera(camera);
}

GOCCameraController::~GOCCameraController()
{
	if (mCamera) DetachCamera();
	Main::Instance().GetOgreSceneMgr()->destroySceneNode(mCharacterCenterNode);
	Main::Instance().GetOgreSceneMgr()->destroySceneNode(mCameraCenterNode);
	Main::Instance().GetOgreSceneMgr()->destroySceneNode(mCameraNode);
	Main::Instance().GetOgreSceneMgr()->destroySceneNode(mTargetNode);
}

void GOCCameraController::CreateNodes()
{
	mCharacterCenterNode = Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode();
	mCameraCenterNode = mCharacterCenterNode->createChildSceneNode(Ogre::Vector3(0, 2, 0));
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
	mCameraCenterNode->resetOrientation();
	mCamera = camera;
	mCameraNode->attachObject(mCamera);
	mCamera->setPosition(0,0,0);
	mCamera->setOrientation(Ogre::Vector3::UNIT_Z.getRotationTo(Ogre::Vector3(0,0,1)));

	MessageSystem::Instance().JoinNewsgroup(this, "MOUSE_MOVE");
	MessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
}
void GOCCameraController::DetachCamera()
{
	if (!mCamera) return;
	mCameraNode->detachObject(mCamera);
	MessageSystem::Instance().QuitNewsgroup(this, "MOUSE_MOVE");
	MessageSystem::Instance().QuitNewsgroup(this, "UPDATE_PER_FRAME");
	mCamera = 0;
}	

void GOCCameraController::ReceiveMessage(Msg &msg)
{
	static double sfAbsRefPitch=0.0;
	static double sfAbsCamPitch=0.0;
	static double sfAbsCamYaw=0.0;
	if (msg.mNewsgroup == "MOUSE_MOVE")
	{
		mfZoom -= (mfZoom+0.1)*(msg.mData.GetInt("ROT_Z_REL")*0.001);
		if(mfZoom>1.5)
			mfZoom=1.5;
		if(mfZoom<0.0)
			mfZoom=0.0;
		
		double pitch = msg.mData.GetInt("ROT_Y_REL")*mTightness;
		sfAbsRefPitch+=pitch;
		if(sfAbsRefPitch>=Ogre::Math::PI*0.4)
			sfAbsRefPitch=Ogre::Math::PI*0.4;
		if(sfAbsRefPitch<=-Ogre::Math::PI*0.4)
			sfAbsRefPitch=-Ogre::Math::PI*0.4;
		if(mfZoom>0.0)
		{
			mCameraNode->setPosition(Ogre::Vector3(0,0.0f,-6)*mfZoom);
			mCameraNode->setAutoTracking(true, mTargetNode);
		}
		else
		{
			mCameraNode->setPosition(0,0,0);
			mCameraNode->setAutoTracking(false);
			sfAbsCamPitch=sfAbsRefPitch;
			sfAbsCamYaw=mfRefCharacterAngle;
		}
		
	}
	if (msg.mNewsgroup == "UPDATE_PER_FRAME")
	{
		float time = msg.mData.GetFloat("TIME");
		if(mfZoom>0.0)
		{
			const double cfSpeed=7.0f;
			if(time>0.5/cfSpeed)
				time=0.5/cfSpeed;

			double fRelPitch=(sfAbsCamPitch-sfAbsRefPitch)*time*cfSpeed;
			sfAbsCamPitch-=fRelPitch;

			double fRelAngle=(((mfRefCharacterAngle-sfAbsCamYaw)/(2.0*Ogre::Math::PI)-Ogre::Math::Floor((mfRefCharacterAngle-sfAbsCamYaw)/(2.0*Ogre::Math::PI)+0.5)+0.5)*2.0-1)*Ogre::Math::PI;
			double fRelYaw=fRelAngle*time*cfSpeed;
			sfAbsCamYaw+=fRelYaw;
		}
	}
	Ogre::Matrix3 mat3;
	mat3.FromEulerAnglesYXZ(-Ogre::Radian(sfAbsCamYaw), Ogre::Radian(sfAbsCamPitch), Ogre::Radian(0));
	Ogre::Quaternion q;
	q.FromRotationMatrix(mat3);
	mCameraCenterNode->setOrientation(q);

}

void GOCCameraController::AttachToGO(GameObject *go)
{
	go->RemoveComponent(GetFamilyID());
	go->AddComponent(this);
}

void GOCCameraController::ReceiveObjectMessage(Ogre::SharedPtr<ObjectMsg> msg)
{
}

void GOCCameraController::UpdatePosition(Ogre::Vector3 position)
{
	mCharacterCenterNode->setPosition(position);
}

void GOCCameraController::UpdateOrientation(Ogre::Quaternion orientation)
{
	mfRefCharacterAngle=-orientation.getYaw().valueRadians();
}

};