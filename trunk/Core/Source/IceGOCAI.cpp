
#include "IceGOCAI.h"
#include "IceAIManager.h"
#include "IceScriptSystem.h"
#include "IceGameObject.h"

namespace Ice
{

GOCAI::GOCAI(void)
{
	mCharacterMovementState = 0;
	mOwnerGO = 0;
	MessageSystem::Instance().JoinNewsgroup(this, "ENABLE_GAME_CLOCK");
}

GOCAI::~GOCAI(void)
{
	ClearActionQueue();
	ClearIdleQueue();
	if (mOwnerGO)
	{
		AIManager::Instance().UnregisterAIObject(mOwnerGO->GetID());
		AIManager::Instance().UnregisterScriptAIBind(mScript.GetID());
	}
	MessageSystem::Instance().QuitNewsgroup(this, "ENABLE_GAME_CLOCK");
}

int GOCAI::GetID()
{
	if (mOwnerGO) return mOwnerGO->GetID();
	throw Ogre::Exception(Ogre::Exception::ERR_INVALID_STATE, "Called GetID, but AI isn't attached to a game object!", "int GOCAI::GetID()");
}

void GOCAI::SetOwner(GameObject *go)
{
	if (mOwnerGO) AIManager::Instance().UnregisterAIObject(mOwnerGO->GetID());
	mOwnerGO = go;

	if (mOwnerGO)
	{
		UpdatePosition(mOwnerGO->GetGlobalPosition());
		UpdateOrientation(mOwnerGO->GetGlobalOrientation());

		AIManager::Instance().RegisterAIObject(this, mOwnerGO->GetID());
		std::vector<ScriptParam> params;
		params.push_back(ScriptParam(mOwnerGO->GetID()));
		mScript = ScriptSystem::GetInstance().CreateInstance(mScriptFileName, params);
		AIManager::Instance().RegisterScriptAIBind(this, mScript.GetID());
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

void GOCAI::SetProperty(std::string key, ScriptParam prop)
{
	std::map<Ogre::String, ScriptParam>::iterator i = mProperties.find(key);
	if (i != mProperties.end())
	{
		(*i).second = prop;
	}
	else
	{
		mProperties.insert(std::make_pair(key, prop));
	}
}
ScriptParam GOCAI::GetProperty(std::string key)
{
	std::map<Ogre::String, ScriptParam>::iterator i = mProperties.find(key);
	if (i != mProperties.end())
	{
		return (*i).second;
	}
	return ScriptParam(0);
}

void GOCAI::ReceiveObjectMessage(Ogre::SharedPtr<ObjectMsg> msg)
{
	if (msg->mName == "CharacterJumpEnded")
	{
		mCharacterMovementState = mCharacterMovementState & ~CharacterMovement::JUMP;
		BroadcastMovementState();
	}
	if (msg->mName == "CharacterCollisionReport")
	{
		NxU32 collisionFlags = msg->mData.GetInt("collisionFlags");
	}
}

void GOCAI::ReceiveMessage(Msg &msg)
{
	if (msg.mNewsgroup == "ENABLE_GAME_CLOCK")
	{
		bool enable = msg.mData.GetBool("enable");
		if (!enable)
		{
			BroadcastMovementState(0);
		}
	}
}

void GOCAI::ReloadScript()
{
	ClearActionQueue();
	ClearIdleQueue();

	if (mOwnerGO)
	{
		std::vector<ScriptParam> params;
		params.push_back(ScriptParam(mOwnerGO->GetID()));
		AIManager::Instance().UnregisterScriptAIBind(mScript.GetID());
		mScript = ScriptSystem::GetInstance().CreateInstance(mScriptFileName, params);
		AIManager::Instance().RegisterScriptAIBind(this, mScript.GetID());
	}
}

void GOCAI::Create(Ogre::String scriptFile)
{
	mScriptFileName = scriptFile;
}

void GOCAI::CreateFromDataMap(DataMap *parameters)
{
	Create(parameters->GetOgreString("Script"));
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
	Create(mScriptFileName);
}

void GOCAI::AttachToGO(GameObject *go)
{
	go->RemoveComponent(GetFamilyID());
	go->AddComponent(this);
}

};