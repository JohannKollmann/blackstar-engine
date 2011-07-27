
#pragma once

#include "LoadSave.h"
#include "IceIncludes.h"
#include "Ogre.h"
#include "IceDataMap.h"
#include "IceGOCEditorInterface.h"
#include "IceMessageListener.h"

namespace Ice
{

	/**
	* A component provides a certain functionality that can be attached to a game object.
	*/
	class DllExport GOComponent : public LoadSave::Saveable, public MessageListener
	{
		friend class GameObject;
	private:

		virtual bool _getIsSaveable() const { return true; }

	protected:
		std::weak_ptr<GameObject> mOwnerGO;
		virtual void NotifyOwnerGO();

		///Called by GameObject::FirePostInit
		virtual void NotifyPostInit() {}

	public:
		typedef std::string TypeID;
		typedef std::string FamilyID;

		GOComponent() {}
		virtual ~GOComponent() {}

		///Sets the position of the component's owner object.
		void SetOwnerPosition(const Ogre::Vector3 &position, bool updateReferences = false, bool updateChildren = true);

		///Sets the orientation of the component's owner object.
		void SetOwnerOrientation(const Ogre::Quaternion &orientation, bool updateReferences = false, bool updateChildren = true);

		///Sets both position and orientation of the owner object.
		void SetOwnerTransform(const Ogre::Vector3 &position, const Ogre::Quaternion &orientation, bool updateReferences = false, bool updateChildren = true);

		///Retrieves the component type identifier.
		virtual GOComponent::TypeID& GetComponentID() const = 0;

		virtual void FreeResources(bool free) {};

		/**
		* Retrieves the component family type identifier.
		* @return the family name of the component.
		* @remarks Only one component of the same family can be attached to the same object.
		*/		
		virtual GOComponent::FamilyID& GetFamilyID() const { return GetComponentID(); }

		///Sets the component owner object.
		virtual void SetOwner(std::weak_ptr<GameObject> go);

		///Retrieves the component owner object.
		GameObjectPtr GetOwner() const { return mOwnerGO.lock(); }

		///Called when a child is added to the owner object.
		virtual void OnAddChild(GameObject *child) {}

		///Called when a child is removed from the owner object.
		virtual void OnRemoveChild(GameObject *child) {}

		///Called when the position of the owner object changes.
		virtual void UpdatePosition(Ogre::Vector3 position) {}

		///Called when the orientation of the owner object changes.
		virtual void UpdateOrientation(Ogre::Quaternion orientation) {}

		///Called when the scale of the owner object changes.
		virtual void UpdateScale(Ogre::Vector3 scale) {}

		///Retrieves whether the component is static (movable) or not.
		virtual bool IsStatic() { return true; }
		
		//Default: access everything.
		virtual AccessPermissionID GetAccessPermissionID() { return AccessPermissions::ACCESS_ALL; }

		///MessageListener default implementation
		virtual void ReceiveMessage(Msg &msg);

		///Sends a message to a component of the owner object of a certain family, if existent.
		void SendObjectMessage(Msg &msg, GOComponent::FamilyID &familyID);

		///Broadcasts a message to all other components of the owner object.
		void BroadcastObjectMessage(Msg &msg);

		///Freezes the component
		virtual void Freeze(bool freeze) {};

		///Shows the editor / debug visual
		virtual void ShowEditorVisual(bool show) {};

		//Load save methods
		std::string& TellName() { return GetComponentID(); };
		void Save(LoadSave::SaveSystem& mgr) {}
		void Load(LoadSave::LoadSystem& mgr) {}
	};

	class DllExport GOComponentEditable : public GOCEditorInterface, public GOComponent
	{
	public:
		virtual ~GOComponentEditable() {}

		GOComponent* GetGOComponent() { return this; }
	};

};