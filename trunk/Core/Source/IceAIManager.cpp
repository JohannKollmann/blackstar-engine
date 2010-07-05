
#include "IceAIManager.h"
#include "IceGameObject.h"

namespace Ice
{

	AIManager::AIManager(void)
	{
		MessageSystem::Instance().JoinNewsgroup(this, "REPARSE_SCRIPTS_POST");
		mNavigationMesh = new NavigationMesh();
		mLoadWayMeshAsObjects = false;
	}

	AIManager::~AIManager(void)
	{
		MessageSystem::Instance().QuitNewsgroup(this, "REPARSE_SCRIPTS_POST");
	}

	void AIManager::RegisterWaypoint(GOCWaypoint *waypoint)
	{
		mWaypoints.push_back(waypoint);
	}
	void AIManager::UnregisterWaypoint(GOCWaypoint *waypoint)
	{
		for (std::vector<GOCWaypoint*>::iterator i = mWaypoints.begin(); i != mWaypoints.end(); i++)
		{
			if ((*i) == waypoint)
			{
				mWaypoints.erase(i);
				return;
			}
		}
	}
	GOCWaypoint* AIManager::GetWPByName(Ogre::String name)
	{
		for (std::vector<GOCWaypoint*>::iterator i = mWaypoints.begin(); i != mWaypoints.end(); i++)
		{
			if ((*i)->GetOwner()->GetName() == name) return (*i);
		}
		Ogre::LogManager::getSingleton().logMessage("Error: Waypoint " + name + " doesn't exist!");
		return 0;
	}
	void AIManager::FindPath(Ogre::Vector3 origin, Ogre::String targetWP, std::vector<AStarNode3D*> &oPath)
	{
		GOCWaypoint *target = GetWPByName(targetWP);
		if (!target)
		{
			Ogre::LogManager::getSingleton().logMessage("Error in AIManager::FindPath: Target Point '" + targetWP + "' does not exist!");
			return;
		}
		mNavigationMesh->ShortestPath(origin, target->GetOwner()->GetGlobalPosition(), oPath);
	}

	void AIManager::RegisterAIObject(GOCAI* object, int id)
	{
		mAIObjects.insert(std::make_pair<int, GOCAI*>(id, object));
	}
	void AIManager::UnregisterAIObject(int id)
	{
		std::map<int, GOCAI*>::iterator i = mAIObjects.find(id);
		if (i != mAIObjects.end()) mAIObjects.erase(i);
	}

	void AIManager::RegisterScriptAIBind(GOCAI* object, int scriptID)
	{
		mScriptAIBinds.insert(std::make_pair<int, GOCAI*>(scriptID, object));
	}
	void AIManager::UnregisterScriptAIBind(int scriptID)
	{
		std::map<int, GOCAI*>::iterator i = mScriptAIBinds.find(scriptID);
		if (i != mScriptAIBinds.end()) mScriptAIBinds.erase(i);
	}

	GOCAI* AIManager::GetAIByID(int id)
	{
		std::map<int, GOCAI*>::iterator i = mAIObjects.find(id);
		if (i != mAIObjects.end()) return i->second;
		return 0;
	}

	GOCAI* AIManager::GetAIByScriptID(int scriptID)
	{
		std::map<int, GOCAI*>::iterator i = mScriptAIBinds.find(scriptID);
		if (i != mScriptAIBinds.end()) return i->second;
		return 0;
	}

	NavigationMesh* AIManager::GetNavigationMesh()
	{
		return mNavigationMesh;
	}

	void AIManager::ReloadScripts()
	{
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
		if (msg.type == "REPARSE_SCRIPTS_POST")
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