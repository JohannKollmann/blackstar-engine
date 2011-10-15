
#pragma once

#include "IceIncludes.h"
#include "IceGOComponent.h"
#include "IceGOCEditorInterface.h"
#include "OgrePhysX.h"
#include "IceGOCScriptMakros.h"
#include "PxPhysicsAPI.h"
#include "IceSeeSense.h"

namespace Ice
{
	using namespace physx;

	class DllExport GOCPhysics : public GOComponent
	{
	public:
		virtual ~GOCPhysics(void) {};
		GOComponent::FamilyID& GetFamilyID() const { static std::string name = "Physics"; return name; }

		AccessPermissionID GetAccessPermissionID() { return AccessPermissions::ACCESS_PHYSICS; }
	};

	template<class T>
	class DllExport GOCPhysXActor : public GOCPhysics
	{
	protected:
		OgrePhysX::Actor<T> mActor;

	public:
		OgrePhysX::Actor<T>* GetActor()
		{
			return &mActor;
		}

		virtual void UpdatePosition(const Ogre::Vector3 &position)
		{
			mActor.setGlobalPosition(position);
		}
		virtual void UpdateOrientation(const Ogre::Quaternion &orientation)
		{
			mActor.setGlobalOrientation(orientation);
		}
	};

	namespace Shapes
	{
		const int SHAPE_BOX = 0;
		const int SHAPE_SPHERE = 1;
		const int SHAPE_CONVEX = 2;
		const int SHAPE_NXS = 3;
		const int SHAPE_CAPSULE = 4;
	};

	class DllExport GOCRigidBody : public GOCEditorInterface, public GOCPhysXActor<PxRigidDynamic>, public OgrePhysX::PointRenderable, public SeeSense::VisualObject
	{
	private:
		OgrePhysX::RenderedActorBinding *mRenderBinding;

		void Create(Ogre::String collision_mesh, float density, int shapetype, Ogre::Vector3 scale);
		Ogre::String mCollisionMeshName;
		Ogre::String mMaterialName;
		float mDensity;
		int mShapeType;
		bool mIsKinematic;
		bool mIsFreezed;

		void _clear();

	public:
		GOCRigidBody();
		GOCRigidBody(Ogre::String collision_mesh, float density, int shapetype);
		~GOCRigidBody(void);

		GOComponent::TypeID& GetComponentID() const { static std::string name = "RigidBody"; return name; }

		void UpdatePosition(const Ogre::Vector3 &position);
		void UpdateScale(const Ogre::Vector3 &scale);

		void setTransform(Ogre::Vector3 &position, Ogre::Quaternion &rotation);

		void Freeze(bool freeze);

		void SetOwner(std::weak_ptr<GameObject> go);
		bool IsStatic() { return false; }

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

		Ogre::String GetVisualObjectDescription();
		void GetTrackPoints(std::vector<Ogre::Vector3> &outPoints);

		std::vector<ScriptParam> Body_GetSpeed(Script& caller, std::vector<ScriptParam> &vParams);
		std::vector<ScriptParam> Body_AddImpulse(Script& caller, std::vector<ScriptParam> &vParams);
		DEFINE_GOCLUAMETHOD(GOCRigidBody, Body_GetSpeed)
		DEFINE_TYPEDGOCLUAMETHOD(GOCRigidBody, Body_AddImpulse, "float float float")
	};


	class DllExport GOCStaticBody : public GOCEditorInterface, public GOCPhysXActor<PxRigidStatic>, public SeeSense::VisualObject
	{
	private:
		void Create(Ogre::String collision_mesh, Ogre::Vector3 scale = Ogre::Vector3(1,1,1));
		Ogre::String mCollisionMeshName;

		void _clear();

	public:
		GOCStaticBody() {}
		GOCStaticBody(Ogre::String collision_mesh);
		~GOCStaticBody(void);

		GOComponent::TypeID& GetComponentID() const { static std::string name = "StaticBody"; return name; }

		void UpdateScale(const Ogre::Vector3 &scale);

		void SetOwner(std::weak_ptr<GameObject> go);

		void SetParameters(DataMap *parameters);
		void GetParameters(DataMap *parameters);
		void GetDefaultParameters(DataMap *parameters);
		Ogre::String GetLabel() { return "Static Body"; }
		GOComponent* GetGOComponent() { return this; }

		Ogre::String GetVisualObjectDescription();
		void GetTrackPoints(std::vector<Ogre::Vector3> &outPoints);

		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		std::string& TellName() { static std::string name = "StaticBody"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "StaticBody"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCStaticBody; };
		GOCEditorInterface* New() { return new GOCStaticBody(); }
	};

	class DllExport GOCTrigger : public GOCEditorInterface, public GOCPhysXActor<PxRigidStatic>
	{
		enum TriggerShapes
		{
			BOX = 0,
			SPHERE = 1
		};
	private:
		TriggerShapes mShapeType;
		Ogre::Vector3 mBoxDimensions;
		bool mActive;
		float mSphereRadius;
		void Create(Ogre::Vector3 scale);

		void _clear();

	public:
		GOCTrigger() { mSphereRadius = -1; }
		GOCTrigger(Ogre::Vector3 boxDimensions);
		GOCTrigger(float sphereRadius);
		~GOCTrigger(void);

		GOComponent::TypeID& GetComponentID() const { static std::string name = "Trigger"; return name; }

		void UpdateScale(const Ogre::Vector3 &scale);

		void onEnter(GameObject *object);
		void onLeave(GameObject *object);

		void SetOwner(std::weak_ptr<GameObject> go);

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