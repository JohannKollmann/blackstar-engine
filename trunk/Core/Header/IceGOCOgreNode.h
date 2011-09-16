#pragma once

#include "IceGOComponent.h"
#include "IceGOCScriptMakros.h"
#include "IceScriptSystem.h"

namespace Ice
{
	class DllExport GOCOgreNode : public GOComponent
	{
	protected:
		Ogre::SceneNode *mNode;

	public:
		GOCOgreNode(void);
		~GOCOgreNode(void);

		void UpdatePosition(const Ogre::Vector3 &position);
		void UpdateOrientation(const Ogre::Quaternion &orientation);
		void UpdateScale(const Ogre::Vector3 &scale);

		virtual GOComponent::FamilyID& GetComponentID() const { static std::string name = "OgreNode"; return name; }

		bool _getIsSaveable() const { return false; }

		Ogre::SceneNode* GetNode() { return mNode; }

		std::vector<ScriptParam> SetVisible(Script &caller, std::vector<ScriptParam> &params) { mNode->setVisible(params[0].getBool());  return std::vector<ScriptParam>(); };
		DEFINE_TYPEDGOCLUAMETHOD(GOCOgreNode, SetVisible, "bool")
	};

	class DllExport GOCOgreNodeUser : public GOComponent
	{
	public:
		virtual ~GOCOgreNodeUser();
		Ogre::SceneNode* GetNode();

		virtual AccessPermissionID GetAccessPermissionID() { return AccessPermissions::ACCESS_VIEW; }
	};

	class DllExport GOCOgreNodeUserEditable : public GOCOgreNodeUser, public GOCEditorInterface
	{
	public:
		virtual ~GOCOgreNodeUserEditable() {};

		GOComponent* GetGOComponent() { return this; }
	};

}