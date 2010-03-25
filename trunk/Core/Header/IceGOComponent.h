
#pragma once

#include "LoadSave.h"
#include "IceIncludes.h"
#include "Ogre.h"
#include "IceDataMap.h"

namespace Ice
{

	struct ObjectMsg
	{
		Ogre::String mName;
		DataMap mData;
		void *rawData;
	};

	class DllExport GOComponent : public LoadSave::Saveable
	{
		friend class GameObject;
	private:
		void _updatePosition(const Ogre::Vector3 &position);
		void _updateOrientation(const Ogre::Quaternion &quat);
		void _updateScale(const Ogre::Vector3 &scale);
		bool mTranformingOwner;

	protected:
		GameObject *mOwnerGO;

	public:
		typedef std::string goc_id_type;
		typedef std::string goc_id_family;

		GOComponent() : mOwnerGO(nullptr), mTranformingOwner(false) {}
		virtual ~GOComponent() {}

		void SetOwnerPosition(const Ogre::Vector3 &position);
		void SetOwnerOrientation(const Ogre::Quaternion &orientation);

		virtual goc_id_type& GetComponentID() const = 0;
		virtual goc_id_family& GetFamilyID() const = 0;

		std::string& TellName() { return GetComponentID(); };

		virtual void SetOwner(GameObject *go);
		GameObject* GetOwner() const { return mOwnerGO; }

		virtual void OnAddChild(GameObject *child) {}
		virtual void OnRemoveChild(GameObject *child) {}

		//Called by GO Owner
		virtual void UpdatePosition(Ogre::Vector3 position) {}
		virtual void UpdateOrientation(Ogre::Quaternion orientation) {}
		virtual void UpdateScale(Ogre::Vector3 scale) {}

		virtual bool IsStatic() { return true; }

		//Messaging
		virtual void ReceiveObjectMessage(Ogre::SharedPtr<ObjectMsg> msg) {}

		//Editor stuff
		virtual void Freeze(bool freeze) {};
		virtual void ShowEditorVisual(bool show) {};
	};

};