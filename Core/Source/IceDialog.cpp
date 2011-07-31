
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
		JoinNewsgroup(GlobalMessageIDs::PHYSICS_BEGIN);
	}

	Dialog::~Dialog(void)
	{
	}

	void Dialog::OnSetActive(bool active)
	{
		if (active)
		{
			Msg startDialog;
			/*startDialog.type = "StartDialog";
			mAIObject->GetOwner()->SendMessage(startDialog);*/
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
		else
		{
			if (SceneManager::Instance().GetPlayer())
			{
				GOCCameraController *cam = (GOCCameraController*)SceneManager::Instance().GetPlayer()->GetComponent("Camera");
				cam->AttachCamera(Main::Instance().GetCamera());

				GOCPlayerInput *playerInput = (GOCPlayerInput*)SceneManager::Instance().GetPlayer()->GetComponent("CharacterInput", "PlayerInput");
				playerInput->Pause(false);
			}
		}
	}

	void Dialog::OnReceiveMessage(Msg &msg)
	{
		if (msg.typeID == GlobalMessageIDs::PHYSICS_BEGIN)
		{
			if (mDirectionBlender.HasNext())
			{
				Ogre::Vector3 direction = mDirectionBlender.Next(msg.params.GetValue<float>(0));
				Ogre::Quaternion quat = Ogre::Vector3::UNIT_Z.getRotationTo(direction);
				mAIObject->GetOwner()->SetGlobalOrientation(quat);
			}
		}
	}

}