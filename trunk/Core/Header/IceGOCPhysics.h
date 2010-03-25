
#pragma once

#include "IceIncludes.h"
#include "IceGOComponent.h"
#include "IceGOCEditorInterface.h"
#include "OgrePhysX.h"

namespace Ice
{

	class DllExport GOCPhysics : public GOComponent
	{
	public:
		virtual ~GOCPhysics(void) {};
		goc_id_family& GetFamilyID() const { static std::string name = "Physics"; return name; }
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
		AI
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
		OgrePhysX::RenderedActor *mActor;
		GOPhysXRenderable *mRenderable;
		void Create(Ogre::String collision_mesh, float density, int shapetype, Ogre::Vector3 scale);
		Ogre::String mCollisionMeshName;
		float mDensity;
		int mShapeType;

	public:
		GOCRigidBody() { mActor = 0; mOwnerGO = 0; mRenderable = 0; }
		GOCRigidBody(Ogre::String collision_mesh, float density, int shapetype);
		~GOCRigidBody(void);

		GOComponent::goc_id_type& GetComponentID() const { static std::string name = "RigidBody"; return name; }

		void UpdatePosition(Ogre::Vector3 position);
		void UpdateOrientation(Ogre::Quaternion orientation);
		void UpdateScale(Ogre::Vector3 scale);

		void Freeze(bool freeze);

		void SetOwner(GameObject *go);
		bool IsStatic() { return false; }

		void CreateFromDataMap(DataMap *parameters);
		void GetParameters(DataMap *parameters);
		static void GetDefaultParameters(DataMap *parameters);
		Ogre::String GetLabel() { return "Rigid Body"; }

		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		std::string& TellName() { static std::string name = "RigidBody"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "RigidBody"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCRigidBody; };
		static GOCEditorInterface* NewEditorInterfaceInstance() { return new GOCRigidBody(); }
		void AttachToGO(GameObject *go);
	};


	class DllExport GOCStaticBody : public GOCEditorInterface, public GOCPhysics
	{
	private:
		OgrePhysX::Actor *mActor;
		void Create(Ogre::String collision_mesh, Ogre::Vector3 scale = Ogre::Vector3(1,1,1));
		Ogre::String mCollisionMeshName;

	public:
		GOCStaticBody() { mActor = 0; mOwnerGO = 0; }
		GOCStaticBody(Ogre::String collision_mesh);
		~GOCStaticBody(void);

		GOComponent::goc_id_type& GetComponentID() const { static std::string name = "StaticBody"; return name; }

		void UpdatePosition(Ogre::Vector3 position);
		void UpdateOrientation(Ogre::Quaternion orientation);
		void UpdateScale(Ogre::Vector3 scale);

		void SetOwner(GameObject *go);

		void CreateFromDataMap(DataMap *parameters);
		void GetParameters(DataMap *parameters);
		static void GetDefaultParameters(DataMap *parameters);
		void AttachToGO(GameObject *go);
		Ogre::String GetLabel() { return "Static Body"; }

		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		std::string& TellName() { static std::string name = "StaticBody"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "StaticBody"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCStaticBody; };
		static GOCEditorInterface* NewEditorInterfaceInstance() { return new GOCStaticBody(); }
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
		float mSphereRadius;
		void Create(Ogre::Vector3 scale);

	public:
		GOCTrigger() { mActor = 0; mOwnerGO = 0; }
		GOCTrigger(Ogre::Vector3 boxDimensions);
		GOCTrigger(float sphereRadius);
		~GOCTrigger(void);

		GOComponent::goc_id_type& GetComponentID() const { static std::string name = "Trigger"; return name; }

		void UpdatePosition(Ogre::Vector3 position);
		void UpdateOrientation(Ogre::Quaternion orientation);
		void UpdateScale(Ogre::Vector3 scale);

		void onEnter(GameObject *object);
		void onLeave(GameObject *object);

		void SetOwner(GameObject *go);

		void CreateFromDataMap(DataMap *parameters);
		void GetParameters(DataMap *parameters);
		static void GetDefaultParameters(DataMap *parameters);
		void AttachToGO(GameObject *go);
		Ogre::String GetLabel() { return "Trigger"; }

		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		std::string& TellName() { static std::string name = "Trigger"; return name; };
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "Trigger"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCTrigger; };
		static GOCEditorInterface* NewEditorInterfaceInstance() { return new GOCTrigger(); }
	};

};