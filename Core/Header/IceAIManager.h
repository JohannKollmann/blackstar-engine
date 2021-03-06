#pragma once

#include "IceIncludes.h"
#include "IceGOCAI.h"
#include <list>
#include "IceMessageSystem.h"
#include "IceNavigationMesh.h"

/*
Verwaltet AI Objekte, vergibt IDs und entscheidet, was aktualisiert wird und was nicht.
*/

namespace Ice
{

class DllExport AIManager : public IndependantMessageListener
{
	friend class SceneManager;

private:

	//maps object ID <=> ai object
	std::vector<GOCAI*> mAIObjects;

	std::vector<Ogre::Light*> mLights;

	NavigationMesh *mNavigationMesh;

public:
	AIManager(void);
	~AIManager(void);

	void FindPath(const Ogre::Vector3 &origin, const Ogre::Vector3 &target, std::vector<AStarNode3D*> &oPath);

	void RegisterAIObject(GOCAI* object);
	void UnregisterAIObject(GOCAI* object);

	NavigationMesh* GetNavigationMesh();
	void SetNavigationMesh(NavigationMesh *mesh);

	//Used for Npcs see sense.
	void GetLights(std::vector<Ogre::Light*> &lights);
	void RegisterLight(Ogre::Light *light);
	void UnregisterLight(Ogre::Light *light);

	void NotifySound(Ogre::String soundName, const Ogre::Vector3 &position, float range, float loudness);

	void Clear();
	void Shutdown();

	void ReceiveMessage(Msg &msg);
	void Update(float time);

	///Retrieves the current ambient light as a single brightness float value, used in SeeSense.
	float GetAmbientLightBrightness(const Ogre::Vector3 &position, const Ogre::Vector3 &normal);

	//Singleton
	static AIManager& Instance();

};

};