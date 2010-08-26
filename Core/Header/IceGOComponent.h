
#pragma once

#include "LoadSave.h"
#include "IceIncludes.h"
#include "Ogre.h"
#include "IceDataMap.h"
#include "IceGOCEditorInterface.h"

namespace Ice
{

	/**
	A component provides a certain functionality that can be attached to game object.
	*/
	class DllExport GOComponent : public LoadSave::Saveable
	{
		friend class GameObject;
	private:
		//Called by GO Owner
		void _updatePosition(const Ogre::Vector3 &position);
		void _updateOrientation(const Ogre::Quaternion &quat);
		void _updateScale(const Ogre::Vector3 &scale);
		bool mTransformingOwner;

		virtual bool _getIsSaveable() const { return true; }

	protected:
		GameObject *mOwnerGO;
		virtual void NotifyOwnerGO();

	public:
		typedef std::string goc_id_type;
		typedef std::string goc_id_family;

		GOComponent() : mOwnerGO(nullptr), mTransformingOwner(false) {}
		virtual ~GOComponent() {}

		///Sets the position of the component's owner object.
		void SetOwnerPosition(const Ogre::Vector3 &position, bool updateChildren = true);

		///Sets the orientation of the component's owner object.
		void SetOwnerOrientation(const Ogre::Quaternion &orientation, bool updateChildren = true);

		///Retrieves the component type identifier.
		virtual goc_id_type& GetComponentID() const = 0;

		virtual void FreeResources(bool free) {};

		/**
		Retrieves the component family type identifier.
		@return the family name of the component.
		@remarks Only one component of the same family can be attached to the same object.
		*/		
		virtual goc_id_family& GetFamilyID() const { return GetComponentID(); }

		///Sets the component owner object.
		virtual void SetOwner(GameObject *go);

		///Retrieves the component owner object.
		GameObject* GetOwner() const { return mOwnerGO; }

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

		///Dispatches an object message
		virtual void ReceiveObjectMessage(Msg &msg) {}

		///Freezes the component
		virtual void Freeze(bool freeze) {};

		///Shows the editor / debug visual
		virtual void ShowEditorVisual(bool show) {};

		//Load save methods
		std::string& TellName() { return GetComponentID(); };
		void Save(LoadSave::SaveSystem& mgr) {}
		void Load(LoadSave::LoadSystem& mgr) {}
	};

	typedef DllExport std::shared_ptr<GOComponent> GOComponentPtr;

	class DllExport GOComponentEditable : public GOCEditorInterface, public GOComponent
	{
	public:
		virtual ~GOComponentEditable() {}

		GOComponent* GetGOComponent() { return this; }
	};

};