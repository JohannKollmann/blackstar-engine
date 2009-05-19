
#include "SGTCameraController.h"
#include "SGTInput.h"
#include "SGTMessageSystem.h"
#include "SGTMain.h"
#include "SGTMsg.h"
#include "SGTConsole.h"
#include "SGTSceneManager.h"
#include "SGTRagdoll.h"

SGTCameraController::SGTCameraController(void)
{
	SGTMessageSystem::Instance().JoinNewsgroup(this, "MOUSE_MOVE");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "CONSOLE_INGAME");
	SGTMessageSystem::Instance().JoinNewsgroup(this, "KEY_UP");
	SGTConsole::Instance().AddCommand("cam_goto_pos", "vector3");

	mMovementVec = Ogre::Vector3(0,0,0);
	mYRot = true;
	mXRot = true;
	mMove = true;
	mRotSpeed = 0.2f;
	mMoveSpeed = 10.0f;
	mCamera = SGTMain::Instance().GetCamera();
	mCamera->lookAt(Ogre::Vector3(0,0,0));
}

SGTCameraController::~SGTCameraController(void)
{
}

void SGTCameraController::ReceiveMessage(SGTMsg &msg)
{

	//Der folgende Code ermöglicht eine einfache Kamerasteuerung über WASD und die Maus und ist eher für Debug oder Editorzwecke gedacht.
	if (msg.mNewsgroup == "MOUSE_MOVE")
	{
		if (mYRot) mCamera->yaw(Ogre::Degree(-msg.mData.GetInt("ROT_X_REL") * mRotSpeed));
		if (mXRot) mCamera->pitch(Ogre::Degree(-msg.mData.GetInt("ROT_Y_REL") * mRotSpeed));
	}
	if (msg.mNewsgroup == "UPDATE_PER_FRAME")
	{
		if (mMove)
		{
			float time = msg.mData.GetFloat("TIME");
			mMovementVec = Ogre::Vector3(0,0,0);
			if (SGTMain::Instance().GetInputManager()->isKeyDown(OIS::KeyCode::KC_A)) mMovementVec.x = -1;
			if (SGTMain::Instance().GetInputManager()->isKeyDown(OIS::KeyCode::KC_D)) mMovementVec.x = 1;
			if (SGTMain::Instance().GetInputManager()->isKeyDown(OIS::KeyCode::KC_W)) mMovementVec.z = -1;
			if (SGTMain::Instance().GetInputManager()->isKeyDown(OIS::KeyCode::KC_S)) mMovementVec.z = 1;
			if (SGTMain::Instance().GetInputManager()->isKeyDown(OIS::KeyCode::KC_Z)) mMovementVec.y = 1;	//Z und Y sind auf englischen Tastaturen vertauscht!
			if (SGTMain::Instance().GetInputManager()->isKeyDown(OIS::KeyCode::KC_X)) mMovementVec.y = -1;
			mCamera->moveRelative(mMovementVec * time * mMoveSpeed);
		}
		SGTMain::Instance().GetSoundManager()->getListener()->setPosition(mCamera->getDerivedPosition());
		SGTMain::Instance().GetSoundManager()->getListener()->setOrientation(mCamera->getDerivedOrientation());
	}
	if (msg.mNewsgroup == "CONSOLE_INGAME")
	{
		if (msg.mData.GetOgreString("COMMAND") == "cam_goto_pos")
		{
			mCamera->setPosition(msg.mData.GetOgreVec3("PARAM1"));
		}
	}
	if (msg.mNewsgroup == "KEY_UP")
	{
		int keyid = msg.mData.GetInt("KEY_ID_OIS");
		if (keyid == OIS::KC_R)
		{
			/*Ogre::Entity *jaiqua = SGTMain::Instance().GetOgreSceneMgr()->createEntity("ragdolltest" + Ogre::StringConverter::toString(SGTSceneManager::Instance().RequestID()), "jaiqua.mesh");
			Ogre::Bone *bone = (Ogre::Bone*)jaiqua->getSkeleton()->getBone("Spine01")->getParent();
			Ogre::Vector3 offset = Ogre::Vector3(0,0,-3);
			Ogre::SceneNode *rnode = SGTMain::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode("ragdolltest" + Ogre::StringConverter::toString(SGTSceneManager::Instance().RequestID()), mCamera->getDerivedPosition() + (mCamera->getDerivedOrientation() * offset));
			rnode->attachObject(jaiqua);
			rnode->scale(0.1,0.1,0.1);*/
			SGTRagdoll *ragdoll = new SGTRagdoll("jaiqua.mesh", Ogre::Vector3(0.1,0.1,0.1));
			ragdoll->SetAnimationState("Walk");
			ragdoll->SetControlToActors();
		}
	}

}

void SGTCameraController::Reset()
{
	if (mCamera->getParentSceneNode())
	{
		mCamera->getParentSceneNode()->detachObject(mCamera->getName());
	}
	mCamera->setPosition(Ogre::Vector3(0,0,0));
	mCamera->lookAt(Ogre::Vector3(0,0,1));
	//SGTMain::Instance().GetOgreSceneMgr()->getRootSceneNode()->attachObject(mCamera);
}

void SGTCameraController::AttachToNode(Ogre::SceneNode *node, Ogre::Vector3 offset)
{
	Reset();
	mCamera->setPosition(offset);
	node->attachObject(mCamera);
}