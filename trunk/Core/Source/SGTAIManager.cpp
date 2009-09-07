
#include "SGTAIManager.h"

SGTAIManager::SGTAIManager(void)
{
	mNextID = 0;
	SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
}

SGTAIManager::~SGTAIManager(void)
{
	SGTMessageSystem::Instance().QuitNewsgroup(this, "UPDATE_PER_FRAME");
}

unsigned int SGTAIManager::RegisterAIObject(SGTGOCAI* object)
{
	mAIObjects.push_back(object);
	return mNextID++;
}
void SGTAIManager::UnregisterAIObject(SGTGOCAI* object)
{
	mAIObjects.remove(object);
}

SGTGOCAI* SGTAIManager::GetAIByID(unsigned int id)
{
	for (std::list<SGTGOCAI*>::iterator i = mAIObjects.begin(); i != mAIObjects.end(); i++)
	{
		if ((*i)->GetID() == id) return (*i);
	}
	return 0;
}

void SGTAIManager::ReloadScripts()
{
	for (std::list<SGTGOCAI*>::iterator i = mAIObjects.begin(); i != mAIObjects.end(); i++)
	{
		(*i)->ReloadScript();
	}
}

void SGTAIManager::Clear()
{
	std::list<SGTGOCAI*>::iterator i = mAIObjects.begin();
	while (i != mAIObjects.end())
	{
		delete (*i)->GetOwner();
		i = mAIObjects.begin();
	}
}

void SGTAIManager::ReceiveMessage(SGTMsg &msg)
{
	if (msg.mNewsgroup == "UPDATE_PER_FRAME")
	{
		float time = msg.mData.GetFloat("TIME");
		for (std::list<SGTGOCAI*>::iterator i = mAIObjects.begin(); i != mAIObjects.end(); i++)
		{
			(*i)->Update(time);
		}
	}
}

SGTAIManager& SGTAIManager::Instance()
{
	static SGTAIManager TheOneAndOnly;
	return TheOneAndOnly;
};