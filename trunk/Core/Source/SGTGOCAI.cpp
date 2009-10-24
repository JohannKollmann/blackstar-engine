
#include "SGTGOCAI.h"
#include "SGTAIManager.h"
#include "SGTScriptSystem.h"

SGTGOCAI::SGTGOCAI(void)
{
	mCharacterMovementState = 0;
	mOwnerGO = 0;
	mActiveState = 0;
	mID = SGTAIManager::Instance().RegisterAIObject(this);
}

SGTGOCAI::~SGTGOCAI(void)
{
	ClearActionQueue();
	ClearIdleQueue();
	SGTAIManager::Instance().UnregisterAIObject(this);
}

void SGTGOCAI::SetOwner(SGTGameObject *go)
{
	mOwnerGO = go;
	UpdatePosition(go->GetGlobalPosition());
	UpdateOrientation(go->GetGlobalOrientation());
}

void SGTGOCAI::AddState(SGTAIState *state)
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

void SGTGOCAI::AddScriptedState(SGTDayCycle *state)
{
	mIdleQueue.push_back(state);
	if (mIdleQueue.size() == 1) (*mIdleQueue.begin())->OnEnter();
}

void SGTGOCAI::ClearActionQueue()
{
	for (std::list<SGTAIState*>::iterator i = mActionQueue.begin(); i != mActionQueue.end(); i++)
	{
		delete (*i);
	}
	mActionQueue.clear();
	delete mActiveState;
	mActiveState = 0;
}

void SGTGOCAI::ClearIdleQueue()
{
	for (std::list<SGTDayCycle*>::iterator i = mIdleQueue.begin(); i != mIdleQueue.end(); i++)
	{
		delete (*i);
	}
	mIdleQueue.clear();
}

void SGTGOCAI::SelectState()
{
	if (mActionQueue.size() > 0)
	{
		std::list<SGTAIState*>::iterator highest = mActionQueue.begin();
		for (std::list<SGTAIState*>::iterator i = mActionQueue.begin(); i != mActionQueue.end(); i++)
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

void SGTGOCAI::Update(float time)
{
	if (mOwnerGO == 0) return;
	if (mActionQueue.size() > 0 || mActiveState)
	{
		if (!mActiveState) SelectState();
		bool finished = mActiveState->OnUpdate(time);
		if (finished)
		{
			mActiveState->OnLeave();
			delete mActiveState;
			mActiveState = 0;
			SelectState();
		}
	}
	else if (mIdleQueue.size() > 0)
	{
		SGTDayCycle *state = *mIdleQueue.begin();
		if (state->OnUpdate(0))
		{
			mIdleQueue.pop_front();
			mIdleQueue.push_back(state);
			(*mIdleQueue.begin())->OnEnter();
		}
	}	
}

void SGTGOCAI::SetProperty(std::string key, SGTScriptParam prop)
{
	std::map<Ogre::String, SGTScriptParam>::iterator i = mProperties.find(key);
	if (i != mProperties.end())
	{
		(*i).second = prop;
	}
	else
	{
		mProperties.insert(std::make_pair(key, prop));
	}
}
SGTScriptParam SGTGOCAI::GetProperty(std::string key)
{
	std::map<Ogre::String, SGTScriptParam>::iterator i = mProperties.find(key);
	if (i != mProperties.end())
	{
		return (*i).second;
	}
	return SGTScriptParam(0);
}

void SGTGOCAI::ReceiveObjectMessage(Ogre::SharedPtr<SGTObjectMsg> msg)
{
	if (msg->mName == "CharacterJumpEnded")
	{
		mCharacterMovementState = mCharacterMovementState & ~SGTCharacterMovement::JUMP;
		BroadcastMovementState();
	}
	if (msg->mName == "CharacterCollisionReport")
	{
		NxU32 collisionFlags = msg->mData.GetFloat("collisionFlags");
	}
}

void SGTGOCAI::ReloadScript()
{
	ClearActionQueue();
	ClearIdleQueue();
	//SGTScriptSystem::GetInstance().KillScript(mScript.GetScriptName());
	std::vector<SGTScriptParam> params;
	params.push_back(SGTScriptParam((int)GetID()));
	mScript = SGTScriptSystem::GetInstance().CreateInstance(mScriptFileName, params);
}

void SGTGOCAI::Create(Ogre::String scriptFile)
{
	mScriptFileName = scriptFile;
	std::vector<SGTScriptParam> params;
	params.push_back(SGTScriptParam((int)GetID()));
	mScript = SGTScriptSystem::GetInstance().CreateInstance(mScriptFileName, params);
}

void SGTGOCAI::CreateFromDataMap(SGTDataMap *parameters)
{
	Create(parameters->GetOgreString("Script"));
}
void SGTGOCAI::GetParameters(SGTDataMap *parameters)
{
	parameters->AddOgreString("Script", mScriptFileName);
}
void SGTGOCAI::GetDefaultParameters(SGTDataMap *parameters)
{
	parameters->AddOgreString("Script", "script.lua");
}

void SGTGOCAI::Save(SGTSaveSystem& mgr)
{
	mgr.SaveAtom("Ogre::String", &mScriptFileName, "Script");
}
void SGTGOCAI::Load(SGTLoadSystem& mgr)
{
	mgr.LoadAtom("Ogre::String", &mScriptFileName);
	Create(mScriptFileName);
}