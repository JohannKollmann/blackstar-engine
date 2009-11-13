
#pragma once

#include "SGTIncludes.h"
#include "NxOgre.h"
#include "SGTGOComponent.h"
#include "SGTGOCEditorInterface.h"

class SGTDllExport SGTGOCRenderable : public NxOgre::Renderable
{
private:
	SGTGameObject *mOwner;

public:
	SGTGOCRenderable(SGTGameObject *owner) : Renderable(0) { mOwner = owner; }
	~SGTGOCRenderable() {}

	NxOgre::NxString getType() {return "Esgaroth-Renderable";}
	NxOgre::NxShortHashIdentifier getHashType() const {return 4179;}

	void setPose(const NxOgre::Pose& p);
	NxOgre::Pose getPose() const;
};

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

class SGTDllExport SGTGOCRigidBody : public SGTGOCEditorInterface, public SGTGOCPhysics, public NxOgre::RenderableSource
{
private:
	NxOgre::Actor *mActor;
	void Create(Ogre::String collision_mesh, float density, int shapetype, Ogre::Vector3 scale);
	Ogre::String mCollisionMeshName;
	float mDensity;
	int mShapeType;

public:
	SGTGOCRigidBody() { mActor = 0; mRenderable = 0; }
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

	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	std::string& TellName() { static std::string name = "RigidBody"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "RigidBody"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTSaveable* NewInstance() { return new SGTGOCRigidBody; };
	static SGTGOCEditorInterface* NewEditorInterfaceInstance() { return new SGTGOCRigidBody(); }
	void AttachToGO(SGTGameObject *go);

	// Required by the RenderableSource. (Tells where the Actor is)
	NxOgre::Pose getSourcePose(const NxOgre::TimeStep&) const {
		return mActor->getGlobalPose();
	}
	// To work out what exactly the class is, if the pointer is passed
	// around as an Actor.
	NxOgre::NxShortHashIdentifier getType() const {
		return 12445;
	}
	// Same as above, but uses a String. It is slower to check this way.
	NxOgre::NxString getStringType() const {
	   return "EsgarothActor";
	}
};


class SGTDllExport SGTGOCStaticBody : public SGTGOCEditorInterface, public SGTGOCPhysics
{
private:
	NxOgre::Actor *mActor;
	void Create(Ogre::String collision_mesh);
	Ogre::String mCollisionMeshName;

public:
	SGTGOCStaticBody() { mActor = 0; }
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

	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	std::string& TellName() { static std::string name = "StaticBody"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "StaticBody"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTSaveable* NewInstance() { return new SGTGOCStaticBody; };
	static SGTGOCEditorInterface* NewEditorInterfaceInstance() { return new SGTGOCStaticBody(); }
};