
#pragma once

#include "IceIncludes.h"
#include "IceGOComponent.h"
#include "IceGOCView.h"

namespace Ice
{

	/**
	This component really does almost nothing, only adds a nice editor visual to the object.
	*/
	class DllExport GOCWaypoint : public GOCEditorVisualised, public GOCStaticEditorInterface
	{

	public:
		GOCWaypoint() {}
		~GOCWaypoint() {}

		GOComponent::FamilyID& GetFamilyID() const { static std::string name = "Waypoint"; return name; }
		GOComponent::TypeID& GetComponentID() const { static std::string name = "Waypoint"; return name; }

		BEGIN_GOCEDITORINTERFACE(GOCWaypoint, "Waypoint")
		END_GOCEDITORINTERFACE

		Ogre::String GetEditorVisualMeshName() { return "Editor_Waypoint.mesh"; }

		void Save(LoadSave::SaveSystem& mgr) {}
		void Load(LoadSave::LoadSystem& mgr) {}
		std::string& TellName() { static std::string name = "Waypoint"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "Waypoint"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCWaypoint; };
	};

};