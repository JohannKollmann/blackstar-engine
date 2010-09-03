
#pragma once

#include "EDTIncludes.h"
#include "IceNavigationMesh.h"
#include "IceGOCView.h"
#include <vector>
#include "IceMessageSystem.h"

class NavMeshEditorNode : public Ice::GOCEditorVisualised, public Ice::MessageListener
{
protected:
	Ogre::String GetEditorVisualMeshName() { return "sphere.25cm.mesh"; }

	public:
		enum Type
		{
			NODE,
			EDGE
		};
		typedef NavMeshEditorNode* NavMeshEditorNodePtr;
		struct NeighbourBind
		{
			NavMeshEditorNodePtr neighbour;
			NavMeshEditorNodePtr edge;
			Ogre::ManualObject* line;
		};
		struct TriangleBind
		{
			NeighbourBind n1;
			NeighbourBind n2;
		};

	private:
		std::vector<TriangleBind> mTriangles;
		Type mType;
		bool mClearingScene;
		bool mEdgeLocked;

		Ogre::ManualObject* CreateLine(NavMeshEditorNode *other);

		void _notifyNeighbour(TriangleBind bind);
		void _notifyEdgeDestruction(NavMeshEditorNodePtr edge);
		void _notifyTriangleDestruction(NavMeshEditorNodePtr neighbour, NavMeshEditorNodePtr other);
		void _destroyThirdEdge(NavMeshEditorNodePtr neighbour, NavMeshEditorNodePtr other);
		void _destroyEdge(NeighbourBind bind);
		NeighbourBind* _getExistingNeighbourBind(NavMeshEditorNodePtr n);
		void _connect(NeighbourBind &bind);
		void _ensureedges();
		NeighbourBind* _getBorderEdge(NavMeshEditorNodePtr n);

	public:
		NavMeshEditorNode();
		NavMeshEditorNode(Ice::GameObject *owner, Type type, NavMeshEditorNodePtr node1, NavMeshEditorNodePtr node2);
		~NavMeshEditorNode(void);

		goc_id_type& GetComponentID() const { static std::string name = "NavMeshNode"; return name; } 

		virtual std::string& TellName() { static std::string name = "NavMeshNode"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "NavMeshNode"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new NavMeshEditorNode; };

		void AddTriangle(NavMeshEditorNodePtr node1, NavMeshEditorNodePtr node2, bool addToMesh = true);

		void LockEdgePosition(bool locked);
		void UpdatePosition(Ogre::Vector3 position);

		std::vector<TriangleBind>& GetTriangles();

		Type GetType() { return mType; }

		static bool IsEdge(Ice::GameObject *obj);

		void ReceiveMessage(Ice::Msg &msg);

		static void FromMesh(Ice::NavigationMesh *mesh);
};