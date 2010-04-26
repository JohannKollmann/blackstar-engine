
#include "IceDialog.h"
#include "IceGOCAI.h"
#include "IceSceneManager.h"
#include "IceGOCCameraController.h"
#include "IceGOCPlayerInput.h"
#include "IceMain.h"

namespace Ice
{

	Dialog::Dialog(GOCAI *ai)
	{
		mAIObject = ai;
	}

	Dialog::~Dialog(void)
	{
	}

	void Dialog::OnEnter()
	{
		mAIObject->mScript.CallFunction("StartDialog", std::vector<ScriptParam>());
		if (SceneManager::Instance().GetPlayer())
		{
			Ogre::Vector3 oldDir = mAIObject->GetOwner()->GetGlobalOrientation() * Ogre::Vector3::UNIT_Z;
			Ogre::Vector3 targetDir = SceneManager::Instance().GetPlayer()->GetGlobalPosition() - mAIObject->GetOwner()->GetGlobalPosition();
			mDirectionBlender.StartBlend(oldDir, targetDir);
		}
		if (SceneManager::Instance().GetPlayer())
		{
			GOCCameraController *cam = (GOCCameraController*)SceneManager::Instance().GetPlayer()->GetComponent("Camera");
			cam->DetachCamera();

			GOCPlayerInput *playerInput = (GOCPlayerInput*)SceneManager::Instance().GetPlayer()->GetComponent("CharacterInput", "PlayerInput");
			playerInput->Pause(true);
		}
	}
	void Dialog::Leave()
	{
		if (SceneManager::Instance().GetPlayer())
		{
			GOCCameraController *cam = (GOCCameraController*)SceneManager::Instance().GetPlayer()->GetComponent("Camera");
			cam->AttachCamera(Main::Instance().GetCamera());

			GOCPlayerInput *playerInput = (GOCPlayerInput*)SceneManager::Instance().GetPlayer()->GetComponent("CharacterInput", "PlayerInput");
			playerInput->Pause(false);
		}
	}
	bool Dialog::Update(float time)
	{
		if (mDirectionBlender.HasNext())
		{
			Ogre::Vector3 direction = mDirectionBlender.Next(time);
			Ogre::Quaternion quat = Ogre::Vector3::UNIT_Z.getRotationTo(direction);
			mAIObject->GetOwner()->SetGlobalOrientation(quat);
		}
		//Hack: Dieser State wird von auﬂen mit KillState beendet
		return false;
	}

}