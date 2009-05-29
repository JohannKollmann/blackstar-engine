
#pragma once

#include "SGTIncludes.h"
#include "NxOgre.h"
#include "SGTGameObject.h"
#include "SGTGOComponent.h"

class SGTDllExport SGTGOCRenderable : public NxOgre::Renderable
{
private:
	SGTGameObject *mOwner;

public:
	SGTGOCRenderable(SGTGameObject *owner) : Renderable(0) { mOwner = owner; }
	~SGTGOCRenderable() {}

	NxOgre::NxString getType() {return "Esgaroth-Renderable";}
	NxOgre::NxShortHashIdentifier getHashType() const {return 4179;}

	void setPose(const NxOgre::Pose& p)
	{
		mOwner->UpdateTransform(p, p);
		SGTObjectMsg *p_msg = new SGTObjectMsg();
		p_msg->mName = "Update_Transform";
		p_msg->mData.AddOgreVec3("Position", p);
		p_msg->mData.AddOgreQuat("Orientation", p);
		mOwner->SendMessage(Ogre::SharedPtr<SGTObjectMsg>(p_msg));
	}
	NxOgre::Pose getPose() const
	{
		return NxOgre::Pose(mOwner->GetGlobalPosition(), mOwner->GetGlobalOrientation());
	}
};

class SGTDllExport SGTGOCPhysics : public SGTGOComponent
{
public:
	virtual ~SGTGOCPhysics(void) {};
	goc_id_family& GetFamilyID() const { static std::string name = "GOCPhysics"; return name; }
};

namespace SGTShapes
{
	const int SHAPE_BOX = 0;
	const int SHAPE_SPHERE = 1;
	const int SHAPE_CONVEX = 2;
	const int SHAPE_NXS = 3;
	const int SHAPE_CAPSULE = 4;
};

class SGTDllExport SGTGOCRigidBody : public SGTGOCEditorInterface, public SGTGOComponent, public NxOgre::RenderableSource
{
private:
	NxOgre::Actor *mActor;
	void Create(Ogre::String collision_mesh, float density, int shapetype, Ogre::Vector3 scale);
	Ogre::String mCollisionMeshName;
	float mDensity;
	int mShapeType;

public:
	SGTGOCRigidBody() : SGTGOComponent() { mActor = 0; mRenderable = 0; }
	SGTGOCRigidBody(Ogre::String collision_mesh, float density, int shapetype);
	~SGTGOCRigidBody(void);

	goc_id_family& GetFamilyID() const { static std::string name = "GOCPhysics"; return name; }
	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "RigidBody"; return name; }

	void UpdatePosition(Ogre::Vector3 position);
	void UpdateOrientation(Ogre::Quaternion orientation);
	void UpdateScale(Ogre::Vector3 scale);

	void Freeze(bool freeze);

	void SetOwner(SGTGameObject *go);

	void CreateFromDataMap(SGTDataMap *parameters);
	void GetParameters(SGTDataMap *parameters);
	static void GetDefaultParameters(SGTDataMap *parameters);

	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	std::string& TellName() { static std::string name = "RigidBody"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "RigidBody"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTGOCRigidBody* NewInstance() { return new SGTGOCRigidBody; };


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

	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	std::string& TellName() { static std::string name = "StaticBody"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "StaticBody"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTGOCStaticBody* NewInstance() { return new SGTGOCStaticBody; };
};


#include "NxControllerManager.h"
#include "SGTMessageListener.h"

class SGTDllExport SGTGOCCharacterController : public SGTGOCPhysics, public SGTMessageListener
{
private:
	NxController *mCharacterController;
	void Create(Ogre::Vector3 dimensions);
	Ogre::Vector3 mDirection;

public:
	SGTGOCCharacterController() { mCharacterController = 0; }
	SGTGOCCharacterController(Ogre::Vector3 dimensions);
	~SGTGOCCharacterController(void);

	SGTGOComponent::goc_id_type& GetComponentID() const { static std::string name = "CharacterController"; return name; }

	void UpdatePosition(Ogre::Vector3 position);
	void UpdateOrientation(Ogre::Quaternion orientation);
	void UpdateScale(Ogre::Vector3 scale);

	void ReceiveObjectMessage(Ogre::SharedPtr<SGTObjectMsg> msg);
	void ReceiveMessage(SGTMsg &msg);

	void SetOwner(SGTGameObject *go);

	void Save(SGTSaveSystem& mgr);
	void Load(SGTLoadSystem& mgr);
	std::string& TellName() { static std::string name = "CharacterController"; return name; };
	static void Register(std::string* pstrName, SGTSaveableInstanceFn* pFn) { *pstrName = "CharacterController"; *pFn = (SGTSaveableInstanceFn)&NewInstance; };
	static SGTGOCCharacterController* NewInstance() { return new SGTGOCCharacterController; };
};
