
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

	void GOCWaypoint::ShowEditorVisual(bool show)
	{
	}

	void GOCWaypoint::Save(LoadSave::SaveSystem& mgr)
	{
		//mgr.SaveAtom("std::list<Saveable*>", (void*)(&mConnectedWaypoints), "mConnectedWaypoints");
	}
	void GOCWaypoint::Load(LoadSave::LoadSystem& mgr)
	{
		std::list<LoadSave::Saveable*> dummy;
		mgr.LoadAtom("std::list<Saveable*>", (void*)(&dummy));
	}

};