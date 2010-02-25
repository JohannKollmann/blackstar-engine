
#pragma once

#include "IceIncludes.h"
#include "Ice3D.h"
#include "IceGOComponent.h"

namespace Ice
{

class GOCWaypoint;

struct LineNeighborBind
{
	Ogre::ManualObject* mLine;
	GOCWaypoint* mNeighbor;
	LineNeighborBind(Ogre::ManualObject* line, GOCWaypoint* waypoint) { mLine = line; mNeighbor = waypoint; }
};

class WPEdge
{
public:
	GOCWaypoint *mWP;
	GOCWaypoint *mNeighbor;
	float mCost;
	float mCostOffset;		//for Heuristik
	
	bool operator < (const WPEdge &rhs) const
	{
		return ((mCost + mCostOffset) < (rhs.mCost + mCostOffset));
	}
	bool operator > (const WPEdge &rhs) const
	{
		return ((mCost + mCostOffset) > (rhs.mCost + mCostOffset));
	}

};

class DllExport GOCWaypoint : public GOComponent
{
private:
	std::list<GOCWaypoint*> mConnectedWaypoints;
	//Visualisation
	std::list<LineNeighborBind> mLines;


	Ogre::ManualObject* CreateEditorLine(GOCWaypoint *waypoint);

public:
	GOCWaypoint(void);
	~GOCWaypoint(void);

	goc_id_family& GetFamilyID() const { static std::string name = "Waypoint"; return name; }
	GOComponent::goc_id_type& GetComponentID() const { static std::string name = "Waypoint"; return name; }

	void ConnectWaypoint(GOCWaypoint *waypoint, bool notify_only = false);
	void DisconnectWaypoint(GOCWaypoint *waypoint, bool notify_only = false);
	bool HasConnectedWaypoint(GOCWaypoint *waypoint);
	void AddLine(Ogre::ManualObject* line, GOCWaypoint *waypoint);
	void RemoveLine(Ogre::ManualObject* line);

	void GetNeighbors(std::vector<WPEdge> *edges, Ogre::Vector3 targetPos);
	bool HasLine(GOCWaypoint *waypoint);

	void ShowEditorVisual(bool show);

	void UpdatePosition(Ogre::Vector3 position);
	Ogre::Vector3 GetPosition();	//AI soll sauber getrennt sein von Objektsystem
	Ogre::Quaternion GetOrientation();

	void Save(LoadSave::SaveSystem& mgr);
	void Load(LoadSave::LoadSystem& mgr);
	std::string& TellName() { static std::string name = "Waypoint"; return name; };
	static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "Waypoint"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
	static LoadSave::Saveable* NewInstance() { return new GOCWaypoint; };
};

};