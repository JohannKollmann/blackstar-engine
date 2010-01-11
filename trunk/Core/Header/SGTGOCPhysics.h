
#pragma once

#include "SGTIncludes.h"
#include "SGTGOComponent.h"
#include "SGTGOCEditorInterface.h"
#include "OgrePhysX.h"

class SGTDllExport SGTGOCPhysics : public SGTGOComponent
{
public:
	virtual ~SGTGOCPhysics(void) {};
	goc_id_family& GetFamilyID() const { static std::string name = "Physics"; return name; }
};

namespace SGTShapes
{
	const int SHAPE_BOX = 0;
	const int SHAPE_SPHERE = 1;
	const int SHAPE_CONVEX = 2;
	const int SHAPE_NXS = 3;
	const int SHAPE_CAPSULE = 4;
};

enum SGTCollisionGroups
{
	DEFAULT,
	LEVELMESH,
	BONE
};

class SGTDllExport SGTGOPhysXRenderable : public OgrePhysX::PointRenderable
{
private:
	SGTGameObject *mGO;
public:
	SGTGOPhysXRenderable(SGTGameObject *mGO) : mGO(mGO) {};
	void setTransform(Ogre::Vector3 position, Ogre::Quaternion rotation);
	void setGO(SGTGameObject *go);
};

class SGTDllExport SGTGOCRigidBody : public SGTGOCEditorInterface, public SGTGOCPhysics
{
private:
	OgrePhysX::RenderedActor *mActor;
	SGTGOPhysXRenderable *mRenderable;
	void Create(Ogre::String collision_mesh, float density, int shapetype, Ogre::Vector3 scale);
	Ogre::String mCollisionMeshName;
	float mDensity;
	int mShapeType;

public:
	SGTGOCRigidBody() { mActor = 0; mOwnerGO = 0; mRenderable = 0; }
	SGTGOCRigidBody(Ogre::String collision_mesh, float density, int shapetype);
	~SGTGOCRigidBody(void);

	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "RigidBody"; return name; }

	void UpdatePosition(Ogre::Vector3 position);
	void UpdateOrientation(Ogre::Quaternion orientation);
	void UpdateScale(Ogre::Vector3 scale);

	void Freeze(bool freeze);

	void SetOwner(SGTGameObject *go);
	bool IsStatic() { return false; }

	void CreateFromDataMap(SGTDataMap *parameters);
	void GetParameters(SGTDataMap *parameters);
	static void GetDefaultParameters(SGTDataMap *parameters);
	Ogre::String GetLabel() { return "Rigid Body"; }

	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	std::string& TellName() { static std::string name = "RigidBody"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "RigidBody"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTSaveable* NewInstance() { return new SGTGOCRigidBody; };
	static SGTGOCEditorInterface* NewEditorInterfaceInstance() { return new SGTGOCRigidBody(); }
	void AttachToGO(SGTGameObject *go);
};


class SGTDllExport SGTGOCStaticBody : public SGTGOCEditorInterface, public SGTGOCPhysics
{
private:
	OgrePhysX::Actor *mActor;
	void Create(Ogre::String collision_mesh, Ogre::Vector3 scale = Ogre::Vector3(1,1,1));
	Ogre::String mCollisionMeshName;

public:
	SGTGOCStaticBody() { mActor = 0; mOwnerGO = 0; }
	SGTGOCStaticBody(Ogre::String collision_mesh);
	~SGTGOCStaticBody(void);

	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "StaticBody"; return name; }

	void UpdatePosition(Ogre::Vector3 position);
	void UpdateOrientation(Ogre::Quaternion orientation);
	void UpdateScale(Ogre::Vector3 scale);

	void SetOwner(SGTGameObject *go);

	void CreateFromDataMap(SGTDataMap *parameters);
	void GetParameters(SGTDataMap *parameters);
	static void GetDefaultParameters(SGTDataMap *parameters);
	void AttachToGO(SGTGameObject *go);
	Ogre::String GetLabel() { return "Static Body"; }

	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	std::string& TellName() { static std::string name = "StaticBody"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "StaticBody"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTSaveable* NewInstance() { return new SGTGOCStaticBody; };
	static SGTGOCEditorInterface* NewEditorInterfaceInstance() { return new SGTGOCStaticBody(); }
};

class SGTDllExport SGTGOCTrigger : public SGTGOCEditorInterface, public SGTGOCPhysics
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
	SGTGOCTrigger() { mActor = 0; mOwnerGO = 0; }
	SGTGOCTrigger(Ogre::Vector3 boxDimensions);
	SGTGOCTrigger(float sphereRadius);
	~SGTGOCTrigger(void);

	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "Trigger"; return name; }

	void UpdatePosition(Ogre::Vector3 position);
	void UpdateOrientation(Ogre::Quaternion orientation);
	void UpdateScale(Ogre::Vector3 scale);

	void onEnter(SGTGameObject *object);
	void onLeave(SGTGameObject *object);

	void SetOwner(SGTGameObject *go);

	void CreateFromDataMap(SGTDataMap *parameters);
	void GetParameters(SGTDataMap *parameters);
	static void GetDefaultParameters(SGTDataMap *parameters);
	void AttachToGO(SGTGameObject *go);
	Ogre::String GetLabel() { return "Trigger"; }

	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	std::string& TellName() { static std::string name = "Trigger"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "Trigger"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTSaveable* NewInstance() { return new SGTGOCTrigger; };
	static SGTGOCEditorInterface* NewEditorInterfaceInstance() { return new SGTGOCTrigger(); }
};