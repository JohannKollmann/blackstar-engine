
#include "IceGOCIntern.h"
#include "IceMain.h"
#include "IceGameObject.h"
#include "IceAIManager.h"

namespace Ice
{

	GOCWaypoint::GOCWaypoint(void)
	{
		AIManager::Instance().RegisterWaypoint(this);
		mRenderEditorVisuals = false;
	}

	GOCWaypoint::~GOCWaypoint(void)
	{
		AIManager::Instance().UnregisterWaypoint(this);
		/*while (mConnectedWaypoints.size() > 0)
		{
			(*mConnectedWaypoints.begin())->DisconnectWaypoint(this);
		}*/
		for (std::list<LineNeighborBind>::iterator i = mLines.begin(); i != mLines.end(); i++)
		{
			Main::Instance().GetOgreSceneMgr()->destroySceneNode((*i).mLine->getParentSceneNode());
			Main::Instance().GetOgreSceneMgr()->destroyManualObject((*i).mLine);
			(*i).mNeighbor->DisconnectWaypoint(this, true);
		}
	}

	Ogre::ManualObject* GOCWaypoint::CreateEditorLine(GOCWaypoint *waypoint)
	{
		Ogre::ManualObject* WPLine = Main::Instance().GetOgreSceneMgr()->createManualObject("WaynetLine_" + mOwnerGO->GetIDStr() + waypoint->GetOwner()->GetIDStr());
		Ogre::SceneNode* WPLineNode = Main::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode("WaynetLine_" + mOwnerGO->GetIDStr() + waypoint->GetOwner()->GetIDStr());
		WPLine->begin("WPLine", Ogre::RenderOperation::OT_LINE_LIST);
		WPLine->position(GetOwner()->GetGlobalPosition() + Ogre::Vector3(0,0.2f,0));
		WPLine->position(waypoint->GetOwner()->GetGlobalPosition() + Ogre::Vector3(0,0.2f,0));
		WPLine->end();
		WPLine->setCastShadows(false);
		WPLineNode->attachObject(WPLine);
		mLines.push_back(LineNeighborBind(WPLine, waypoint));
		return WPLine;
	}

	void GOCWaypoint::AddLine(Ogre::ManualObject* line, GOCWaypoint *waypoint)
	{
		mLines.push_back(LineNeighborBind(line, waypoint));
	}
	void GOCWaypoint::RemoveLine(Ogre::ManualObject* line)
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

	void GOCWaypoint::ConnectWaypoint(GOCWaypoint *waypoint, bool notify_only)
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

	void GOCWaypoint::DisconnectWaypoint(GOCWaypoint *waypoint, bool notify_only)
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
						Main::Instance().GetOgreSceneMgr()->destroySceneNode((*i).mLine->getParentSceneNode());
						Main::Instance().GetOgreSceneMgr()->destroyManualObject((*i).mLine);
					}
					mLines.erase(i);
					break;
				}
			}
		}
		if (!notify_only) waypoint->DisconnectWaypoint(this, true);
		mConnectedWaypoints.remove(waypoint);
	}

	bool GOCWaypoint::HasLine(GOCWaypoint *waypoint)
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

	void GOCWaypoint::UpdatePosition(Ogre::Vector3 position)
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

	void GOCWaypoint::ShowEditorVisual(bool show)
	{
		if (show == mRenderEditorVisuals) return;
		mRenderEditorVisuals = show;
		if (show)
		{
			for (std::list<GOCWaypoint*>::iterator i = mConnectedWaypoints.begin(); i != mConnectedWaypoints.end(); i++)
			{
				if (!HasLine((*i))) (*i)->AddLine(CreateEditorLine((*i)), this);
			}
		}
		else
		{
			for (std::list<LineNeighborBind>::iterator i = mLines.begin(); i != mLines.end(); i++)
			{
				(*i).mNeighbor->RemoveLine((*i).mLine);
				Main::Instance().GetOgreSceneMgr()->destroySceneNode((*i).mLine->getParentSceneNode());
				Main::Instance().GetOgreSceneMgr()->destroyManualObject((*i).mLine);
			}
			mLines.clear();
		}
	}

	bool GOCWaypoint::HasConnectedWaypoint(GOCWaypoint *waypoint)
	{
		for (std::list<GOCWaypoint*>::iterator i = mConnectedWaypoints.begin(); i != mConnectedWaypoints.end(); i++)
		{
			if ((*i) == waypoint) return true;
		}
		return false;
	}

	Ogre::Vector3 GOCWaypoint::GetPosition()
	{
		return GetOwner()->GetGlobalPosition();
	}
	Ogre::Quaternion GOCWaypoint::GetOrientation()
	{
		return GetOwner()->GetGlobalOrientation();
	}

	void GOCWaypoint::GetNeighbors(std::vector<WPEdge> *edges, Ogre::Vector3 targetPos)
	{
		for (std::list<GOCWaypoint*>::iterator i = mConnectedWaypoints.begin(); i != mConnectedWaypoints.end(); i++)
		{
			WPEdge edge;
			edge.mCost = GetPosition().distance((*i)->GetPosition());
			edge.mWP = this;
			edge.mNeighbor = (*i);
			edge.mCostOffset = (*i)->GetPosition().distance(targetPos);
			edges->push_back(edge);
		}
	}

	void GOCWaypoint::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("std::list<Saveable*>", (void*)(&mConnectedWaypoints), "mConnectedWaypoints");
	}
	void GOCWaypoint::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("std::list<Saveable*>", (void*)(&mConnectedWaypoints));
	}

};