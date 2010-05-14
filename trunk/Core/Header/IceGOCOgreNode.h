#pragma once

#include "IceGOComponent.h"

namespace Ice
{
	class DllExport GOCOgreNode : public GOComponent
	{
	protected:
		Ogre::SceneNode *mNode;

	public:
		GOCOgreNode(void);
		~GOCOgreNode(void);

		void UpdatePosition(Ogre::Vector3 position);
		void UpdateOrientation(Ogre::Quaternion orientation);

		virtual goc_id_family& GetComponentID() const { static std::string name = "OgreNode"; return name; }

		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "OgreNode"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; }
		static LoadSave::Saveable* NewInstance() { return new GOCOgreNode; }

		Ogre::SceneNode* GetNode() { return mNode; }
	};

	class DllExport GOCOgreNodeUser : public GOComponent
	{
	public:
		virtual ~GOCOgreNodeUser();
		Ogre::SceneNode* GetNode();
	};

	class DllExport GOCOgreNodeUserEditable : public GOCOgreNodeUser, public GOCEditorInterface
	{
	public:
		virtual ~GOCOgreNodeUserEditable() {};

		GOComponent* GetGOComponent() { return this; }
	};

}