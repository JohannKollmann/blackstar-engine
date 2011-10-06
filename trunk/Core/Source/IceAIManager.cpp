
#include "IceAIManager.h"
#include "IceGameObject.h"

namespace Ice
{

	AIManager::AIManager(void)
	{
		JoinNewsgroup(GlobalMessageIDs::REPARSE_SCRIPTS_POST);
		JoinNewsgroup(GlobalMessageIDs::UPDATE_INDEPENDANT);
		mNavigationMesh = ICE_NEW NavigationMesh();
	}

	AIManager::~AIManager(void)
	{
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
		Log::Instance().LogMessage("Error: Waypoint " + name + " doesn't exist!");
		return 0;
	}
	void AIManager::FindPath(Ogre::Vector3 origin, Ogre::String targetWP, std::vector<AStarNode3D*> &oPath)
	{
		GOCWaypoint *target = GetWPByName(targetWP);
		if (!target)
		{
			Log::Instance().LogMessage("Error in AIManager::FindPath: Target Point '" + targetWP + "' does not exist!");
			return;
		}
		mNavigationMesh->ShortestPath(origin, target->GetOwner()->GetGlobalPosition(), oPath);
	}

	void AIManager::RegisterAIObject(GOCAI* object)
	{
		mAIObjects.push_back(object);
	}
	void AIManager::UnregisterAIObject(GOCAI* object)
	{
		for (auto i = mAIObjects.begin(); i != mAIObjects.end(); i++)
		{
			if ((*i) == object)
			{
				mAIObjects.erase(i);
				return;
			}
		}
		IceWarning("AI Object does not exist");
	}

	NavigationMesh* AIManager::GetNavigationMesh()
	{
		return mNavigationMesh;
	}
	void AIManager::SetNavigationMesh(NavigationMesh *mesh)
	{
		if (mNavigationMesh) ICE_DELETE mNavigationMesh;
		mNavigationMesh = mesh;
	}

	void AIManager::Clear()
	{
		mAIObjects.clear();
	}
	void AIManager::Shutdown()
	{
		ICE_DELETE mNavigationMesh;
		mNavigationMesh = nullptr;
	}

	void AIManager::Update(float time)
	{
		for (auto i = mAIObjects.begin(); i != mAIObjects.end(); i++)
		{
			(*i)->Update(time);
		}
	}

	void AIManager::ReceiveMessage(Msg &msg)
	{
		if (msg.typeID == GlobalMessageIDs::UPDATE_INDEPENDANT)
			Update(msg.params.GetValue<float>(0));
	}

	AIManager& AIManager::Instance()
	{
		static AIManager TheOneAndOnly;
		return TheOneAndOnly;
	};

}