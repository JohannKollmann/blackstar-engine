
#include "SGTGOCIntern.h"
#include "SGTMain.h"
#include "SGTGameObject.h"
#include "SGTPathfinder.h"

SGTGOCWaypoint::SGTGOCWaypoint(void)
{
	SGTPathfinder::Instance().RegisterWaypoint(this);
	mRenderEditorVisuals = false;
}

SGTGOCWaypoint::~SGTGOCWaypoint(void)
{
	SGTPathfinder::Instance().UnregisterWaypoint(this);
	/*while (mConnectedWaypoints.size() > 0)
	{
		(*mConnectedWaypoints.begin())->DisconnectWaypoint(this);
	}*/
	for (std::list<LineNeighborBind>::iterator i = mLines.begin(); i != mLines.end(); i++)
	{
		SGTMain::Instance().GetOgreSceneMgr()->destroySceneNode((*i).mLine->getParentSceneNode());
		SGTMain::Instance().GetOgreSceneMgr()->destroyManualObject((*i).mLine);
		(*i).mNeighbor->DisconnectWaypoint(this, true);
	}
}

Ogre::ManualObject* SGTGOCWaypoint::CreateEditorLine(SGTGOCWaypoint *waypoint)
{
	Ogre::ManualObject* WPLine = SGTMain::Instance().GetOgreSceneMgr()->createManualObject("WaynetLine_" + mOwnerGO->GetName() + waypoint->GetOwner()->GetName());
	Ogre::SceneNode* WPLineNode = SGTMain::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode("WaynetLine_" + mOwnerGO->GetName() + waypoint->GetOwner()->GetName());
	WPLine->begin("WPLine", Ogre::RenderOperation::OT_LINE_LIST);
	WPLine->position(GetOwner()->GetGlobalPosition() + Ogre::Vector3(0,0.2f,0));
	WPLine->position(waypoint->GetOwner()->GetGlobalPosition() + Ogre::Vector3(0,0.2f,0));
	WPLine->end();
	WPLine->setCastShadows(false);
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

bool SGTGOCWaypoint::HasLine(SGTGOCWaypoint *waypoint)
{
	for (std::list<LineNeighborBind>::iterator i = mLines.begin(); i != mLines.end(); i++)
	{
		if ((*i).mNeighbor == waypoint)
		{
			return true;
		}
	}
	return false;
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
			if (!HasLine((*i))) (*i)->AddLine(CreateEditorLine((*i)), this);
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

Ogre::Vector3 SGTGOCWaypoint::GetPosition()
{
	return GetOwner()->GetGlobalPosition();
}
Ogre::Quaternion SGTGOCWaypoint::GetOrientation()
{
	return GetOwner()->GetGlobalOrientation();
}

void SGTGOCWaypoint::GetNeighbors(std::list<WPEdge> *edges, Ogre::Vector3 targetPos)
{
	for (std::list<SGTGOCWaypoint*>::iterator i = mConnectedWaypoints.begin(); i != mConnectedWaypoints.end(); i++)
	{
		WPEdge edge;
		edge.mCost = GetPosition().distance((*i)->GetPosition());
		edge.mWP = this;
		edge.mNeighbor = (*i);
		edge.mCostOffset = (*i)->GetPosition().distance(targetPos);
		edges->push_back(edge);
	}
}

void SGTGOCWaypoint::Save(SGTSaveSystem& mgr)
{
	mgr.SaveAtom("std::list<SGTSaveable*>", (void*)(&mConnectedWaypoints), "mConnectedWaypoints");
}
void SGTGOCWaypoint::Load(SGTLoadSystem& mgr)
{
	mgr.LoadAtom("std::list<SGTSaveable*>", (void*)(&mConnectedWaypoints));
}