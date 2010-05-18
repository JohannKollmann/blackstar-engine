
#pragma once

#include "LoadSave.h"
#include "IceIncludes.h"
#include "Ogre.h"
#include "IceDataMap.h"
#include "IceGOCEditorInterface.h"

namespace Ice
{

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

		void SetOwnerPosition(const Ogre::Vector3 &position);
		void SetOwnerOrientation(const Ogre::Quaternion &orientation);

		virtual goc_id_type& GetComponentID() const = 0;
		virtual goc_id_family& GetFamilyID() const { return GetComponentID(); }

		std::string& TellName() { return GetComponentID(); };
		void Save(LoadSave::SaveSystem& mgr) {}
		void Load(LoadSave::LoadSystem& mgr) {}

		virtual void SetOwner(GameObject *go);
		GameObject* GetOwner() const { return mOwnerGO; }

		virtual void OnAddChild(GameObject *child) {}
		virtual void OnRemoveChild(GameObject *child) {}

		virtual void UpdatePosition(Ogre::Vector3 position) {}
		virtual void UpdateOrientation(Ogre::Quaternion orientation) {}
		virtual void UpdateScale(Ogre::Vector3 scale) {}

		virtual bool IsStatic() { return true; }

		//Messaging
		virtual void ReceiveObjectMessage(const Msg &msg) {}

		//Editor stuff
		virtual void Freeze(bool freeze) {};
		virtual void ShowEditorVisual(bool show) {};
	};

	class DllExport GOComponentEditable : public GOCEditorInterface, public GOComponent
	{
	public:
		virtual ~GOComponentEditable() {}

		GOComponent* GetGOComponent() { return this; }
	};

};