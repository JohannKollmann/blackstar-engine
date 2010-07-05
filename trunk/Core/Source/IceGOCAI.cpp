
#include "IceGOCAI.h"
#include "IceAIManager.h"
#include "IceScriptSystem.h"
#include "IceGameObject.h"
#include "IceFollowPathway.h"

namespace Ice
{

	GOCAI::GOCAI(void)
	{
		mCharacterMovementState = 0;
		mOwnerGO = 0;
	}

	GOCAI::~GOCAI(void)
	{
		ClearActionQueue();
		ClearIdleQueue();
	}

	int GOCAI::GetID()
	{
		if (mOwnerGO) return mOwnerGO->GetID();
		throw Ogre::Exception(Ogre::Exception::ERR_INVALID_STATE, "Called GetID, but AI isn't attached to a game object!", "int GOCAI::GetID()");
	}

	void GOCAI::SetOwner(GameObject *go)
	{
		if (mOwnerGO)
		{
			MessageSystem::Instance().QuitAllNewsgroups(this);
			AIManager::Instance().UnregisterAIObject(mOwnerGO->GetID());
		}
		mOwnerGO = go;

		if (mOwnerGO)
		{
			MessageSystem::Instance().JoinNewsgroup(this, "ENABLE_GAME_CLOCK");

			UpdatePosition(mOwnerGO->GetGlobalPosition());
			UpdateOrientation(mOwnerGO->GetGlobalOrientation());

			AIManager::Instance().RegisterAIObject(this, mOwnerGO->GetID());
			std::vector<ScriptParam> params;
			params.push_back(ScriptParam(mOwnerGO->GetID()));
			InitScript(mScriptFileName);
		}
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
			delete (*i);
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
		delete tmp;

		if (mActionQueue.size() > 0) mActionQueue[0]->Enter();
	}

	void GOCAI::ClearIdleQueue()
	{
		for (std::vector<DayCycle*>::iterator i = mIdleQueue.begin(); i != mIdleQueue.end(); i++)
		{
			delete (*i);
		}
		mIdleQueue.clear();
	}

	void GOCAI::Update(float time)
	{
		if (mOwnerGO == 0) return;
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

	std::vector<ScriptParam> GOCAI::Npc_AddState(std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		return out;
	}
	std::vector<ScriptParam> GOCAI::Npc_KillActiveState(std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		LeaveActiveActionState();
		return out;
	}
	std::vector<ScriptParam> GOCAI::Npc_ClearQueue(std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		return out;
		ClearActionQueue();
	}
	std::vector<ScriptParam> GOCAI::Npc_AddTA(std::vector<ScriptParam> &vParams)
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
		
		AddDayCycleState(new DayCycle(this, ta_script, miscparams, end_timeH, end_timeM, time_abs));
		return out;
	}
	std::vector<ScriptParam> GOCAI::Npc_GotoWP(std::vector<ScriptParam> &vParams)
	{
		std::vector<ScriptParam> out;
		Ogre::String wp = vParams[0].getString().c_str();
		AddState(new FollowPathway(this, wp));
		return out;
	}

	void GOCAI::ReceiveObjectMessage(const Msg &msg)
	{
		if (msg.type == "CharacterJumpEnded")
		{
			mCharacterMovementState = mCharacterMovementState & ~CharacterMovement::JUMP;
			BroadcastMovementState();
		}
		if (msg.type == "CharacterCollisionReport")
		{
			NxU32 collisionFlags = msg.params.GetInt("collisionFlags");
		}
	}

	void GOCAI::OnReceiveMessage(Msg &msg)
	{
		if (msg.type == "ENABLE_GAME_CLOCK")
		{
			bool enable = msg.params.GetBool("enable");
			if (!enable)
			{
				BroadcastMovementState(0);
			}
		}
	}

	void GOCAI::OnScriptReload()
	{
		ClearActionQueue();
		ClearIdleQueue();
	}

	int GOCAI::GetThisID()
	{
		IceAssert(mOwnerGO);
		return mOwnerGO->GetID();
	}

	void GOCAI::SetParameters(DataMap *parameters)
	{
		mScriptFileName = parameters->GetOgreString("Script");
		SetOwner(mOwnerGO);
	}
	void GOCAI::GetParameters(DataMap *parameters)
	{
		parameters->AddOgreString("Script", mScriptFileName);
	}
	void GOCAI::GetDefaultParameters(DataMap *parameters)
	{
		parameters->AddOgreString("Script", "script.lua");
	}

	void GOCAI::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("Ogre::String", &mScriptFileName, "Script");
	}
	void GOCAI::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("Ogre::String", &mScriptFileName);
	}

};