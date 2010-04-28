
#pragma once

#include "IceIncludes.h"
#include "Ogre.h"
#include <list>
#include "LoadSave.h"
#include <string>
#include "IceIncludes.h"
#include "IceGOComponent.h"
#include "Ice3D.h"
#include "IceMsg.h"

namespace Ice
{

	class DllExport GameObject : public LoadSave::Saveable, public Transformable3D
	{
	protected:
		int mID;

		//Name
		Ogre::String mName;

		bool mLoadSaveByParent;

		bool mSelectable;
		bool mFreezePosition;
		bool mFreezeOrientation;

		bool mTransformingChildren;
		bool mUpdatingFromParent;

		bool mIgnoreParent;

		//Global and local transform
		Ogre::Vector3 mPosition;
		Ogre::Quaternion mOrientation;
		Ogre::Vector3 mLocalPosition;
		Ogre::Quaternion mLocalOrientation;
		Ogre::Vector3 mScale;

		//Components
		std::vector<GOComponent*> mComponents;

		//Messaging
		std::vector<Msg> mCurrentMessages;

		//Parent and children
		GameObject* mParent;
		std::vector<GameObject*> mChildren;

		void UpdateChildren(bool move = true);
		void UpdateLocalTransform();

	public:
		GameObject();
		virtual ~GameObject();

		Ogre::String GetName() { return mName; }
		void SetName(Ogre::String name) { mName = name; }

		int GetID() { return mID; }
		Ogre::String GetIDStr() { return Ogre::StringConverter::toString(mID); }

		GameObject* GetParent() { return mParent; }

		void SendMessage(const Msg &msg);
		void SendInstantMessage(Ogre::String receiver_family, const Msg &msg);
		void ProcessMessages();

		void AddComponent(GOComponent* component);

		template<class T>
		T* GetComponent()
		{
			for (std::vector<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
			{
				T *rtti = dynamic_cast<T*>((*i));
				if (rtti) return rtti;
			}
			return 0;
		}
		GOComponent* GetComponent(const GOComponent::goc_id_family& familyID);
		GOComponent* GetComponent(const GOComponent::goc_id_family& familyID, GOComponent::goc_id_type typeID);
		std::vector<Ogre::String> GetComponentsStr();
		std::vector<GOComponent*>::iterator GetComponentIterator() { return mComponents.begin(); }
		std::vector<GOComponent*>::iterator GetComponentIteratorEnd() { return mComponents.end(); }
		void RemoveComponent(const GOComponent::goc_id_family& familyID);
		void ClearGOCs();
		void ClearChildren();

		void SetLoadSaveByParent(bool loadsave) { mLoadSaveByParent = loadsave; }
		void SetIgnoreParent(bool ignore) { mIgnoreParent = ignore; }

		Ogre::Vector3 GetGlobalPosition() { return mPosition; }
		Ogre::Quaternion GetGlobalOrientation() { return mOrientation; }	
		Ogre::Vector3 GetGlobalScale() { return mScale; }
		void SetGlobalPosition(Ogre::Vector3 pos) { SetGlobalPosition(pos, true); }
		void SetGlobalPosition(Ogre::Vector3 pos, bool updateChildren);
		void SetGlobalOrientation(Ogre::Quaternion quat) { SetGlobalOrientation(quat, true); }
		void SetGlobalOrientation(Ogre::Quaternion quat, bool updateChildren);
		void SetGlobalScale(Ogre::Vector3 scale);
		void Translate(Ogre::Vector3 vec, bool updateChildren = true) { if (!mFreezePosition) SetGlobalPosition(mPosition + vec, updateChildren); }
		void Rotate(Ogre::Vector3 axis, Ogre::Radian angle, bool updateChildren = true) { if (!mFreezeOrientation) { Ogre::Quaternion q; q.FromAngleAxis(angle, axis); SetGlobalOrientation(mOrientation * q, updateChildren); } }
		void Rescale(Ogre::Vector3 scaleoffset) { SetGlobalScale(mScale + scaleoffset); }
		bool GetTransformingChildren() { return mTransformingChildren; }
		bool GetUpdatingFromParent() { return mUpdatingFromParent; }

		bool IsSelectable() { return mSelectable; }
		void SetSelectable(bool selectable) { mSelectable = selectable; }
		void SetFreezePosition(bool freeze) { mFreezePosition = freeze; }
		void SetFreezeOrientation(bool freeze) { mFreezeOrientation = freeze; }

		bool IsStatic();

		//Editor stuff
		void Freeze(bool freeze);
		void ShowEditorVisuals(bool show);

		void SetParent(GameObject *parent);
		void RegisterChild(GameObject *child);
		void UnregisterChild(GameObject *child);
		std::vector<GameObject*> DetachChildren();

		unsigned int GetNumChildren()
		{
			return mChildren.size();
		}
		GameObject* GetChild(unsigned short index);

		void OnParentChanged();

		//Load / Save
		std::string& TellName()
		{
			static std::string name = "GameObject"; return name;
		};
		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "GameObject"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; }
		static LoadSave::Saveable* NewInstance() { return new GameObject(); }
	};

	class ManagedGameObject : public GameObject
	{
	public:
		ManagedGameObject();
		~ManagedGameObject();

		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "ManagedGameObject"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; }
		std::string& TellName()
		{
			static std::string name = "ManagedGameObject"; return name;
		};
		static LoadSave::Saveable* NewInstance() { return new ManagedGameObject(); }
	};

};