
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

	//Script makro, must be here because of include issues
	#define DEFINE_GOLUAMETHOD_H(methodName) \
	static std::vector<ScriptParam> Lua_##methodName (Script& caller, std::vector<ScriptParam> vParams);

	class DllExport ObjectReference : public LoadSave::Saveable
	{
	public:
		ObjectReference() : Flags(0), UserID(0) {}

		enum FlagTypes
		{
			OWNER = 1,		//deletes linked object when object is deleted
			MOVER = 2,		//moves link object (parent-child style)
			PERSISTENT = 4	//loadsaves linked object
		};
		unsigned int Flags;
		std::weak_ptr<GameObject> Object;

		//You can store your own flags here.
		unsigned int UserID;


		//Load / Save
		std::string& TellName()
		{
			static std::string name = "ObjectReference"; return name;
		};
		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "ObjectReference"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; }
		static LoadSave::Saveable* NewInstance() { return new ObjectReference(); }
	};

	/**
	Every entity in 3D space is a game object. A game object has a position, rotation and scale (Transformable3D) and provides parent-child support.
	A game object also consists of a set of components, which provide special functionality like mesh, sound, ai etc.
	*/
	class DllExport GameObject : public LoadSave::Saveable, public Transformable3D, public Utils::DeleteListener
	{
	public:
		enum ReferenceTypes
		{
			PARENT = 11235813
		};

	protected:
		int mID;
		std::weak_ptr<GameObject> mWeakThis;

		//Name
		Ogre::String mName;

		bool mSelectable;
		bool mFreezed;

		bool mTransformingLinkedObjects;

		//Global and local transform
		Ogre::Vector3 mPosition;
		Ogre::Quaternion mOrientation;
		Ogre::Vector3 mLocalPosition;
		Ogre::Quaternion mLocalOrientation;
		Ogre::Vector3 mScale;

		//Components
		std::vector<GOComponentPtr> mComponents;

		//Messaging
		std::vector<Msg> mCurrentMessages;

		//Linked objects
		std::vector<ObjectReferencePtr> mReferencedObjects;
		std::vector<ObjectReferencePtr>::iterator mReferencedObjectsInternIterator;

	public:
		GameObject();
		virtual ~GameObject();

		/**
		Sets the weak "this" pointer. The factory responsible for the game object must call this function!
		@pre w.This.lock().get() == this
		*/
		void SetWeakThis(std::weak_ptr<GameObject> wThis);

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

		///Broadcasts a message to all components.
		void SendMessage(Msg &msg);

		///Broadcasts a messe too all components without any delay.
		void SendInstantMessage(Msg &msg);

		///Dispatches all waiting message requests created using SendMessage.
		void ProcessMessages();

		/**
		Attaches a component to the object.
		@param component The component.
		*/
		void AddComponent(GOComponentPtr component);

		///Removes the component of family familyID.
		void RemoveComponent(const GOComponent::goc_id_family& familyID);

		///Returns the component of type T if it exists, otherwise nullptr.
		template<class T>
		T* GetComponent()
		{
			for (auto i = mComponents.begin(); i != mComponents.end(); i++)
			{
				T *rtti = dynamic_cast<T*>((*i).get());
				if (rtti) return rtti;
			}
			return nullptr;
		}

		///Returns the component of family familyID if it exists, otherwise nullptr.
		GOComponent* GetComponent(const GOComponent::goc_id_family& familyID);

		///Returns the component of family familyID and type typeID if it exists, otherwise nullptr.
		GOComponent* GetComponent(const GOComponent::goc_id_family& familyID, GOComponent::goc_id_type typeID);

		//Provides a mechanism to iterate over the components attached to the object.
		std::vector<GOComponentPtr>::iterator GetComponentIterator() { return mComponents.begin(); }
		std::vector<GOComponentPtr>::iterator GetComponentIteratorEnd() { return mComponents.end(); }

		///Detaches and deletes all attaches components.
		void ClearGOCs();

		//Deletes all referenced objects that are owned by this object.
		void ClearOwnedObjects();

		/**
		Creates a directional link between two objects.
		@param other The other GameObject.
		@param flags Characterizes the relationship between the two objects, see ObjectReference.
		@pre It is NOT allowed that the MOVER or OWNER flag is set bidirectionally.
		*/
		void AddObjectReference(std::weak_ptr<GameObject> other, unsigned int flags = 0, unsigned int userID = 0);

		//Removes all object references with ObjectReference.Object == object.
		void RemoveObjectReferences(GameObject *object);

		//Removes all object references with ObjectReference.UserID == userID.
		void RemoveObjectReferences(unsigned int userID);

		//Creates a parent-child relationship between two objects.
		void SetParent(GameObjectPtr parent);

		//Retrieves the parent object if existent, otherwise an empty GameObjectPtr.
		GameObjectPtr GetParent();

		/**
		Provides a mechanism to iterate over the linked objects.
		example usage: 
		while (obj->HasNextLinkedObject()) { link = GetNextLinkedObject(); }
		*/
		bool HasNextObjectReference();
		std::shared_ptr<ObjectReference> GetNextObjectReference();

		/**
		Sets the linked object intern iterator to the beginning.
		HasNextLinkedObject calls this when it returns false, so you probably don't have to call this manually.
		*/
		void ResetObjectReferenceIterator();	

		/**
		@return The linked object with the passed name, otherwise nullptr.
		*/
		GameObjectPtr GetReferencedObjectByName(Ogre::String name);


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
		bool GetTransformingLinkedObjects() { return mTransformingLinkedObjects; }

		/**
		Returns whether the object is movable and should be included in a save file.
		*/
		bool IsStatic();

		///Tells all components that the object is assembled completely
		void FirePostInit();

		///Tells all components to change to an inactive state, where say don't use any resources.
		void FreeResources(bool free);

		//Scripting
		std::vector<ScriptParam> AddComponent(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> SetObjectProperty(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> GetObjectProperty(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> HasObjectProperty(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> SetObjectPosition(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> SetObjectOrientation(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> SetObjectScale(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> GetObjectName(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> SendObjectMessage(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> ReceiveObjectMessage(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> GetLinkedObjectByName(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> HasScriptListener(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> IsNpc(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> FreeResources(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> Object_Play3DSound(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> Object_GetDistToObject(Script& caller, std::vector<ScriptParam> &vParams);

		DEFINE_GOLUAMETHOD_H(AddComponent)
		DEFINE_GOLUAMETHOD_H(SetObjectProperty)
		DEFINE_GOLUAMETHOD_H(GetObjectProperty)
		DEFINE_GOLUAMETHOD_H(HasObjectProperty)
		DEFINE_GOLUAMETHOD_H(SetObjectPosition)
		DEFINE_GOLUAMETHOD_H(SetObjectOrientation)
		DEFINE_GOLUAMETHOD_H(SetObjectScale)
		DEFINE_GOLUAMETHOD_H(GetObjectName)
		DEFINE_GOLUAMETHOD_H(SendObjectMessage)
		DEFINE_GOLUAMETHOD_H(ReceiveObjectMessage)
		DEFINE_GOLUAMETHOD_H(GetLinkedObjectByName)
		DEFINE_GOLUAMETHOD_H(HasScriptListener)
		DEFINE_GOLUAMETHOD_H(IsNpc)
		DEFINE_GOLUAMETHOD_H(FreeResources)
		DEFINE_GOLUAMETHOD_H(Object_Play3DSound)
		DEFINE_GOLUAMETHOD_H(Object_GetDistToObject)

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

};