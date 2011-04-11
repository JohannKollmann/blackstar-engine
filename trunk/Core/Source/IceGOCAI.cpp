
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
		MessageSystem::Instance().JoinNewsgroup(this, "ENABLE_GAME_CLOCK");
		MessageSystem::Instance().JoinNewsgroup(this, "REPARSE_SCRIPTS");
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

	void GOCAI::AddState(AIState *state)
	{
		AIState *old = 0;
		if (mActionQueue.size() > 0) old = mActionQueue[0];
		mActionQueue.push_back(state);
		std::push_heap(mActionQueue.begin(), mActionQueue.end(), AIState::PointerCompare_Less());
		if (old != mActionQueue[0])
		{
			if (old) old->Pause();
			mActionQueue[0]->Enter();
		}
	}

	void GOCAI::AddDayCycleState(DayCycle *state)
	{
		mIdleQueue.push_back(state);
		if (mIdleQueue.size() == 1) (*mIdleQueue.begin())->Enter();
	}

	void GOCAI::ClearActionQueue()
	{
		for (std::vector<AIState*>::iterator i = mActionQueue.begin(); i != mActionQueue.end(); i++)
		{
			ICE_DELETE (*i);
		}
		mActionQueue.clear();
	}

	void GOCAI::LeaveActiveActionState()
	{
		if (mActionQueue.size() == 0) return;
		std::pop_heap(mActionQueue.begin(), mActionQueue.end(), AIState::PointerCompare_Less());
		AIState *tmp = mActionQueue.back();
		mActionQueue.pop_back();
		tmp->Leave();
		ICE_DELETE tmp;

		if (mActionQueue.size() > 0) mActionQueue[0]->Enter();
	}

	void GOCAI::ClearIdleQueue()
	{
		for (std::vector<DayCycle*>::iterator i = mIdleQueue.begin(); i != mIdleQueue.end(); i++)
		{
			ICE_DELETE (*i);
		}
		mIdleQueue.clear();
	}

	void GOCAI::Update(float time)
	{
		if (mOwnerGO.expired()) return;
		if (mActionQueue.size() > 0)
		{
			bool finished = mActionQueue[0]->Update(time);
			if (finished)
				LeaveActiveActionState();
		}
		else if (mIdleQueue.size() > 0)
		{
			DayCycle *state = *mIdleQueue.begin();
			if (state->Update(0))
			{
				mIdleQueue.erase(mIdleQueue.begin());
				mIdleQueue.push_back(state);
				(*mIdleQueue.begin())->Enter();
			}
		}	
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
		
		AddDayCycleState(ICE_NEW DayCycle(this, ta_script, miscparams, end_timeH, end_timeM, time_abs));
		return out;
	}
	std::vector<ScriptParam> GOCAI::Npc_GotoWP(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		Ogre::String wp = vParams[0].getString().c_str();
		AddState(ICE_NEW FollowPathway(this, wp));
		return out;
	}
	std::vector<ScriptParam> GOCAI::Npc_OpenDialog(Script& caller, std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		AddState(ICE_NEW Dialog(this));
		return out;
	}

	void GOCAI::ReceiveObjectMessage(Msg &msg)
	{
		if (msg.type == "CharacterCollisionReport")
		{
			NxU32 collisionFlags = msg.params.GetInt("collisionFlags");
		}
	}

	void GOCAI::ReceiveMessage(Msg &msg)
	{
		if (mOwnerGO.expired()) return;
		if (msg.type == "ENABLE_GAME_CLOCK")
		{
			bool enable = msg.params.GetBool("enable");
			if (!enable)
			{
				BroadcastMovementState(0);
			}
		}
		else if (msg.type == "REPARSE_SCRIPTS")
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