
#pragma once

#include "EDTIncludes.h"
#include "IceNavigationMesh.h"
#include "IceGOCView.h"
#include <vector>

	class NavMeshEditorNode : public Ice::MeshDebugRenderable
	{
	public:
		enum Type
		{
			NODE,
			EDGE
		};
		typedef Ice::TriangleNode* TriangleNodePtr;
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
			TriangleNodePtr tri;
		};

	private:
		std::vector<TriangleBind> mTriangles;
		Type mType;
		bool mEdgeLocked;

		Ogre::ManualObject* CreateLine(NavMeshEditorNode *other);

		void _notifyNeighbour(TriangleBind bind);
		void _notifyEdgeDestruction(NavMeshEditorNodePtr edge);
		void _notifyNeighbourDestruction(NavMeshEditorNodePtr neighbour, bool destroyEdges);
		void _destroyEdge(NeighbourBind bind);
		NeighbourBind* _getExistingNeighbourBind(NavMeshEditorNodePtr n);
		void _connect(NeighbourBind &bind, TriangleNodePtr triangle);

	public:
		NavMeshEditorNode();
		NavMeshEditorNode(Ice::GameObject *owner, Type type, TriangleNodePtr triangle, NavMeshEditorNodePtr node1, NavMeshEditorNodePtr node2);
		~NavMeshEditorNode(void);

		goc_id_type& GetComponentID() const { static std::string name = "NavMeshNode"; return name; } 

		virtual std::string& TellName() { static std::string name = "NavMeshNode"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "NavMeshNode"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new NavMeshEditorNode; };

		void AddTriangle(TriangleNodePtr triangle, NavMeshEditorNodePtr node1, NavMeshEditorNodePtr node2);

		void LockEdgePosition(bool locked);
		void UpdatePosition(Ogre::Vector3 position);

		Type GetType() { return mType; }
	};