
#include "IceGOCAI.h"
#include "IceAIManager.h"
#include "IceScriptSystem.h"
#include "IceGameObject.h"
#include "IceFollowPathwayProcess.h"
#include "IceDialog.h"
#include "IceObjectMessageIDs.h"
#include "IceProcessNodeManager.h"

namespace Ice
{

	GOCAI::GOCAI(void)
	{
		mCharacterMovementState = 0;
		AIManager::Instance().RegisterAIObject(this);
		JoinNewsgroup(GlobalMessageIDs::ENABLE_GAME_CLOCK);
		JoinNewsgroup(GlobalMessageIDs::REPARSE_SCRIPTS_PRE);
		mSeeSense = std::make_shared<SeeSense>(this, this);
	}

	GOCAI::~GOCAI(void)
	{
		AIManager::Instance().UnregisterAIObject(this);
	}

	int GOCAI::GetID()
	{
		GameObjectPtr owner = mOwnerGO.lock();
		if (owner.get()) return owner->GetID();
		throw Ogre::Exception(Ogre::Exception::ERR_INVALID_STATE, "Called GetID, but AI isn't attached to a game object!", "int GOCAI::GetID()");
	}

	void GOCAI::SetOwner(std::weak_ptr<GameObject> go)
	{
		mOwnerGO = go;
		GameObjectPtr owner = mOwnerGO.lock();
		if (!owner.get()) return;
		UpdatePosition(owner->GetGlobalPosition());
		UpdateOrientation(owner->GetGlobalOrientation());
	}

	void GOCAI::OnSeeSomething(const Ogre::Vector3 &eyeSpacePosition, float distance, float viewFactor, int goID)
	{
		GameObjectPtr owner = mOwnerGO.lock();
		if (owner.get())
		{
			Msg msg;
			msg.typeID = ObjectMessageIDs::AI_SEE;
			msg.params.AddOgreVec3("eyeSpacePosition", eyeSpacePosition);
			msg.params.AddFloat("distance", distance);
			msg.params.AddFloat("viewFactor", viewFactor);
			msg.params.AddInt("object", goID);
			owner->BroadcastObjectMessage(msg);
		}
	}

	void GOCAI::Update(float time)
	{
		if (GetOwner().get())
			mSeeSense->UpdateSense(time);
	}

	Ogre::Vector3 GOCAI::GetEyePosition()
	{
		Ogre::Vector3 forwardOffset = GetEyeOrientation() * Ogre::Vector3(0, 0, 0.5f);
		return GetOwner()->GetGlobalPosition() + Ogre::Vector3(0, 1.5f, 0) + forwardOffset;
	}
	Ogre::Quaternion GOCAI::GetEyeOrientation()
	{
		return GetOwner()->GetGlobalOrientation();
	}

	std::vector<ScriptParam> GOCAI::Npc_GetObjectVisibility(Script& caller, std::vector<ScriptParam> &vParams)
	{
		int id = vParams[0].getInt();
		GameObjectPtr go = SceneManager::Instance().GetObjectByInternID(id);
		if (!go.get()) SCRIPT_RETURNERROR("Object (" + Ogre::StringConverter::toString(id) + ") not found!")
		SeeSense::VisualObject *visual = go->GetComponent<SeeSense::VisualObject>();
		if (!visual) SCRIPT_RETURNERROR("Object (" + Ogre::StringConverter::toString(id) + ") must have a visual component!")
		SCRIPT_RETURNVALUE(mSeeSense->GetObjectVisibility(visual))
	}

	std::vector<ScriptParam> GOCAI::Npc_CreateFollowPathwayProcess(Script& caller, std::vector<ScriptParam> &vParams)
	{
		GameObjectPtr owner = mOwnerGO.lock();
		if (owner.get()) SCRIPT_RETURNERROR("AI must be attached to an object!")
		int targetID = vParams[0].getInt();
		GameObjectPtr targetGO = SceneManager::Instance().GetObjectByInternID(targetID);
		if (!targetGO.get()) SCRIPT_RETURNERROR("Object (" + Ogre::StringConverter::toString(targetID) + ") not found!")

		SCRIPT_RETURNVALUE(ProcessNodeManager::Instance().CreateFollowPathwayProcess(owner->GetComponentPtr<GOCAI>(), targetGO->GetGlobalPosition()))
	}
	std::vector<ScriptParam> GOCAI::Npc_OpenDialog(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		return out;
	}

	void GOCAI::ReceiveMessage(Msg &msg)
	{
		if (mOwnerGO.expired()) return;
		if (msg.typeID == GlobalMessageIDs::ENABLE_GAME_CLOCK)
		{
			bool enable = msg.params.GetBool("enable");
			if (!enable)
			{
				BroadcastMovementState(0);
			}
		}
		else if (msg.typeID == GlobalMessageIDs::REPARSE_SCRIPTS_PRE)
		{
		}
	}

	void GOCAI::SetParameters(DataMap *parameters)
	{
	}
	void GOCAI::GetParameters(DataMap *parameters)
	{
	}
	void GOCAI::GetDefaultParameters(DataMap *parameters)
	{
	}

	void GOCAI::Save(LoadSave::SaveSystem& mgr)
	{
		//Todo: Save current state
	}
	void GOCAI::Load(LoadSave::LoadSystem& mgr)
	{
		//Todo
	}

};