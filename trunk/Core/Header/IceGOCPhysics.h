
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

		void setTransform(const Ogre::Vector3 &position, const Ogre::Quaternion &rotation);

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

	class DllExport GOCDestructible : public GOCPhysics, public GOCStaticEditorInterface, public OgrePhysX::PointRenderable, public SeeSense::VisualObject
	{
	private:
		OgrePhysX::Destructible *mDestructible;

		Ogre::String mConfigFile;
		Ogre::String mMaterialName;
		float mDensity;
		float mMaxForce;
		float mMaxTorque;
		bool mIsFreezed;

		void _create();
		void _clear();

	public:
		GOCDestructible() : mDestructible(nullptr), mIsFreezed(false) {}
		~GOCDestructible();

		GOComponent::TypeID& GetComponentID() const { static std::string name = "Destructible"; return name; }

		void UpdatePosition(const Ogre::Vector3 &position);
		void UpdateScale(const Ogre::Vector3 &scale);

		void setTransform(const Ogre::Vector3 &position, const Ogre::Quaternion &rotation);

		void Freeze(bool freeze);

		void NotifyOwnerGO();
		bool IsStatic() { return false; }

		BEGIN_GOCEDITORINTERFACE(GOCDestructible, "Destructible")
			PROPERTY_STRING(mConfigFile, "Split file", ".xml");
			PROPERTY_STRING(mMaterialName, "Material", "Wood");
			PROPERTY_FLOAT(mDensity, "Density", 100.0f);
			PROPERTY_FLOAT(mMaxForce, "Max Force", 1000.0f);
			PROPERTY_FLOAT(mMaxTorque, "Max Torque", 1000.0f);
		END_GOCEDITORINTERFACE

		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		std::string& TellName() { static std::string name = "Destructible"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "Destructible"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCDestructible(); };

		Ogre::String GetVisualObjectDescription();
		void GetTrackPoints(std::vector<Ogre::Vector3> &outPoints);
	};

	class DllExport GOCParticleChain : public GOCPhysics, public GOCStaticEditorInterface
	{
	private:
		OgrePhysX::ParticleChain *mParticleChain;
		OgrePhysX::ParticleChainDebugVisual *mDebugVisual;
		int mNumParticles;
		float mChainLength;
		float mParticleMass;
		float mPointDamping;
		float mPBDPointDamping;
		float mFTLDamping;
		float mSpringDamping;
		float mSpringStiffness;
		float mChainStiffness;
		int mIterationCount;
		float mOverRelaxation;
		void _create();
		void _clear();
		DataMap::Enum mSimulationMethod;

	public:
		GOCParticleChain() : mParticleChain(nullptr), mDebugVisual(nullptr) {}
		virtual ~GOCParticleChain() { _clear(); }

		void NotifyPostInit();

		void UpdatePosition(const Ogre::Vector3 &position);

		BEGIN_GOCEDITORINTERFACE(GOCParticleChain, "ParticleChain")
			PROPERTY_INT(mNumParticles, "Num particles", 30);
			PROPERTY_FLOAT(mChainLength, "Chain length", 2.0f);
			PROPERTY_FLOAT(mParticleMass, "Particle mass", 0.01f);
			PROPERTY_FLOAT(mPointDamping, "Point damping", 0.02f);
			PROPERTY_FLOAT(mPBDPointDamping, "PBD point damping", 0.0f);
			PROPERTY_FLOAT(mFTLDamping, "FTL damping", 0.9f);
			PROPERTY_FLOAT(mSpringDamping, "Spring damping", 0.4f);
			PROPERTY_FLOAT(mSpringStiffness, "Spring stiffness 1", 300.0f);
			PROPERTY_FLOAT(mChainStiffness, "Chain stiffness", 0.0f);
			PROPERTY_INT(mIterationCount, "Iteration Count", 4);
			PROPERTY_FLOAT(mOverRelaxation, "Over relaxation", 1.6f);
			PROPERTY_ENUM(mSimulationMethod, "Simulation method", DataMap::Enum("0 FTL Springs PBD "));
		END_GOCEDITORINTERFACE
		Ogre::String& GetComponentID() const { static Ogre::String name = "ParticleChain"; return name; };

		virtual void Save(LoadSave::SaveSystem& mgr) {}
		virtual void Load(LoadSave::LoadSystem& mgr) {}
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "ParticleChain"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCParticleChain(); };
	};

};