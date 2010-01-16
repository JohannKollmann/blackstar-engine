
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
	mActiveState = 0;
	mID = AIManager::Instance().RegisterAIObject(this);
	MessageSystem::Instance().JoinNewsgroup(this, "ENABLE_GAME_CLOCK");
}

GOCAI::~GOCAI(void)
{
	ClearActionQueue();
	ClearIdleQueue();
	AIManager::Instance().UnregisterAIObject(this);
	MessageSystem::Instance().QuitNewsgroup(this, "ENABLE_GAME_CLOCK");
}

void GOCAI::SetOwner(GameObject *go)
{
	mOwnerGO = go;
	UpdatePosition(go->GetGlobalPosition());
	UpdateOrientation(go->GetGlobalOrientation());
}

void GOCAI::AddState(AIState *state)
{
	mActionQueue.push_back(state);
	/*if (!mActiveState) 
	{
		mActiveState = state;
		return;
	}
	if (state->GetPriority() > mActiveState->GetPriority())
	{
		mActionQueue.push_front(mActiveState);
		mActiveState = state;
		return;
	}*/
}

void GOCAI::AddScriptedState(DayCycle *state)
{
	mIdleQueue.push_back(state);
	if (mIdleQueue.size() == 1) (*mIdleQueue.begin())->OnEnter();
}

void GOCAI::ClearActionQueue()
{
	for (std::list<AIState*>::iterator i = mActionQueue.begin(); i != mActionQueue.end(); i++)
	{
		delete (*i);
	}
	mActionQueue.clear();
	delete mActiveState;
	mActiveState = 0;
}

void GOCAI::ClearIdleQueue()
{
	for (std::list<DayCycle*>::iterator i = mIdleQueue.begin(); i != mIdleQueue.end(); i++)
	{
		delete (*i);
	}
	mIdleQueue.clear();
}

void GOCAI::SelectState()
{
	if (mActionQueue.size() > 0)
	{
		std::list<AIState*>::iterator highest = mActionQueue.begin();
		for (std::list<AIState*>::iterator i = mActionQueue.begin(); i != mActionQueue.end(); i++)
		{
			if ((*i)->GetPriority() > (*highest)->GetPriority())
			{
				highest = i;
			}
		}
		mActiveState = (*highest);
		mActionQueue.erase(highest);
		mActiveState->OnEnter();
	}
	else mActiveState = 0;
}

void GOCAI::Update(float time)
{
	if (mOwnerGO == 0) return;
	if (mActiveState)
	{
		bool finished = mActiveState->OnUpdate(time);
		if (finished)
		{
			mActiveState->OnLeave();
			delete mActiveState;
			mActiveState = 0;
			SelectState();
		}
	}
	else if (mActionQueue.size() > 0)
	{
		SelectState();
	}
	else if (mIdleQueue.size() > 0)
	{
		DayCycle *state = *mIdleQueue.begin();
		if (state->OnUpdate(0))
		{
			mIdleQueue.pop_front();
			mIdleQueue.push_back(state);
			(*mIdleQueue.begin())->OnEnter();
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
		NxU32 collisionFlags = msg->mData.GetFloat("collisionFlags");
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
	std::vector<ScriptParam> params;
	params.push_back(ScriptParam((int)GetID()));
	mScript = ScriptSystem::GetInstance().CreateInstance(mScriptFileName, params);
}

void GOCAI::Create(Ogre::String scriptFile)
{
	mScriptFileName = scriptFile;
	std::vector<ScriptParam> params;
	params.push_back(ScriptParam((int)GetID()));
	mScript = ScriptSystem::GetInstance().CreateInstance(mScriptFileName, params);
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