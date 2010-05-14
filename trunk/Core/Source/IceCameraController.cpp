
#include "IceCameraController.h"
#include "IceInput.h"
#include "IceMessageSystem.h"
#include "IceMain.h"
#include "IceMsg.h"
#include "IceConsole.h"
#include "IceSceneManager.h"
#include "IceGOCAnimatedCharacter.h"
#include "OgreOggSound.h"

namespace Ice
{

CameraController::CameraController(void)
{
	MessageSystem::Instance().JoinNewsgroup(this, "MOUSE_MOVE");
	MessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
	MessageSystem::Instance().JoinNewsgroup(this, "CONSOLE_INGAME");
	MessageSystem::Instance().JoinNewsgroup(this, "KEY_UP");
	Console::Instance().AddCommand("cam_goto_pos", "vector3");

	mMovementVec = Ogre::Vector3(0,0,0);
	mYRot = true;
	mXRot = true;
	mMove = true;
	mRotSpeed = 0.2f;
	mMoveSpeed = 10.0f;
	mCamera = Main::Instance().GetCamera();
	mCamera->lookAt(Ogre::Vector3(0,0,0));
}

CameraController::~CameraController(void)
{
}

void CameraController::ReceiveMessage(Msg &msg)
{

	//Der folgende Code ermöglicht eine einfache Kamerasteuerung über WASD und die Maus und ist eher für Debug oder Editorzwecke gedacht.
	if (msg.type == "MOUSE_MOVE")
	{
		if (mYRot) mCamera->yaw(Ogre::Degree(-msg.params.GetInt("ROT_X_REL") * mRotSpeed));
		if (mXRot) mCamera->pitch(Ogre::Degree(-msg.params.GetInt("ROT_Y_REL") * mRotSpeed));
	}
	else if (msg.type == "UPDATE_PER_FRAME")
	{
		if (mMove)
		{
			float time = msg.params.GetFloat("TIME");
			mMovementVec = Ogre::Vector3(0,0,0);
			if (Main::Instance().GetInputManager()->isKeyDown(OIS::KeyCode::KC_A)) mMovementVec.x = -1;
			if (Main::Instance().GetInputManager()->isKeyDown(OIS::KeyCode::KC_D)) mMovementVec.x = 1;
			if (Main::Instance().GetInputManager()->isKeyDown(OIS::KeyCode::KC_W)) mMovementVec.z = -1;
			if (Main::Instance().GetInputManager()->isKeyDown(OIS::KeyCode::KC_S)) mMovementVec.z = 1;
			if (Main::Instance().GetInputManager()->isKeyDown(OIS::KeyCode::KC_Z)) mMovementVec.y = 1;	//Z und Y sind auf englischen Tastaturen vertauscht!
			if (Main::Instance().GetInputManager()->isKeyDown(OIS::KeyCode::KC_X)) mMovementVec.y = -1;
			mCamera->moveRelative(mMovementVec * time * mMoveSpeed);
		}
		Main::Instance().GetSoundManager()->getListener()->setPosition(mCamera->getDerivedPosition());
		Main::Instance().GetSoundManager()->getListener()->setOrientation(mCamera->getDerivedOrientation());
	}
	else if (msg.type == "CONSOLE_INGAME")
	{
		if (msg.params.GetOgreString("COMMAND") == "cam_goto_pos")
		{
			mCamera->setPosition(msg.params.GetOgreVec3("PARAM1"));
		}
	}
	else if (msg.type == "KEY_UP")
	{
		int keyid = msg.params.GetInt("KEY_ID_OIS");
		if (keyid == OIS::KC_F2)
		{
			/*Ogre::Entity *jaiqua = Main::Instance().GetOgreSceneMgr()->createEntity("ragdolltest" + Ogre::StringConverter::toString(SceneManager::Instance().RequestID()), "jaiqua.mesh");
			Ogre::Bone *bone = (Ogre::Bone*)jaiqua->getSkeleton()->getBone("Spine01")->getParent();
			Ogre::Vector3 offset = Ogre::Vector3(0,0,-3);
			Ogre::SceneNode *rnode = Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode("ragdolltest" + Ogre::StringConverter::toString(SceneManager::Instance().RequestID()), mCamera->getDerivedPosition() + (mCamera->getDerivedOrientation() * offset));
			rnode->attachObject(jaiqua);
			rnode->scale(0.1,0.1,0.1);
			GameObject *object = new GameObject();
			GOCAnimatedCharacter *ragdoll = new GOCAnimatedCharacter("robot.mesh", Ogre::Vector3(0.1f,0.1f,0.1f));
			//ragdoll->SetAnimationState("Walk");
			object->AddComponent(ragdoll);
			object->SetGlobalPosition(mCamera->getDerivedPosition());
			ragdoll->GetRagdoll()->sync();
			ragdoll->Kill();*/
		}
	}

}

void CameraController::Reset()
{
	if (mCamera->getParentSceneNode())
	{
		mCamera->getParentSceneNode()->detachObject(mCamera->getName());
	}
	mCamera->setPosition(Ogre::Vector3(0,0,0));
	mCamera->lookAt(Ogre::Vector3(0,0,1));
	//Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->attachObject(mCamera);
}

void CameraController::AttachToNode(Ogre::SceneNode *node, Ogre::Vector3 offset)
{
	Reset();
	mCamera->setPosition(offset);
	node->attachObject(mCamera);
}

};