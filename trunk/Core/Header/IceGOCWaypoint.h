
#pragma once

#include "IceIncludes.h"
#include "IceGOComponent.h"

namespace Ice
{

	class DllExport GOCWaypoint : public GOComponent
	{

	public:
		GOCWaypoint(void);
		~GOCWaypoint(void);

		goc_id_family& GetFamilyID() const { static std::string name = "Waypoint"; return name; }
		GOComponent::goc_id_type& GetComponentID() const { static std::string name = "Waypoint"; return name; }

		void ShowEditorVisual(bool show);

		void UpdatePosition(Ogre::Vector3 position);

		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		std::string& TellName() { static std::string name = "Waypoint"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "Waypoint"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return ICE_NEW GOCWaypoint; };
	};

};