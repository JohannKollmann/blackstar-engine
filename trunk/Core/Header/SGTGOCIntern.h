
#pragma once

#include "SGTIncludes.h"
#include "SGTGOComponent.h"

class SGTGOCWaypoint;

struct LineNeighborBind
{
	Ogre::ManualObject* mLine;
	SGTGOCWaypoint* mNeighbor;
	LineNeighborBind(Ogre::ManualObject* line, SGTGOCWaypoint* waypoint) { mLine = line; mNeighbor = waypoint; }
};

class SGTDllExport SGTGOCWaypoint : public SGTGOComponent
{
private:
	std::list<SGTGOCWaypoint*> mConnectedWaypoints;
	//Visualisation
	std::list<LineNeighborBind> mLines;

	Ogre::ManualObject* CreateEditorLine(SGTGOCWaypoint *waypoint);

public:
	SGTGOCWaypoint(void);
	~SGTGOCWaypoint(void);

	goc_id_family& GetFamilyID() const { static std::string name = "Waypoint"; return name; }
	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "Waypoint"; return name; }

	void ConnectWaypoint(SGTGOCWaypoint *waypoint, bool notify_only = false);
	void DisconnectWaypoint(SGTGOCWaypoint *waypoint, bool notify_only = false);
	bool HasConnectedWaypoint(SGTGOCWaypoint *waypoint);
	void AddLine(Ogre::ManualObject* line, SGTGOCWaypoint *waypoint);
	void RemoveLine(Ogre::ManualObject* line);

	void ShowEditorVisual(bool show);

	void UpdatePosition(Ogre::Vector3 position);

	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	std::string& TellName() { static std::string name = "Waypoint"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "Waypoint"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTSaveable* NewInstance() { return new SGTGOCWaypoint; };
};
