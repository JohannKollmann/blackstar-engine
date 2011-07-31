
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
			OWNER = 1,			//deletes referenced object when object is deleted
			OWNED = 2,			//Identifier flag for example for edit tree
			PERSISTENT = 4,		//loadsaves referenced object
			MOVEIT = 8,			//moves referenced object (parent-child style)
			MOVEIT_USER = 16,	//moves referenced object (parent-child style) only when user explicitly specifies it (see SetGlobalPosition / SetGlobalOrientation).  
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
	class DllExport GameObject : public LoadSave::Saveable, public Transformable3D, public IndependantMessageListener
	{
	public:
		enum ReferenceTypes
		{
			PARENT = 11235813
		};
		enum MessageIDs
		{
			UPDATE_COMPONENT_POSITION = 5500,
			UPDATE_COMPONENT_ORIENTATION = 5501,
			UPDATE_COMPONENT_SCALE = 5502,
			UPDATE_COMPONENT_TRANSFORM = 5503
		};

	protected:
		int mID;
		std::weak_ptr<GameObject> mWeakThis;

		//Name
		Ogre::String mName;

		bool mSelectable;
		bool mFreezed;

		bool mComponentsNeedUpdate;

		bool mTransformingReferencedObjects;

		//Global and local transform
		Ogre::Vector3 mPosition;
		Ogre::Quaternion mOrientation;
		Ogre::Vector3 mLocalPosition;
		Ogre::Quaternion mLocalOrientation;
		Ogre::Vector3 mScale;

		//Components
		std::vector<GOComponentPtr> mComponents;

		//Referenced objects
		std::vector<ObjectReferencePtr> mReferencedObjects;
		std::vector<ObjectReferencePtr>::iterator mReferencedObjectsInternIterator;

	public:
		GameObject();
		virtual ~GameObject();

		/**
		* Sets the weak "this" pointer. The factory responsible for the game object must call this function!
		* @pre wThis.lock().get() == this
		*/
		void SetWeakThis(std::weak_ptr<LoadSave::Saveable> wThis);

		/**
		* @return The name of the object.
		*/
		Ogre::String GetName() { return mName; }

		/**
		* Sets the name of the object.
		* @param name The new name of the object.
		*/
		void SetName(Ogre::String name) { mName = name; }

		/**
		* @return The unique ID of the object.
		*/
		int GetID() { return mID; }

		/**
		* @return The unique ID of the object as string.
		*/
		Ogre::String GetIDStr() { return Ogre::StringConverter::toString(mID); }

		///Broadcasts a message to all components except sender (optional).
		void BroadcastObjectMessage(Msg &msg, GOComponent *sender = nullptr, bool sendInstantly = false);

		///Sends a message to a component of a certain family, if existent.
		void SendObjectMessage(Msg &msg, GOComponent::FamilyID &familyID, bool sendInstantly = false);

		/**
		* Attaches a component to the object.
		* @param component The component.
		*/
		void AddComponent(GOComponentPtr component);

		///Removes the component of family familyID.
		void RemoveComponent(const GOComponent::FamilyID& familyID);

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

		/**
		* Returns the component of type T if it exists, otherwise it creates the component, attaches it to the object and retrieves it.
		* T must have a default constructor.
		*/
		template<class T>
		T* CreateOrRetrieveComponent()
		{
			for (auto i = mComponents.begin(); i != mComponents.end(); i++)
			{
				T *rtti = dynamic_cast<T*>((*i).get());
				if (rtti) return rtti;
			}
			std::shared_ptr<T> goc = std::make_shared<T>();
			AddComponent(goc);
			return goc.get();
		}

		///Returns the component of family familyID if it exists, otherwise nullptr.
		GOComponent* GetComponent(const GOComponent::FamilyID& familyID);

		///Returns the shared component ptr of family familyID if it exists, otherwise an empty GOComponentPtr().
		GOComponentPtr GetComponentPtr(const GOComponent::FamilyID& familyID);

		///Returns the component of family familyID and type typeID if it exists, otherwise nullptr.
		GOComponent* GetComponent(const GOComponent::FamilyID& familyID, GOComponent::TypeID typeID);

		///Provides a mechanism to iterate over the components attached to the object.
		std::vector<GOComponentPtr>::iterator GetComponentIterator() { return mComponents.begin(); }
		///Provides a mechanism to iterate over the components attached to the object.
		std::vector<GOComponentPtr>::iterator GetComponentIteratorEnd() { return mComponents.end(); }

		///Detaches and deletes all attaches components.
		void ClearGOCs();

		///Deletes all referenced objects that are owned by this object.
		void ClearOwnedObjects();

		/**
		* Creates a directional link between two objects.
		* @param other The other GameObject.
		* @param flags Characterizes the relationship between the two objects, see ObjectReference.
		* @pre It is NOT allowed that the MOVER or OWNER flag is set bidirectionally.
		*/
		void AddObjectReference(const GameObjectPtr &other, unsigned int flags = 0, unsigned int userID = 0);

		///Removes all object references with ObjectReference.Object == object.
		void RemoveObjectReferences(GameObject *object);

		///Removes all object references with ObjectReference.UserID == userID.
		void RemoveObjectReferences(unsigned int userID);

		///Retrieves all object references with ObjectReference.UserID == userID.
		void GetReferencedObjects(unsigned int userID, std::vector<GameObjectPtr> &out);

		///Retrieves all object references with ObjectReference.UserID == userID.
		void GetReferencedObjects(unsigned int userID, std::list<GameObjectPtr> &out);

		///Retrieves all object references with ObjectReference.Flags & flags.
		void GetReferencedObjectsByFlag(unsigned int flags, std::vector<GameObjectPtr> &out);

		///Retrieves all object references with ObjectReference.Flags & flags.
		void GetReferencedObjectsByFlag(unsigned int flags, std::list<GameObjectPtr> &out);

		///Creates a parent-child relationship between two objects.
		void SetParent(GameObjectPtr parent);

		///Retrieves the parent object if existent, otherwise an empty GameObjectPtr.
		GameObjectPtr GetParent();

		/**
		* Provides a mechanism to iterate over the referenced objects.
		* example usage: 
		* while (obj->HasNextObjectReference()) { link = GetNextObjectReference(); }
		*/
		bool HasNextObjectReference();
		/**
		* @see HasNextObjectReference
		*/
		std::shared_ptr<ObjectReference> GetNextObjectReference();

		/**
		* Sets the referenced object intern iterator to the beginning.
		* HasNextObjectReference calls this when it returns false, so you probably don't have to call this manually.
		*/
		void ResetObjectReferenceIterator();	

		/**
		* @return The referenced object with the passed name, otherwise nullptr.
		*/
		GameObjectPtr GetReferencedObjectByName(Ogre::String name);


		Ogre::Vector3 GetGlobalPosition() { return mPosition; }
		Ogre::Quaternion GetGlobalOrientation() { return mOrientation; }	
		Ogre::Vector3 GetGlobalScale() { return mScale; }

		/** Sets the global position of the object.
		* @param moveReferences Specifies whether object references with the flag MOVEIT_USER shall be moved relatively.
		* @param moveChildren Specifies whether object references with the flag MOVEIT shall be moved relatively.
		* @param referenceBlacklist	Pass an empty std::set<GameObject*> if you want to be sure that an object referenced transitively is not moved twice.
									This Parameter is only relevant when moveReferences is true.
		*/
		void SetGlobalPosition(const Ogre::Vector3 &pos, bool updateComponents, bool moveReferences, bool moveChildren = true, std::set<GameObject*> *referenceBlacklist = nullptr);

		///Sets the global position of the object.
		void SetGlobalPosition(const Ogre::Vector3 &pos) { SetGlobalPosition(pos, true, false, true); }

		/** Sets the global orientation of the object.
		* @param moveReferences Specifies whether object references with the flag MOVEIT_USER shall be moved relatively.
		* @param moveChildren Specifies whether object references with the flag MOVEIT shall be moved relatively.
		* @param referenceBlacklist	Pass an empty std::set<GameObject*> if you want to be sure that an object referenced transitively is not moved twice.
									This Parameter is only relevant when moveReferences is true.
		*/
		void SetGlobalOrientation(const Ogre::Quaternion &quat, bool updateComponents, bool moveReferences, bool moveChildren = true, std::set<GameObject*> *referenceBlacklist = nullptr);

		///Sets the global orientation of the object.
		void SetGlobalOrientation(const Ogre::Quaternion &quat) { SetGlobalOrientation(quat, true, false, true); }

		/** Translates the object in global space of the object.
		* @see SetGlobalPosition
		*/
		void Translate(Ogre::Vector3 vec, bool moveReferences = false, bool moveChildren = true, std::set<GameObject*> *referenceBlacklist = nullptr) { SetGlobalPosition(mPosition + vec, true, moveReferences, moveChildren, referenceBlacklist); }

		/** Translates the object in global space of the object.
		* @see SetGlobalOrientation
		*/
		void Rotate(Ogre::Vector3 axis, Ogre::Radian angle, bool moveReferences = false, bool moveChildren = true, std::set<GameObject*> *referenceBlacklist = nullptr) { Ogre::Quaternion q; q.FromAngleAxis(angle, axis); SetGlobalOrientation(mOrientation * q, true, moveReferences, moveChildren, referenceBlacklist); }

		///Sets the scale of the object.
		void SetGlobalScale(const Ogre::Vector3 &scale, bool updateComponents);
		void SetGlobalScale(const Ogre::Vector3 &scale) { SetGlobalScale(scale, true); }

		///Changes the scale of the object. scaleoffset is added to the current scale.
		void Rescale(Ogre::Vector3 scaleoffset) { SetGlobalScale(mScale + scaleoffset, true); }

		///Retrieves wether the object is currently transforming referenced objects.
		bool GetTransformingReferencedObjects() { return mTransformingReferencedObjects; }

		///Returns whether the object is movable and should be included in a save file.
		bool IsStatic();

		///Tells all components that the object is assembled completely.
		void FirePostInit();

		///Tells all components to change to an inactive state, where say don't use any resources.
		void FreeResources(bool free);

		void ReceiveMessage(Msg &msg);

		//Scripting
		std::vector<ScriptParam> AddComponent(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> SetParent(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> Object_SetProperty(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> Object_GetProperty(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> Object_HasProperty(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> SetObjectPosition(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> SetObjectOrientation(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> SetObjectScale(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> GetObjectName(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> SendObjectMessage(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> ReceiveObjectMessage(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> GetReferencedObjectByName(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> HasScriptListener(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> IsNpc(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> FreeResources(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> Object_Play3DSound(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> Object_GetDistToObject(Script& caller, std::vector<ScriptParam> &vParams);

		DEFINE_GOLUAMETHOD_H(AddComponent)
		DEFINE_GOLUAMETHOD_H(SetParent)
		DEFINE_GOLUAMETHOD_H(Object_SetProperty)
		DEFINE_GOLUAMETHOD_H(Object_GetProperty)
		DEFINE_GOLUAMETHOD_H(Object_HasProperty)
		DEFINE_GOLUAMETHOD_H(SetObjectPosition)
		DEFINE_GOLUAMETHOD_H(SetObjectOrientation)
		DEFINE_GOLUAMETHOD_H(SetObjectScale)
		DEFINE_GOLUAMETHOD_H(GetObjectName)
		DEFINE_GOLUAMETHOD_H(SendObjectMessage)
		DEFINE_GOLUAMETHOD_H(ReceiveObjectMessage)
		DEFINE_GOLUAMETHOD_H(GetReferencedObjectByName)
		DEFINE_GOLUAMETHOD_H(HasScriptListener)
		DEFINE_GOLUAMETHOD_H(IsNpc)
		DEFINE_GOLUAMETHOD_H(FreeResources)
		DEFINE_GOLUAMETHOD_H(Object_Play3DSound)
		DEFINE_GOLUAMETHOD_H(Object_GetDistToObject)

		//properties
		std::map<Ogre::String, ScriptParam> mScriptProperties;

		//Editor stuff

		///Freezed / unfreezes the object.
		void Freeze(bool freeze);

		///Shows / hides editor visualisation.
		void ShowEditorVisuals(bool show);

		///Retrieves, whether the object is selectable in an editor.
		bool IsSelectable() { return mSelectable; }

		///Sets whether the object is selectable in an editor.
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