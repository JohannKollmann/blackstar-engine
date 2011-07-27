
#pragma once

#include "IceIncludes.h"
#include "IceGOComponent.h"
#include "IceGOCEditorInterface.h"
#include "OgrePhysX.h"
#include "IceGOCScriptMakros.h"

namespace Ice
{

	class DllExport GOCPhysics : public GOComponent
	{
	protected:
		

	public:
		virtual ~GOCPhysics(void) {};
		GOComponent::FamilyID& GetFamilyID() const { static std::string name = "Physics"; return name; }

		virtual OgrePhysX::Actor* GetActor() = 0;

		AccessPermissionID GetAccessPermissionID() { return AccessPermissions::ACCESS_PHYSICS; }

		std::vector<ScriptParam> Body_GetSpeed(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> Body_AddImpulse(Script& caller, std::vector<ScriptParam> &vParams);
		DEFINE_GOCLUAMETHOD(GOCPhysics, Body_GetSpeed)
		DEFINE_TYPEDGOCLUAMETHOD(GOCPhysics, Body_AddImpulse, "float float float")
	};

	namespace Shapes
	{
		const int SHAPE_BOX = 0;
		const int SHAPE_SPHERE = 1;
		const int SHAPE_CONVEX = 2;
		const int SHAPE_NXS = 3;
		const int SHAPE_CAPSULE = 4;
	};

	enum CollisionGroups
	{
		DEFAULT,
		LEVELMESH,
		BONE,
		CHARACTER,
		AI,
		TRIGGER,
		TMP
	};

	class DllExport GOPhysXRenderable : public OgrePhysX::PointRenderable
	{
	private:
		GOCRigidBody *mBody;
	public:
		GOPhysXRenderable(GOCRigidBody *body) : mBody(body) {};
		virtual ~GOPhysXRenderable() {}
		void setTransform(Ogre::Vector3 position, Ogre::Quaternion rotation);

	};

	class DllExport GOCRigidBody : public GOCEditorInterface, public GOCPhysics
	{
	private:
		OgrePhysX::Actor *mActor;
		GOPhysXRenderable *mRenderable;
		void Create(Ogre::String collision_mesh, float density, int shapetype, Ogre::Vector3 scale);
		Ogre::String mCollisionMeshName;
		Ogre::String mMaterialName;
		float mDensity;
		int mShapeType;
		bool mIsKinematic;

		void _clear();

	public:
		GOCRigidBody() : mActor(nullptr), mRenderable(nullptr), mIsKinematic(false) {}
		GOCRigidBody(Ogre::String collision_mesh, float density, int shapetype);
		~GOCRigidBody(void);

		GOComponent::TypeID& GetComponentID() const { static std::string name = "RigidBody"; return name; }

		void UpdatePosition(Ogre::Vector3 position);
		void UpdateOrientation(Ogre::Quaternion orientation);
		void UpdateScale(Ogre::Vector3 scale);

		void Freeze(bool freeze);

		void SetOwner(std::weak_ptr<GameObject> go);
		bool IsStatic() { return false; }

		OgrePhysX::Actor* GetActor() { return mActor; }

		void SetParameters(DataMap *parameters);
		void GetParameters(DataMap *parameters);
		void GetDefaultParameters(DataMap *parameters);
		Ogre::String GetLabel() { return "Rigid Body"; }
		GOComponent* GetGOComponent() { return this; }

		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		std::string& TellName() { static std::string name = "RigidBody"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "RigidBody"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCRigidBody; };
		GOCEditorInterface* New() { return new GOCRigidBody(); }
	};


	class DllExport GOCStaticBody : public GOCEditorInterface, public GOCPhysics
	{
	private:
		OgrePhysX::Actor *mActor;
		void Create(Ogre::String collision_mesh, Ogre::Vector3 scale = Ogre::Vector3(1,1,1));
		Ogre::String mCollisionMeshName;

		void _clear();

	public:
		GOCStaticBody() { mActor = 0; mOwnerGO; }
		GOCStaticBody(Ogre::String collision_mesh);
		~GOCStaticBody(void);

		GOComponent::TypeID& GetComponentID() const { static std::string name = "StaticBody"; return name; }

		void UpdatePosition(Ogre::Vector3 position);
		void UpdateOrientation(Ogre::Quaternion orientation);
		void UpdateScale(Ogre::Vector3 scale);

		OgrePhysX::Actor* GetActor() { return mActor; }

		void SetOwner(std::weak_ptr<GameObject> go);

		void SetParameters(DataMap *parameters);
		void GetParameters(DataMap *parameters);
		void GetDefaultParameters(DataMap *parameters);
		Ogre::String GetLabel() { return "Static Body"; }
		GOComponent* GetGOComponent() { return this; }

		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		std::string& TellName() { static std::string name = "StaticBody"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "StaticBody"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCStaticBody; };
		GOCEditorInterface* New() { return new GOCStaticBody(); }
	};

	class DllExport GOCTrigger : public GOCEditorInterface, public GOCPhysics
	{
		enum TriggerShapes
		{
			BOX = 0,
			SPHERE = 1
		};
	private:
		OgrePhysX::Actor *mActor;
		TriggerShapes mShapeType;
		Ogre::Vector3 mBoxDimensions;
		bool mActive;
		float mSphereRadius;
		void Create(Ogre::Vector3 scale);

		void _clear();

	public:
		GOCTrigger() { mActor = nullptr; mOwnerGO; mSphereRadius = -1; }
		GOCTrigger(Ogre::Vector3 boxDimensions);
		GOCTrigger(float sphereRadius);
		~GOCTrigger(void);

		GOComponent::TypeID& GetComponentID() const { static std::string name = "Trigger"; return name; }

		void UpdatePosition(Ogre::Vector3 position);
		void UpdateOrientation(Ogre::Quaternion orientation);
		void UpdateScale(Ogre::Vector3 scale);

		void onEnter(GameObject *object);
		void onLeave(GameObject *object);

		void SetOwner(std::weak_ptr<GameObject> go);

		OgrePhysX::Actor* GetActor() { return mActor; }

		void SetParameters(DataMap *parameters);
		void GetParameters(DataMap *parameters);
		void GetDefaultParameters(DataMap *parameters);
		Ogre::String GetLabel() { return "Trigger"; }
		GOComponent* GetGOComponent() { return this; }

		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		std::string& TellName() { static std::string name = "Trigger"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "Trigger"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCTrigger; };
		GOCEditorInterface* New() { return new GOCTrigger(); }

		std::vector<ScriptParam> Trigger_SetActive(Script& caller, std::vector<ScriptParam> &vParams);
		DEFINE_TYPEDGOCLUAMETHOD(GOCTrigger, Trigger_SetActive, "bool")
	};

};