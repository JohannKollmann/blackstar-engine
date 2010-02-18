
#include "IceGOCPhysics.h"
#include "IceMain.h"
#include "IceSceneManager.h"
#include "IceGameObject.h"

namespace Ice
{

void GOPhysXRenderable::setTransform(Ogre::Vector3 position, Ogre::Quaternion rotation)
{
	if (mGO) mGO->UpdateTransform(position, rotation);
}
void GOPhysXRenderable::setGO(GameObject *go)
{
	mGO = go;
}


GOCRigidBody::GOCRigidBody(Ogre::String collision_mesh, float density, int shapetype)
{
	mCollisionMeshName = collision_mesh;
	mDensity = density;
	mShapeType = shapetype;
	mActor = 0;
	mRenderable = 0;
	mOwnerGO = 0;
}

GOCRigidBody::~GOCRigidBody(void)
{
	Main::Instance().GetPhysXScene()->destroyRenderedActor(mActor);
	//This also destroys the renderable!
}

void GOCRigidBody::Create(Ogre::String collision_mesh, float density, int shapetype, Ogre::Vector3 scale)
{
	Ogre::String internname = "RigidBody" + Ogre::StringConverter::toString(SceneManager::Instance().RequestID());
	mCollisionMeshName = collision_mesh;
	mDensity = density;
	if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", mCollisionMeshName))
	{
		Ogre::LogManager::getSingleton().logMessage("Error: Resource \"" + mCollisionMeshName + "\" does not exist. Loading dummy Resource...");
		mCollisionMeshName = "DummyMesh.mesh";
	}
	Ogre::Entity *entity = Main::Instance().GetOgreSceneMgr()->createEntity(internname, mCollisionMeshName);
	if (!mRenderable) mRenderable = new GOPhysXRenderable(mOwnerGO);
	else mRenderable->setGO(mOwnerGO);
	if (mShapeType ==  Shapes::SHAPE_SPHERE)
	{
		float sphereShapeRadius = entity->getBoundingRadius();
		mActor = Main::Instance().GetPhysXScene()->createRenderedActor(
			mRenderable,
			OgrePhysX::SphereShape(sphereShapeRadius * ((scale.x + scale.y + scale.z) / 3)).density(mDensity).group(CollisionGroups::DEFAULT));
	}
	else if (mShapeType == Shapes::SHAPE_CAPSULE)
	{
		Ogre::Vector3 cubeShapeSize = entity->getBoundingBox().getSize();
		cubeShapeSize = cubeShapeSize * scale;
		float capsule_radius = cubeShapeSize.x > cubeShapeSize.z ? cubeShapeSize.x : cubeShapeSize.z;
		float offset = 0.0f;
		if (cubeShapeSize.y - capsule_radius > 0.0f) offset = (cubeShapeSize.y / capsule_radius) * 0.1;
		mActor = Main::Instance().GetPhysXScene()->createRenderedActor(
			mRenderable,
			OgrePhysX::CapsuleShape(capsule_radius * 0.5, cubeShapeSize.y * 0.5 + offset).density(mDensity).group(CollisionGroups::DEFAULT));
	}
	else		//Default: Box
	{
		mActor = Main::Instance().GetPhysXScene()->createRenderedActor(
			mRenderable,
			OgrePhysX::BoxShape(entity, scale).density(mDensity).group(CollisionGroups::DEFAULT));
	}
	Main::Instance().GetOgreSceneMgr()->destroyEntity(entity);
}

void GOCRigidBody::Freeze(bool freeze)
{
	if (freeze)
	{
		mActor->getNxActor()->raiseBodyFlag(NX_BF_DISABLE_GRAVITY);	
		mActor->getNxActor()->raiseBodyFlag(NX_BF_FROZEN);	
	}
	else
	{
		mActor->getNxActor()->clearBodyFlag(NX_BF_DISABLE_GRAVITY);	
		mActor->getNxActor()->clearBodyFlag(NX_BF_FROZEN);	
		mActor->getNxActor()->wakeUp();
	}
}

void GOCRigidBody::UpdatePosition(Ogre::Vector3 position)
{
	if (!mOwnerGO->GetTranformingComponents()) mActor->setGlobalPosition(position);
}
void GOCRigidBody::UpdateOrientation(Ogre::Quaternion orientation)
{
	if (!mOwnerGO->GetTranformingComponents()) mActor->setGlobalOrientation(orientation);
}
void GOCRigidBody::UpdateScale(Ogre::Vector3 scale)
{
	if (mActor) Main::Instance().GetPhysXScene()->destroyRenderedActor(mActor);
	mRenderable = 0;
	Create(mCollisionMeshName, mDensity, mShapeType, scale);
	mActor->setGlobalOrientation(mOwnerGO->GetGlobalOrientation());
	mActor->setGlobalPosition(mOwnerGO->GetGlobalPosition());
}

void GOCRigidBody::SetOwner(GameObject *go)
{
	mOwnerGO = go;
	UpdateScale(mOwnerGO->GetGlobalScale());
	mActor->getNxActor()->userData = mOwnerGO;
	mActor->setGlobalOrientation(mOwnerGO->GetGlobalOrientation());
	mActor->setGlobalPosition(mOwnerGO->GetGlobalPosition());
}

void GOCRigidBody::CreateFromDataMap(DataMap *parameters)
{
	mCollisionMeshName = parameters->GetOgreString("CollisionMeshFile");
	Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
	scale = parameters->GetOgreVec3("Scale");
	mDensity = parameters->GetFloat("Density");
	mShapeType = parameters->GetInt("ShapeType");
	if (mOwnerGO) Create(mCollisionMeshName, mDensity, mShapeType, scale);
}
void GOCRigidBody::GetParameters(DataMap *parameters)
{
	parameters->AddOgreString("CollisionMeshFile", mCollisionMeshName);
	parameters->AddFloat("Density", mDensity);
	parameters->AddInt("ShapeType", mShapeType);
}
void GOCRigidBody::GetDefaultParameters(DataMap *parameters)
{
	parameters->AddOgreString("CollisionMeshFile", "");
	parameters->AddFloat("Density", 10.0f);
	parameters->AddInt("ShapeType", 0);
}

void GOCRigidBody::AttachToGO(GameObject *go)
{
	go->RemoveComponent(GetFamilyID());
	go->AddComponent(this);
}

void GOCRigidBody::Save(LoadSave::SaveSystem& mgr)
{
	mgr.SaveAtom("Ogre::String", &mCollisionMeshName, "CollisionMeshFile");
	mgr.SaveAtom("Ogre::Vector3", &mOwnerGO->GetGlobalScale(), "Scale");
	mgr.SaveAtom("float", &mDensity, "Density");
	mgr.SaveAtom("int", &mShapeType, "ShapeType");
}
void GOCRigidBody::Load(LoadSave::LoadSystem& mgr)
{
	mgr.LoadAtom("Ogre::String", &mCollisionMeshName);
	Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
	mgr.LoadAtom("Ogre::Vector3", &scale);
	mgr.LoadAtom("float", &mDensity);
	mgr.LoadAtom("int", &mShapeType);
	Create(mCollisionMeshName, mDensity, mShapeType, scale);
}


//Static Body

GOCStaticBody::GOCStaticBody(Ogre::String collision_mesh)
{
	mCollisionMeshName = collision_mesh;
	mActor = 0;
	mOwnerGO = 0;
}

GOCStaticBody::~GOCStaticBody(void)
{
	if (mActor)
	{
		Main::Instance().GetPhysXScene()->destroyActor(mActor);
	}
}

void GOCStaticBody::Create(Ogre::String collision_mesh, Ogre::Vector3 scale)
{
	Ogre::String internname = "StaticBody" + Ogre::StringConverter::toString(SceneManager::Instance().RequestID());
	mCollisionMeshName = collision_mesh;
	if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", mCollisionMeshName))
	{
		Ogre::LogManager::getSingleton().logMessage("Error: Resource \"" + mCollisionMeshName + "\" does not exist. Loading dummy Resource...");
		mCollisionMeshName = "DummyMesh.mesh";
	}
	Ogre::Entity *entity = Main::Instance().GetOgreSceneMgr()->createEntity("tempCollisionModell", mCollisionMeshName);
	mActor = Main::Instance().GetPhysXScene()->createActor(
		OgrePhysX::RTMeshShape(entity->getMesh()).scale(scale).group(CollisionGroups::DEFAULT));
	Main::Instance().GetOgreSceneMgr()->destroyEntity(entity);
}

void GOCStaticBody::UpdatePosition(Ogre::Vector3 position)
{
	mActor->setGlobalPosition(position);
}
void GOCStaticBody::UpdateOrientation(Ogre::Quaternion orientation)
{
	mActor->setGlobalOrientation(orientation);
}
void GOCStaticBody::UpdateScale(Ogre::Vector3 scale)
{
	if (mActor) Main::Instance().GetPhysXScene()->destroyActor(mActor);
	Create(mCollisionMeshName, scale);
	mActor->setGlobalOrientation(mOwnerGO->GetGlobalOrientation());
	mActor->setGlobalPosition(mOwnerGO->GetGlobalPosition());
}

void GOCStaticBody::SetOwner(GameObject *go)
{
	mOwnerGO = go;
	if (mActor) Main::Instance().GetPhysXScene()->destroyActor(mActor);
	Create(mCollisionMeshName, mOwnerGO->GetGlobalScale());
	mActor->getNxActor()->userData = mOwnerGO;
	mActor->setGlobalOrientation(mOwnerGO->GetGlobalOrientation());
	mActor->setGlobalPosition(mOwnerGO->GetGlobalPosition());
}

void GOCStaticBody::CreateFromDataMap(DataMap *parameters)
{
	mCollisionMeshName = parameters->GetOgreString("CollisionMeshFile");
	Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
	scale = parameters->GetOgreVec3("Scale");
	if (mOwnerGO) Create(mCollisionMeshName, scale);
}
void GOCStaticBody::GetParameters(DataMap *parameters)
{
	parameters->AddOgreString("CollisionMeshFile", mCollisionMeshName);
}
void GOCStaticBody::GetDefaultParameters(DataMap *parameters)
{
	parameters->AddOgreString("CollisionMeshFile", "");
}
void GOCStaticBody::AttachToGO(GameObject *go)
{
	go->RemoveComponent(GetFamilyID());
	go->AddComponent(this);
}

void GOCStaticBody::Save(LoadSave::SaveSystem& mgr)
{
	mgr.SaveAtom("Ogre::String", &mCollisionMeshName, "CollisionMeshFile");
}
void GOCStaticBody::Load(LoadSave::LoadSystem& mgr)
{
	mgr.LoadAtom("Ogre::String", &mCollisionMeshName);
}


//Trigger

GOCTrigger::GOCTrigger(Ogre::Vector3 boxDimensions)
{
	mShapeType = TriggerShapes::BOX;
	mBoxDimensions = boxDimensions;
	mActor = 0;
	mOwnerGO = 0;
}

GOCTrigger::GOCTrigger(float sphereRadius)
{
	mShapeType = TriggerShapes::SPHERE;
	mSphereRadius = sphereRadius;
	mActor = 0;
	mOwnerGO = 0;
}

GOCTrigger::~GOCTrigger(void)
{
	if (mActor)
	{
		Main::Instance().GetPhysXScene()->destroyActor(mActor);
	}
}

void GOCTrigger::Create(Ogre::Vector3 scale)
{
	if (mShapeType == TriggerShapes::BOX)
	{
		mActor = Main::Instance().GetPhysXScene()->createActor(
			OgrePhysX::BoxShape(mBoxDimensions * scale).setTrigger());
	}
	else
	{
		mActor = Main::Instance().GetPhysXScene()->createActor(
			OgrePhysX::SphereShape(mSphereRadius * scale.length()).setTrigger());
	}
}

void GOCTrigger::onEnter(GameObject *object)
{
	if (mOwnerGO)
	{
		Ogre::SharedPtr<ObjectMsg> msg;
		msg->mName = "TRIGGER_ENTER";
		msg->rawData = object;
		mOwnerGO->SendMessage(msg);
	}
}
void GOCTrigger::onLeave(GameObject *object)
{
	if (mOwnerGO)
	{
		Ogre::SharedPtr<ObjectMsg> msg;
		msg->mName = "TRIGGER_LEAVE";
		msg->rawData = object;
		mOwnerGO->SendMessage(msg);
	}
}

void GOCTrigger::UpdatePosition(Ogre::Vector3 position)
{
	mActor->setGlobalPosition(position);
}
void GOCTrigger::UpdateOrientation(Ogre::Quaternion orientation)
{
	mActor->setGlobalOrientation(orientation);
}
void GOCTrigger::UpdateScale(Ogre::Vector3 scale)
{
	if (mActor) Main::Instance().GetPhysXScene()->destroyActor(mActor);
	Create(scale);
	mActor->setGlobalOrientation(mOwnerGO->GetGlobalOrientation());
	mActor->setGlobalPosition(mOwnerGO->GetGlobalPosition());
}

void GOCTrigger::SetOwner(GameObject *go)
{
	mOwnerGO = go;
	if (mActor) Main::Instance().GetPhysXScene()->destroyActor(mActor);
	Create(mOwnerGO->GetGlobalScale());
	mActor->getNxActor()->userData = mOwnerGO;
	mActor->setGlobalOrientation(mOwnerGO->GetGlobalOrientation());
	mActor->setGlobalPosition(mOwnerGO->GetGlobalPosition());
}

void GOCTrigger::CreateFromDataMap(DataMap *parameters)
{
	Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
	scale = parameters->GetOgreVec3("Scale");
	mShapeType = (TriggerShapes)parameters->GetInt("ShapeType");
	mBoxDimensions = parameters->GetOgreVec3("BoxSize");
	mSphereRadius = parameters->GetFloat("Radius");
	if (mOwnerGO) Create(scale);
}
void GOCTrigger::GetParameters(DataMap *parameters)
{
	parameters->AddInt("ShapeType", mShapeType);
	parameters->AddOgreVec3("BoxSize", mBoxDimensions);
	parameters->AddFloat("Radius", mSphereRadius);
}
void GOCTrigger::GetDefaultParameters(DataMap *parameters)
{
	parameters->AddInt("ShapeType", 0);
	parameters->AddOgreVec3("BoxSize", Ogre::Vector3(1,1,1));
	parameters->AddFloat("Radius", 0);
}
void GOCTrigger::AttachToGO(GameObject *go)
{
	go->RemoveComponent(GetFamilyID());
	go->AddComponent(this);
}

void GOCTrigger::Save(LoadSave::SaveSystem& mgr)
{
	mgr.SaveAtom("int", &mShapeType, "ShapeType");
	mgr.SaveAtom("Ogre::Vector3", &mBoxDimensions, "BoxSize");
	mgr.SaveAtom("float", &mSphereRadius, "Radius");
}
void GOCTrigger::Load(LoadSave::LoadSystem& mgr)
{
	mgr.LoadAtom("int", &mShapeType);
	mgr.LoadAtom("Ogre::Vector3", &mBoxDimensions);
	mgr.LoadAtom("float", &mSphereRadius);
}

};