
#pragma once

#include "IceIncludes.h"
#include "IceGOComponent.h"
#include "IceGOCView.h"

namespace Ice
{

	class DllExport GOCWaypoint : public GOCEditorVisualised
	{

	public:
		GOCWaypoint(void);
		~GOCWaypoint(void);

		goc_id_family& GetFamilyID() const { static std::string name = "Waypoint"; return name; }
		GOComponent::goc_id_type& GetComponentID() const { static std::string name = "Waypoint"; return name; }

		void UpdatePosition(Ogre::Vector3 position);

		Ogre::String GetEditorVisualMeshName() { return "Editor_Waypoint.mesh"; }

		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		std::string& TellName() { static std::string name = "Waypoint"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "Waypoint"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCWaypoint; };
	};

};