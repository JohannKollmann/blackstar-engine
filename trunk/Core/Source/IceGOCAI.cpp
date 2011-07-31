
#include "IceGOCAI.h"
#include "IceAIManager.h"
#include "IceScriptSystem.h"
#include "IceGameObject.h"
#include "IceFollowPathway.h"
#include "IceDialog.h"

namespace Ice
{

	GOCAI::GOCAI(void)
	{
		mCharacterMovementState = 0;
		AIManager::Instance().RegisterAIObject(this);
		JoinNewsgroup(GlobalMessageIDs::ENABLE_GAME_CLOCK);
		JoinNewsgroup(GlobalMessageIDs::REPARSE_SCRIPTS_PRE);
	}

	GOCAI::~GOCAI(void)
	{
		ClearActionQueue();
		ClearIdleQueue();
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

	void GOCAI::AddDayCycleProcess(DayCycleProcess *state)
	{
	}

	void GOCAI::ClearActionQueue()
	{
	}

	void GOCAI::LeaveActiveActionState()
	{
	}

	void GOCAI::ClearIdleQueue()
	{
	}

	void GOCAI::Update(float time)
	{
	}

	std::vector<ScriptParam> GOCAI::Npc_AddState(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		return out;
	}
	std::vector<ScriptParam> GOCAI::Npc_KillActiveState(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		LeaveActiveActionState();
		return out;
	}
	std::vector<ScriptParam> GOCAI::Npc_ClearQueue(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		return out;
		ClearActionQueue();
	}
	std::vector<ScriptParam> GOCAI::Npc_AddTA(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		std::string ta_script = vParams[0].getString();
		int end_timeH = vParams[1].getInt();
		int end_timeM = vParams[2].getInt();
		bool time_abs = true;
		std::vector<ScriptParam> miscparams;
		std::vector<ScriptParam>::iterator i = vParams.begin();
		i++;i++;i++;
		for (; i != vParams.end(); i++)
		{
			miscparams.push_back((*i));
		}
		
		return out;
	}
	std::vector<ScriptParam> GOCAI::Npc_GotoWP(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		return out;
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
			ClearActionQueue();
			ClearIdleQueue();
		}
	}

	void GOCAI::SetParameters(DataMap *parameters)
	{
		ClearActionQueue();
		ClearIdleQueue();
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