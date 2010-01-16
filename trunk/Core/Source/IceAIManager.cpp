
#include "IceAIManager.h"
#include "IceGameObject.h"

namespace Ice
{

AIManager::AIManager(void)
{
	mNextID = 0;
	MessageSystem::Instance().JoinNewsgroup(this, "REPARSE_SCRIPTS_POST");
}

AIManager::~AIManager(void)
{
	MessageSystem::Instance().QuitNewsgroup(this, "REPARSE_SCRIPTS_POST");
}

unsigned int AIManager::RegisterAIObject(GOCAI* object)
{
	mAIObjects.insert(std::make_pair<int, GOCAI*>(mNextID, object));
	return mNextID++;
}
void AIManager::UnregisterAIObject(GOCAI* object)
{
	std::map<int, GOCAI*>::iterator i = mAIObjects.find(object->GetID());
	if (i != mAIObjects.end()) mAIObjects.erase(i);
}

GOCAI* AIManager::GetAIByID(unsigned int id)
{
	std::map<int, GOCAI*>::iterator i = mAIObjects.find(id);
	if (i != mAIObjects.end()) return i->second;
	return 0;
}

void AIManager::ReloadScripts()
{
	for (std::map<int, GOCAI*>::iterator i = mAIObjects.begin(); i != mAIObjects.end(); i++)
	{
		i->second->ReloadScript();
	}
}

void AIManager::Clear()
{
	std::map<int, GOCAI*>::iterator i = mAIObjects.begin();
	while (i != mAIObjects.end())
	{
		delete i->second->GetOwner();
		i = mAIObjects.begin();
	}
}

void AIManager::Update(float time)
{
	for (std::map<int, GOCAI*>::iterator i = mAIObjects.begin(); i != mAIObjects.end(); i++)
	{
		i->second->Update(time);
	}
}

void AIManager::ReceiveMessage(Msg &msg)
{
	if (msg.mNewsgroup == "REPARSE_SCRIPTS_POST")
	{
		ReloadScripts();
	}
}

AIManager& AIManager::Instance()
{
	static AIManager TheOneAndOnly;
	return TheOneAndOnly;
};

}