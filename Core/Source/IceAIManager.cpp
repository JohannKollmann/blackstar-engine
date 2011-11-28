
#include "IceAIManager.h"
#include "IceGameObject.h"
#include "IceObjectMessageIDs.h"
#include "IceMain.h"
#include "IceUtils.h"
#include "IceCollisionCallback.h"

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

	void AIManager::FindPath(const Ogre::Vector3 &origin, const Ogre::Vector3 &target, std::vector<AStarNode3D*> &oPath)
	{
		mNavigationMesh->ShortestPath(origin, target, oPath);
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

	float AIManager::GetAmbientLightBrightness(const Ogre::Vector3 &position, const Ogre::Vector3 &normal)
	{
		Ogre::ColourValue ambientCol = Main::Instance().GetOgreSceneMgr()->getAmbientLight();
		float brightness = std::max(std::max(ambientCol.r, ambientCol.g), ambientCol.b);
		float ao = Utils::ComputeAO(position, normal, CollisionGroups::DYNAMICBODY|CollisionGroups::STATICBODY);
		//IceNote("Ambient light brightness: " + Ogre::StringConverter::toString(brightness) + " AO: " + Ogre::StringConverter::toString(ao));
		return brightness * ao;
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