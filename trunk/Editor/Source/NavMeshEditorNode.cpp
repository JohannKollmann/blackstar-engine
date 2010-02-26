
#include "NavMeshEditorNode.h"
#include "IceMain.h"
#include "IceGameObject.h"


	NavMeshEditorNode::NavMeshEditorNode()
	: Ice::MeshDebugRenderable("sphere.25cm.mesh")
	{
		mType = Type::NODE;
	}

	NavMeshEditorNode::NavMeshEditorNode(Ice::GameObject *owner, Type type, TriangleNodePtr triangle, NavMeshEditorNodePtr node1, NavMeshEditorNodePtr node2)
	: Ice::MeshDebugRenderable("sphere.25cm.mesh")
	{
		mType = NODE;
		owner->AddComponent(this);
		mType = type;
		if (mType == NODE)
			AddTriangle(triangle, node1, node2);
		else
		{
			mEdgeLocked = true;
			TriangleBind bind;
			bind.n1.neighbour = node1;
			bind.n1.line = 0;
			bind.n2.neighbour = node2;
			bind.n2.line = 0;
			bind.tri = triangle;
			mTriangles.push_back(bind);
		}
	}

	std::vector<NavMeshEditorNode::TriangleBind>& NavMeshEditorNode::GetTriangles()
	{
		return mTriangles;
	}

	void NavMeshEditorNode::AddTriangle(TriangleNodePtr triangle, NavMeshEditorNodePtr node1, NavMeshEditorNodePtr node2)
	{
		TriangleBind bind;
		bind.n1.neighbour = node1;
		bind.n2.neighbour = node2;
		bind.tri = triangle;

		_connect(bind.n1, bind.tri);
		_connect(bind.n2, bind.tri);

		TriangleBind bind1 = bind;
		bind1.n1.neighbour = this;
		node1->_connect(bind1.n2, bind1.tri);
		node1->_notifyNeighbour(bind1);

		TriangleBind bind2 = bind;
		bind2.n2.neighbour = this;
		bind2.n1.line = bind1.n2.line;
		bind2.n1.edge = bind1.n2.edge;
		node2->_notifyNeighbour(bind2);

		mTriangles.push_back(bind);

	}

	void NavMeshEditorNode::_connect(NeighbourBind &bind, TriangleNodePtr triangle)
	{
		NeighbourBind *pExists = _getExistingNeighbourBind(bind.neighbour);
		if (pExists)
		{
			if (pExists->edge)
			{
				pExists->neighbour->_notifyEdgeDestruction(pExists->edge);
				delete pExists->edge->GetOwner();
				pExists->edge = 0;
			}
			bind.edge = pExists->edge;
			bind.line = pExists->line;
		}
		else
		{
			bind.edge = new NavMeshEditorNode(new Ice::GameObject(mOwnerGO->GetParent()), EDGE, triangle, this, bind.neighbour);
			bind.line = CreateLine(bind.neighbour);
		}
	}

	NavMeshEditorNode::NeighbourBind* NavMeshEditorNode::_getBorderEdge(NavMeshEditorNodePtr n)
	{
		int num = 0;
		for (std::vector<TriangleBind>::iterator i = mTriangles.begin(); i != mTriangles.end(); i++)
		{
			if (i->n1.neighbour == n || i->n2.neighbour == n) num++;
		}
		return ((num == 1) ? n->_getExistingNeighbourBind(this) : 0);
	}

	void NavMeshEditorNode::_ensureEdges()
	{
		for (std::vector<TriangleBind>::iterator i = mTriangles.begin(); i != mTriangles.end(); i++)
		{
			NeighbourBind *border = _getBorderEdge(i->n1.neighbour);
			if (!i->n1.edge && border)
			{
				if (border->edge) i->n1.edge = border->edge;
				else i->n1.edge = new NavMeshEditorNode(new Ice::GameObject(mOwnerGO->GetParent()), EDGE, i->tri, this, i->n1.neighbour);
			}
			border = _getBorderEdge(i->n2.neighbour);
			if (!i->n2.edge && border)
			{
				if (border->edge) i->n2.edge = border->edge;
				else i->n2.edge = new NavMeshEditorNode(new Ice::GameObject(mOwnerGO->GetParent()), EDGE, i->tri, this, i->n2.neighbour);
			}
		}
		UpdatePosition(mOwnerGO->GetGlobalPosition());
	}

	NavMeshEditorNode::~NavMeshEditorNode(void)
	{
		if (mType == NODE)
		{
			std::vector<NavMeshEditorNodePtr> uniqueNeighbours;
			std::vector<TriangleBind>::iterator i = mTriangles.begin();
			while (i != mTriangles.end())
			{
				i->n1.neighbour->_destroyThirdEdge(this, i->n2.neighbour);
				i->n1.neighbour->_notifyTriangleDestruction(this, i->n2.neighbour);
				i->n2.neighbour->_notifyTriangleDestruction(this, i->n1.neighbour);
				_destroyEdge(i->n1);
				_destroyEdge(i->n2);

				bool stop = false;
				for (std::vector<NavMeshEditorNodePtr>::iterator x = uniqueNeighbours.begin(); x != uniqueNeighbours.end(); x++)
					if ((*x) == i->n1.neighbour) stop = true;
				if (!stop) uniqueNeighbours.push_back(i->n1.neighbour);
				stop = false;
				for (std::vector<NavMeshEditorNodePtr>::iterator x = uniqueNeighbours.begin(); x != uniqueNeighbours.end(); x++)
					if ((*x) == i->n2.neighbour) stop = true;
				if (!stop) uniqueNeighbours.push_back(i->n2.neighbour);
				mTriangles.erase(i);
				i = mTriangles.begin();
			}

			for (std::vector<NavMeshEditorNodePtr>::iterator i = uniqueNeighbours.begin(); i != uniqueNeighbours.end(); i++)
			{
				if ((*i)->mTriangles.size() == 0)
				{
					(*i)->mType = EDGE; //Hack
					delete (*i)->GetOwner();
				}
				else (*i)->_ensureEdges();
			}
		}
	}

	void NavMeshEditorNode::_notifyNeighbour(TriangleBind bind)
	{
		mTriangles.push_back(bind);
	}

	NavMeshEditorNode::NeighbourBind* NavMeshEditorNode::_getExistingNeighbourBind(NavMeshEditorNodePtr n)
	{
		for (std::vector<TriangleBind>::iterator i = mTriangles.begin(); i != mTriangles.end(); i++)
		{
			if (i->n1.neighbour == n) return &i->n1;
			if (i->n2.neighbour == n) return &i->n2;
		}
		return 0;
	}

	void NavMeshEditorNode::_destroyEdge(NeighbourBind bind)
	{
		int num = 0;
		for (std::vector<TriangleBind>::iterator i = mTriangles.begin(); i != mTriangles.end(); i++)
		{
			if (i->n1.line == bind.line || i->n2.line == bind.line) num++;
		}
		if (num < 2)
		{
			bind.line->getParentSceneNode()->detachObject(bind.line);
			Ice::Main::Instance().GetOgreSceneMgr()->destroyManualObject(bind.line);
			if (bind.edge)
			{
				delete bind.edge->GetOwner();
				bind.edge = 0;
			}
		}
	}

	void NavMeshEditorNode::_notifyTriangleDestruction(NavMeshEditorNodePtr neighbour, NavMeshEditorNodePtr other)
	{
		std::vector<TriangleBind>::iterator i = mTriangles.begin();
		for (; i != mTriangles.end(); i++)
		{
			if (i->n1.neighbour == neighbour && i->n2.neighbour == other) break;
			if (i->n2.neighbour == neighbour && i->n1.neighbour == other) break;
		}
		mTriangles.erase(i);
	}

	void NavMeshEditorNode::_destroyThirdEdge(NavMeshEditorNodePtr neighbour, NavMeshEditorNodePtr other)
	{
		for (std::vector<TriangleBind>::iterator i = mTriangles.begin(); i != mTriangles.end(); i++)
		{
			if (i->n1.neighbour == neighbour && i->n2.neighbour == other) _destroyEdge(i->n2);
			if (i->n2.neighbour == neighbour && i->n1.neighbour == other) _destroyEdge(i->n1);
		}
	}

	void NavMeshEditorNode::LockEdgePosition(bool lock)
	{
		mEdgeLocked = lock;
	}

	void NavMeshEditorNode::_notifyEdgeDestruction(NavMeshEditorNodePtr edge)
	{
		for (std::vector<TriangleBind>::iterator i = mTriangles.begin(); i != mTriangles.end(); i++)
		{
			if (i->n1.edge == edge) i->n1.edge = 0;
			if (i->n2.edge == edge) i->n2.edge = 0;
		}
	}

	void NavMeshEditorNode::UpdatePosition(Ogre::Vector3 position)
	{
		mNode->setPosition(position);
		if (mType == EDGE)
		{
			bool parentUpdate = false;
			if (mOwnerGO->GetParent()) parentUpdate = mOwnerGO->GetParent()->GetTranformingChildren();
			if (mEdgeLocked && !parentUpdate)
			{
				mType = NODE;
				TriangleBind bind = *mTriangles.begin();
				TriangleNodePtr triangle = bind.tri;
				NavMeshEditorNodePtr node1 = bind.n1.neighbour;
				NavMeshEditorNodePtr node2 = bind.n2.neighbour;
				mTriangles.pop_back();
				node1->_notifyEdgeDestruction(this);
				node2->_notifyEdgeDestruction(this);
				AddTriangle(triangle, node1, node2);
			}
		}
		if (mType == NODE)
		{
			Ogre::Vector3 myPos = GetOwner()->GetGlobalPosition();
			for (std::vector<TriangleBind>::iterator i = mTriangles.begin(); i != mTriangles.end(); i++)
			{
				if (i->n1.line)
				{
					i->n1.line->clear();
					i->n1.line->begin("WPLine", Ogre::RenderOperation::OT_LINE_LIST);
					i->n1.line->position(myPos);
					i->n1.line->position(i->n1.neighbour->GetOwner()->GetGlobalPosition());
					i->n1.line->end();
				}
				if (i->n2.line)
				{
					i->n2.line->clear();
					i->n2.line->begin("WPLine", Ogre::RenderOperation::OT_LINE_LIST);
					i->n2.line->position(myPos);
					i->n2.line->position(i->n2.neighbour->GetOwner()->GetGlobalPosition());
					i->n2.line->end();
				}

				if (i->n1.edge)
				{
					i->n1.edge->LockEdgePosition(false);
					i->n1.edge->GetOwner()->SetGlobalPosition(0.5f * (i->n1.neighbour->GetOwner()->GetGlobalPosition() + myPos));
					i->n1.edge->LockEdgePosition(true);
				}
				if (i->n2.edge)
				{
					i->n2.edge->LockEdgePosition(false);
					i->n2.edge->GetOwner()->SetGlobalPosition(0.5f * (i->n2.neighbour->GetOwner()->GetGlobalPosition() + myPos));
					i->n2.edge->LockEdgePosition(true);
				}
			}
		}
	}

	Ogre::ManualObject* NavMeshEditorNode::CreateLine(NavMeshEditorNode *other)
	{
		Ogre::ManualObject* line = Ice::Main::Instance().GetOgreSceneMgr()->createManualObject("WaynetLine_" + mOwnerGO->GetIDStr() + other->GetOwner()->GetIDStr());
		Ogre::SceneNode* lineNode = Ice::Main::Instance().GetOgreSceneMgr()->getRootSceneNode();//->createChildSceneNode("WaynetLine_" + mOwnerGO->GetIDStr() + other->GetOwner()->GetIDStr());
		line->begin("WPLine", Ogre::RenderOperation::OT_LINE_LIST);
		line->position(GetOwner()->GetGlobalPosition() + Ogre::Vector3(0,0.2f,0));
		line->position(other->GetOwner()->GetGlobalPosition() + Ogre::Vector3(0,0.2f,0));
		line->end();
		line->setCastShadows(false);
		lineNode->attachObject(line);
		return line;
	}

	bool NavMeshEditorNode::IsEdge(Ice::GameObject *obj)
	{
		NavMeshEditorNode *node = (NavMeshEditorNode*)obj->GetComponent("MeshDebugRenderable", "NavMeshNode");
		if (node) return (node->GetType() == EDGE);
		return false;
	}