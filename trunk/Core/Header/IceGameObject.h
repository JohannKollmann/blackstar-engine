
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
#include "IceUtils.h"

namespace Ice
{

	/**
	Every entity in 3D space is a game object. A game object has a position, rotation and scale (Transformable3D) and provides parent-child support.
	A game object also consists of a set of components, which provide special functionality like mesh, sound, ai etc.
	*/
	class DllExport GameObject : public LoadSave::Saveable, public Transformable3D, public Utils::DeleteListener
	{
	protected:
		int mID;

		//Name
		Ogre::String mName;

		bool mLoadSaveByParent;

		bool mSelectable;
		bool mFreezed;

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

		std::map<Ogre::String, ScriptParam> mScriptProperties;

	public:
		GameObject();
		virtual ~GameObject();

		/**
		@return The name of the object.
		*/
		Ogre::String GetName() { return mName; }

		/**
		Sets the name of the object.
		@param name The new name of the object.
		*/
		void SetName(Ogre::String name) { mName = name; }

		/**
		@return The unique ID of the object.
		*/
		int GetID() { return mID; }

		/**
		@return The unique ID of the object as string.
		*/
		Ogre::String GetIDStr() { return Ogre::StringConverter::toString(mID); }

		/**
		@return The object's parent (nullptr if not existing).
		*/
		GameObject* GetParent() { return mParent; }

		///Broadcasts a message to all components.
		void SendMessage(const Msg &msg);

		///Broadcasts a messe too all components without any delay.
		void SendInstantMessage(const Msg &msg);

		///Dispatches all waiting message requests created using SendMessage.
		void ProcessMessages();

		/**
		Attaches a component to the object.
		@param component The component.
		*/
		void AddComponent(GOComponent* component);

		///Removes the component of family familyID.
		void RemoveComponent(const GOComponent::goc_id_family& familyID);

		///Returns the component of type T if it exists, otherwise nullptr.
		template<class T>
		T* GetComponent()
		{
			for (std::vector<GOComponent*>::iterator i = mComponents.begin(); i != mComponents.end(); i++)
			{
				T *rtti = dynamic_cast<T*>((*i));
				if (rtti) return rtti;
			}
			return nullptr;
		}

		///Returns the component of family familyID if it exists, otherwise nullptr.
		GOComponent* GetComponent(const GOComponent::goc_id_family& familyID);

		///Returns the component of family familyID and type typeID if it exists, otherwise nullptr.
		GOComponent* GetComponent(const GOComponent::goc_id_family& familyID, GOComponent::goc_id_type typeID);

		//Provides a mechanism to iterate over the components attached to the object.
		std::vector<GOComponent*>::iterator GetComponentIterator() { return mComponents.begin(); }
		std::vector<GOComponent*>::iterator GetComponentIteratorEnd() { return mComponents.end(); }

		///Detaches and deletes all attaches components.
		void ClearGOCs();

		///Deletes all children.
		void ClearChildren();

		///Sets the parent.
		void SetParent(GameObject *parent);

		///Registers an object as child of this object.
		void RegisterChild(GameObject *child);

		///Unregisters an object as child of this object.
		void UnregisterChild(GameObject *child);

		/**
		Detaches all children.
		@return The detached children.
		*/
		std::vector<GameObject*> DetachChildren();

		/**
		@return The number of children.
		*/
		unsigned int GetNumChildren()
		{
			return mChildren.size();
		}

		/**
		@return The child at index index if existing, otherwise nullptr.
		*/
		GameObject* GetChild(unsigned short index);

		///Notifies the object that the state of its parent has changed.
		void OnParentChanged();

		///Shall the object ignore the parent's movement/rotation/scale?
		void SetIgnoreParent(bool ignore) { mIgnoreParent = ignore; }

		Ogre::Vector3 GetGlobalPosition() { return mPosition; }
		Ogre::Quaternion GetGlobalOrientation() { return mOrientation; }	
		Ogre::Vector3 GetGlobalScale() { return mScale; }
		void SetGlobalPosition(Ogre::Vector3 pos) { SetGlobalPosition(pos, true); }
		void SetGlobalPosition(Ogre::Vector3 pos, bool updateChildren);
		void SetGlobalOrientation(Ogre::Quaternion quat) { SetGlobalOrientation(quat, true); }
		void SetGlobalOrientation(Ogre::Quaternion quat, bool updateChildren);
		void SetGlobalScale(Ogre::Vector3 scale);
		void Translate(Ogre::Vector3 vec, bool updateChildren = true) { SetGlobalPosition(mPosition + vec, updateChildren); }
		void Rotate(Ogre::Vector3 axis, Ogre::Radian angle, bool updateChildren = true) { Ogre::Quaternion q; q.FromAngleAxis(angle, axis); SetGlobalOrientation(mOrientation * q, updateChildren); }
		void Rescale(Ogre::Vector3 scaleoffset) { SetGlobalScale(mScale + scaleoffset); }
		bool GetTransformingChildren() { return mTransformingChildren; }
		bool GetUpdatingFromParent() { return mUpdatingFromParent; }

		/**
		Returns whether the object is movable and should be included in a save file.
		*/
		bool IsStatic();

		//Scripting
		std::vector<ScriptParam> SetObjectProperty(std::vector<ScriptParam> &params);
		std::vector<ScriptParam> GetObjectProperty(std::vector<ScriptParam> &params);
		std::vector<ScriptParam> SetObjectPosition(std::vector<ScriptParam> &params);
		std::vector<ScriptParam> SetObjectOrientation(std::vector<ScriptParam> &params);
		std::vector<ScriptParam> SetObjectScale(std::vector<ScriptParam> &params);
		std::vector<ScriptParam> GetObjectName(std::vector<ScriptParam> &params);

		static std::vector<ScriptParam> Lua_SetObjectProperty(Script& caller, std::vector<ScriptParam> params);
		static std::vector<ScriptParam> Lua_GetObjectProperty(Script& caller, std::vector<ScriptParam> params);
		static std::vector<ScriptParam> Lua_SetObjectPosition(Script& caller, std::vector<ScriptParam> params);
		static std::vector<ScriptParam> Lua_SetObjectOrientation(Script& caller, std::vector<ScriptParam> params);
		static std::vector<ScriptParam> Lua_SetObjectScale(Script& caller, std::vector<ScriptParam> params);
		static std::vector<ScriptParam> Lua_GetObjectName(Script& caller, std::vector<ScriptParam> params);

		//Editor stuff
		void Freeze(bool freeze);
		void ShowEditorVisuals(bool show);

		bool IsSelectable() { return mSelectable; }
		void SetSelectable(bool selectable) { mSelectable = selectable; }

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