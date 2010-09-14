
#include "IceGOCWaypoint.h"
#include "IceMain.h"
#include "IceGameObject.h"
#include "IceAIManager.h"

namespace Ice
{

	GOCWaypoint::GOCWaypoint(void)
	{
		AIManager::Instance().RegisterWaypoint(this);
	}

	GOCWaypoint::~GOCWaypoint(void)
	{
		AIManager::Instance().UnregisterWaypoint(this);
	}

	void GOCWaypoint::UpdatePosition(Ogre::Vector3 position)
	{
	}

	void GOCWaypoint::Save(LoadSave::SaveSystem& mgr)
	{
	}
	void GOCWaypoint::Load(LoadSave::LoadSystem& mgr)
	{
	}

};