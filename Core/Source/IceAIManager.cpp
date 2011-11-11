
#include "IceAIManager.h"
#include "IceGameObject.h"
#include "IceObjectMessageIDs.h"

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

	void AIManager::NotifySound(Ogre::String soundName, const Ogre::Vector3 &position, float range, float loudness)
	{
		float squaredRange = range*range;
		ITERATE(i, mAIObjects)
		{
			GameObjectPtr object = (*i)->GetOwner();
			if (!object.get()) return;
			float squaredDist = object->GetGlobalPosition().squaredDistance(position);
			if (squaredDist < squaredRange)
			{
				Msg msg;
				msg.typeID = ObjectMessageIDs::AI_HEAR;
				msg.params.AddOgreString("soundName", soundName);
				msg.params.AddFloat("loudness", (Ogre::Math::Sqrt(squaredDist) / squaredRange) * loudness);
				object->BroadcastObjectMessage(msg);
			}
		}
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

	void AIManager::GetLights(std::vector<Ogre::Light*> &lights)
	{
		lights.assign(mLights.begin(), mLights.end());
	}
	void AIManager::RegisterLight(Ogre::Light *light)
	{
		UnregisterLight(light);
		mLights.push_back(light);
	}
	void AIManager::UnregisterLight(Ogre::Light *light)
	{
		ITERATE(i, mLights)
		{
			if (*i == light)
			{
				mLights.erase(i);
				return;
			}
		}
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

	float AIManager::GetAmbientLightBrightness()
	{
		//TODO
		return 0.5f;
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