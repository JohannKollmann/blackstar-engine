
#include "SGTGOCIntern.h"
#include "SGTMain.h"
#include "SGTGameObject.h"

SGTGOCWaypoint::SGTGOCWaypoint(void)
{
}

SGTGOCWaypoint::~SGTGOCWaypoint(void)
{
	while (mConnectedWaypoints.size() > 0)
	{
		(*mConnectedWaypoints.begin())->DisconnectWaypoint(this);
	}
}

Ogre::ManualObject* SGTGOCWaypoint::CreateEditorLine(SGTGOCWaypoint *waypoint)
{
	Ogre::ManualObject* WPLine = SGTMain::Instance().GetOgreSceneMgr()->createManualObject("WaynetLine_" + mOwnerGO->GetName() + waypoint->GetOwner()->GetName());
	Ogre::SceneNode* WPLineNode = SGTMain::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode("WaynetLine_" + mOwnerGO->GetName() + waypoint->GetOwner()->GetName());
	WPLine->begin("WPLine", Ogre::RenderOperation::OT_LINE_LIST);
	WPLine->position(GetOwner()->GetGlobalPosition());
	WPLine->position(waypoint->GetOwner()->GetGlobalPosition());
	WPLine->end();
	WPLineNode->attachObject(WPLine);
	mLines.push_back(LineNeighborBind(WPLine, waypoint));
	return WPLine;
}

void SGTGOCWaypoint::AddLine(Ogre::ManualObject* line, SGTGOCWaypoint *waypoint)
{
	mLines.push_back(LineNeighborBind(line, waypoint));
}
void SGTGOCWaypoint::RemoveLine(Ogre::ManualObject* line)
{
	for (std::list<LineNeighborBind>::iterator i = mLines.begin(); i != mLines.end(); i++)
	{
		if ((*i).mLine == line)
		{
			mLines.erase(i);
			return;
		}
	}
}

void SGTGOCWaypoint::ConnectWaypoint(SGTGOCWaypoint *waypoint, bool notify_only)
{
	if (HasConnectedWaypoint(waypoint)) return;
	mConnectedWaypoints.push_back(waypoint);
	if (!notify_only)
	{
		waypoint->ConnectWaypoint(this, true);
		if (mRenderEditorVisuals)
		{
			waypoint->AddLine(CreateEditorLine(waypoint), this);
		}
	}
}

void SGTGOCWaypoint::DisconnectWaypoint(SGTGOCWaypoint *waypoint, bool notify_only)
{
	if (!HasConnectedWaypoint(waypoint)) return;
	if (mRenderEditorVisuals)
	{
		for (std::list<LineNeighborBind>::iterator i = mLines.begin(); i != mLines.end(); i++)
		{
			if ((*i).mNeighbor == waypoint)
			{
				if (!notify_only)
				{
					SGTMain::Instance().GetOgreSceneMgr()->destroySceneNode((*i).mLine->getParentSceneNode());
					SGTMain::Instance().GetOgreSceneMgr()->destroyManualObject((*i).mLine);
				}
				mLines.erase(i);
				break;
			}
		}
	}
	if (!notify_only) waypoint->DisconnectWaypoint(this, true);
	mConnectedWaypoints.remove(waypoint);
}

void SGTGOCWaypoint::UpdatePosition(Ogre::Vector3 position)
{
	if (mRenderEditorVisuals)
	{
		//Update Lines
		for (std::list<LineNeighborBind>::iterator i = mLines.begin(); i != mLines.end(); i++)
		{
			(*i).mLine->clear();
			(*i).mLine->begin("WPLine", Ogre::RenderOperation::OT_LINE_LIST);
			(*i).mLine->position(GetOwner()->GetGlobalPosition());
			(*i).mLine->position((*i).mNeighbor->GetOwner()->GetGlobalPosition());
			(*i).mLine->end();
		}
	}
}

void SGTGOCWaypoint::ShowEditorVisual(bool show)
{
	if (show == mRenderEditorVisuals) return;
	mRenderEditorVisuals = show;
	if (show)
	{
		for (std::list<SGTGOCWaypoint*>::iterator i = mConnectedWaypoints.begin(); i != mConnectedWaypoints.end(); i++)
		{
			(*i)->AddLine(CreateEditorLine((*i)), this);
		}
	}
	else
	{
		for (std::list<LineNeighborBind>::iterator i = mLines.begin(); i != mLines.end(); i++)
		{
			(*i).mNeighbor->RemoveLine((*i).mLine);
			SGTMain::Instance().GetOgreSceneMgr()->destroySceneNode((*i).mLine->getParentSceneNode());
			SGTMain::Instance().GetOgreSceneMgr()->destroyManualObject((*i).mLine);
		}
		mLines.clear();
	}
}

bool SGTGOCWaypoint::HasConnectedWaypoint(SGTGOCWaypoint *waypoint)
{
	for (std::list<SGTGOCWaypoint*>::iterator i = mConnectedWaypoints.begin(); i != mConnectedWaypoints.end(); i++)
	{
		if ((*i) == waypoint) return true;
	}
	return false;
}

void SGTGOCWaypoint::Save(SGTSaveSystem& mgr)
{
	mgr.SaveAtom("std::list<SGTSaveable*>", (void*)(&mConnectedWaypoints), "mComponents");
}
void SGTGOCWaypoint::Load(SGTLoadSystem& mgr)
{
	mgr.LoadAtom("std::list<SGTSaveable*>", (void*)(&mConnectedWaypoints));
}