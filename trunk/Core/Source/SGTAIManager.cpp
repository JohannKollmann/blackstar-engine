
#include "SGTAIManager.h"
#include "SGTGameObject.h"

SGTAIManager::SGTAIManager(void)
{
	mNextID = 0;
	SGTMessageSystem::Instance().JoinNewsgroup(this, "REPARSE_SCRIPTS");
}

SGTAIManager::~SGTAIManager(void)
{
	SGTMessageSystem::Instance().QuitNewsgroup(this, "REPARSE_SCRIPTS");
}

unsigned int SGTAIManager::RegisterAIObject(SGTGOCAI* object)
{
	mAIObjects.insert(std::make_pair<int, SGTGOCAI*>(mNextID, object));
	return mNextID++;
}
void SGTAIManager::UnregisterAIObject(SGTGOCAI* object)
{
	std::map<int, SGTGOCAI*>::iterator i = mAIObjects.find(object->GetID());
	if (i != mAIObjects.end()) mAIObjects.erase(i);
}

SGTGOCAI* SGTAIManager::GetAIByID(unsigned int id)
{
	std::map<int, SGTGOCAI*>::iterator i = mAIObjects.find(id);
	if (i != mAIObjects.end()) return i->second;
	return 0;
}

void SGTAIManager::ReloadScripts()
{
	for (std::map<int, SGTGOCAI*>::iterator i = mAIObjects.begin(); i != mAIObjects.end(); i++)
	{
		i->second->ReloadScript();
	}
}

void SGTAIManager::Clear()
{
	std::map<int, SGTGOCAI*>::iterator i = mAIObjects.begin();
	while (i != mAIObjects.end())
	{
		delete i->second->GetOwner();
		i = mAIObjects.begin();
	}
}

void SGTAIManager::Update(float time)
{
	for (std::map<int, SGTGOCAI*>::iterator i = mAIObjects.begin(); i != mAIObjects.end(); i++)
	{
		i->second->Update(time);
	}
}

void SGTAIManager::ReceiveMessage(SGTMsg &msg)
{
	if (msg.mNewsgroup == "REPARSE_SCRIPTS")
	{
		ReloadScripts();
	}
}

SGTAIManager& SGTAIManager::Instance()
{
	static SGTAIManager TheOneAndOnly;
	return TheOneAndOnly;
};