
#include "SGTGOCPhysics.h"
#include "SGTMain.h"
#include "SGTSceneManager.h"
#include "SGTGameObject.h"

void SGTGOPhysXRenderable::setTransform(Ogre::Vector3 position, Ogre::Quaternion rotation)
{
	if (mGO) mGO->UpdateTransform(position, rotation);
}
void SGTGOPhysXRenderable::setGO(SGTGameObject *go)
{
	mGO = go;
}


SGTGOCRigidBody::SGTGOCRigidBody(Ogre::String collision_mesh, float density, int shapetype)
{
	mCollisionMeshName = collision_mesh;
	mDensity = density;
	mShapeType = shapetype;
	mActor = 0;
	mRenderable = 0;
	mOwnerGO = 0;
}

SGTGOCRigidBody::~SGTGOCRigidBody(void)
{
	SGTMain::Instance().GetPhysXScene()->destroyActor(mActor);
	//This also destroys the renderable!
}

void SGTGOCRigidBody::Create(Ogre::String collision_mesh, float density, int shapetype, Ogre::Vector3 scale)
{
	Ogre::String internname = "RigidBody" + Ogre::StringConverter::toString(SGTSceneManager::Instance().RequestID());
	mCollisionMeshName = collision_mesh;
	mDensity = density;
	if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", mCollisionMeshName))
	{
		Ogre::LogManager::getSingleton().logMessage("Error: Resource \"" + mCollisionMeshName + "\" does not exist. Loading dummy Resource...");
		mCollisionMeshName = "DummyMesh.mesh";
	}
	Ogre::Entity *entity = SGTMain::Instance().GetOgreSceneMgr()->createEntity(internname, mCollisionMeshName);
	if (!mRenderable) mRenderable = new SGTGOPhysXRenderable(mOwnerGO);
	else mRenderable->setGO(mOwnerGO);
	if (mShapeType ==  SGTShapes::SHAPE_SPHERE)
	{
		float sphereShapeRadius = entity->getBoundingRadius();
		mActor = SGTMain::Instance().GetPhysXScene()->createRenderedActor(
			mRenderable,
			OgrePhysX::SphereShape(sphereShapeRadius * ((scale.x + scale.y + scale.z) / 3)).density(mDensity).group(SGTCollisionGroups::DEFAULT));
	}
	if (mShapeType == SGTShapes::SHAPE_CAPSULE)
	{
		Ogre::Vector3 cubeShapeSize = entity->getBoundingBox().getSize();
		cubeShapeSize = cubeShapeSize * scale;
		float capsule_radius = cubeShapeSize.x > cubeShapeSize.z ? cubeShapeSize.x : cubeShapeSize.z;
		float offset = 0.0f;
		if (cubeShapeSize.y - capsule_radius > 0.0f) offset = (cubeShapeSize.y / capsule_radius) * 0.1;
		mActor = SGTMain::Instance().GetPhysXScene()->createRenderedActor(
			mRenderable,
			OgrePhysX::CapsuleShape(capsule_radius * 0.5, cubeShapeSize.y * 0.5 + offset).density(mDensity).group(SGTCollisionGroups::DEFAULT));
	}
	else		//Default: Box
	{
		Ogre::Vector3 cubeShapeSize = entity->getBoundingBox().getSize();
		mActor = SGTMain::Instance().GetPhysXScene()->createRenderedActor(
			mRenderable,
			OgrePhysX::BoxShape(entity, scale).density(mDensity).group(SGTCollisionGroups::DEFAULT));
	}
	mActor->userData = mOwnerGO;
	SGTMain::Instance().GetOgreSceneMgr()->destroyEntity(entity);
}

void SGTGOCRigidBody::Freeze(bool freeze)
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

void SGTGOCRigidBody::UpdatePosition(Ogre::Vector3 position)
{
	if (!mOwnerGO->GetTranformingComponents()) mActor->setGlobalPosition(position);
}
void SGTGOCRigidBody::UpdateOrientation(Ogre::Quaternion orientation)
{
	if (!mOwnerGO->GetTranformingComponents()) mActor->setGlobalOrientation(orientation);
}
void SGTGOCRigidBody::UpdateScale(Ogre::Vector3 scale)
{
	if (mActor) SGTMain::Instance().GetPhysXScene()->destroyActor(mActor);
	mRenderable = 0;
	Create(mCollisionMeshName, mDensity, mShapeType, scale);
	mActor->setGlobalOrientation(mOwnerGO->GetGlobalOrientation());
	mActor->setGlobalPosition(mOwnerGO->GetGlobalPosition());
}

void SGTGOCRigidBody::SetOwner(SGTGameObject *go)
{
	mOwnerGO = go;
	Create(mCollisionMeshName, mDensity, mShapeType, mOwnerGO->GetGlobalScale());
	mActor->setGlobalOrientation(mOwnerGO->GetGlobalOrientation());
	mActor->setGlobalPosition(mOwnerGO->GetGlobalPosition());
}

void SGTGOCRigidBody::CreateFromDataMap(SGTDataMap *parameters)
{
	mCollisionMeshName = parameters->GetOgreString("CollisionMeshFile");
	Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
	scale = parameters->GetOgreVec3("Scale");
	mDensity = parameters->GetFloat("Density");
	mShapeType = parameters->GetInt("ShapeType");
	if (mOwnerGO) Create(mCollisionMeshName, mDensity, mShapeType, scale);
}
void SGTGOCRigidBody::GetParameters(SGTDataMap *parameters)
{
	parameters->AddOgreString("CollisionMeshFile", mCollisionMeshName);
	parameters->AddFloat("Density", mDensity);
	parameters->AddInt("ShapeType", mShapeType);
}
void SGTGOCRigidBody::GetDefaultParameters(SGTDataMap *parameters)
{
	parameters->AddOgreString("CollisionMeshFile", "");
	parameters->AddFloat("Density", 10.0f);
	parameters->AddInt("ShapeType", 0);
}

void SGTGOCRigidBody::AttachToGO(SGTGameObject *go)
{
	go->RemoveComponent(GetFamilyID());
	go->AddComponent(this);
}

void SGTGOCRigidBody::Save(SGTSaveSystem& mgr)
{
	mgr.SaveAtom("Ogre::String", &mCollisionMeshName, "CollisionMeshFile");
	mgr.SaveAtom("Ogre::Vector3", &mOwnerGO->GetGlobalScale(), "Scale");
	mgr.SaveAtom("float", &mDensity, "Density");
	mgr.SaveAtom("int", &mShapeType, "ShapeType");
}
void SGTGOCRigidBody::Load(SGTLoadSystem& mgr)
{
	mgr.LoadAtom("Ogre::String", &mCollisionMeshName);
	Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
	mgr.LoadAtom("Ogre::Vector3", &scale);
	mgr.LoadAtom("float", &mDensity);
	mgr.LoadAtom("int", &mShapeType);
	Create(mCollisionMeshName, mDensity, mShapeType, scale);
}


//Static Body

SGTGOCStaticBody::SGTGOCStaticBody(Ogre::String collision_mesh)
{
	mCollisionMeshName = collision_mesh;
	mActor = 0;
	mOwnerGO = 0;
}

SGTGOCStaticBody::~SGTGOCStaticBody(void)
{
	if (mActor)
	{
		SGTMain::Instance().GetPhysXScene()->destroyActor(mActor);
	}
}

void SGTGOCStaticBody::Create(Ogre::String collision_mesh, Ogre::Vector3 scale)
{
	Ogre::String internname = "StaticBody" + Ogre::StringConverter::toString(SGTSceneManager::Instance().RequestID());
	mCollisionMeshName = collision_mesh;
	if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", mCollisionMeshName))
	{
		Ogre::LogManager::getSingleton().logMessage("Error: Resource \"" + mCollisionMeshName + "\" does not exist. Loading dummy Resource...");
		mCollisionMeshName = "DummyMesh.mesh";
	}
	Ogre::Entity *entity = SGTMain::Instance().GetOgreSceneMgr()->createEntity("tempCollisionModell", mCollisionMeshName);
	mActor = SGTMain::Instance().GetPhysXScene()->createActor(
		OgrePhysX::RTMeshShape(entity->getMesh()).scale(scale));
	mActor->userData = mOwnerGO;
	SGTMain::Instance().GetOgreSceneMgr()->destroyEntity(entity);
}

void SGTGOCStaticBody::UpdatePosition(Ogre::Vector3 position)
{
	mActor->setGlobalPosition(position);
}
void SGTGOCStaticBody::UpdateOrientation(Ogre::Quaternion orientation)
{
	mActor->setGlobalOrientation(orientation);
}
void SGTGOCStaticBody::UpdateScale(Ogre::Vector3 scale)
{
	if (mActor) SGTMain::Instance().GetPhysXScene()->destroyActor(mActor);
	Create(mCollisionMeshName, scale);
	mActor->setGlobalOrientation(mOwnerGO->GetGlobalOrientation());
	mActor->setGlobalPosition(mOwnerGO->GetGlobalPosition());
}

void SGTGOCStaticBody::SetOwner(SGTGameObject *go)
{
	mOwnerGO = go;
	if (mActor) SGTMain::Instance().GetPhysXScene()->destroyActor(mActor);
	Create(mCollisionMeshName, mOwnerGO->GetGlobalScale());
	mActor->setGlobalOrientation(mOwnerGO->GetGlobalOrientation());
	mActor->setGlobalPosition(mOwnerGO->GetGlobalPosition());
}

void SGTGOCStaticBody::CreateFromDataMap(SGTDataMap *parameters)
{
	mCollisionMeshName = parameters->GetOgreString("CollisionMeshFile");
	Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
	scale = parameters->GetOgreVec3("Scale");
	if (mOwnerGO) Create(mCollisionMeshName, scale);
}
void SGTGOCStaticBody::GetParameters(SGTDataMap *parameters)
{
	parameters->AddOgreString("CollisionMeshFile", mCollisionMeshName);
}
void SGTGOCStaticBody::GetDefaultParameters(SGTDataMap *parameters)
{
	parameters->AddOgreString("CollisionMeshFile", "");
}
void SGTGOCStaticBody::AttachToGO(SGTGameObject *go)
{
	go->RemoveComponent(GetFamilyID());
	go->AddComponent(this);
}

void SGTGOCStaticBody::Save(SGTSaveSystem& mgr)
{
	mgr.SaveAtom("Ogre::String", &mCollisionMeshName, "CollisionMeshFile");
}
void SGTGOCStaticBody::Load(SGTLoadSystem& mgr)
{
	mgr.LoadAtom("Ogre::String", &mCollisionMeshName);
}


//Trigger

SGTGOCTrigger::SGTGOCTrigger(Ogre::Vector3 boxDimensions)
{
	mShapeType = TriggerShapes::BOX;
	mBoxDimensions = boxDimensions;
	mActor = 0;
	mOwnerGO = 0;
}

SGTGOCTrigger::SGTGOCTrigger(float sphereRadius)
{
	mShapeType = TriggerShapes::SPHERE;
	mSphereRadius = sphereRadius;
	mActor = 0;
	mOwnerGO = 0;
}

SGTGOCTrigger::~SGTGOCTrigger(void)
{
	if (mActor)
	{
		SGTMain::Instance().GetPhysXScene()->destroyActor(mActor);
	}
}

void SGTGOCTrigger::Create(Ogre::Vector3 scale)
{
	if (mShapeType == TriggerShapes::BOX)
	{
		mActor = SGTMain::Instance().GetPhysXScene()->createActor(
			OgrePhysX::BoxShape(mBoxDimensions * scale).setTrigger());
	}
	else
	{
		mActor = SGTMain::Instance().GetPhysXScene()->createActor(
			OgrePhysX::SphereShape(mSphereRadius * scale.length()).setTrigger());
	}
	mActor->userData = mOwnerGO;
}

void SGTGOCTrigger::onEnter(SGTGameObject *object)
{
	if (mOwnerGO)
	{
		Ogre::SharedPtr<SGTObjectMsg> msg;
		msg->mName = "TRIGGER_ENTER";
		msg->rawData = object;
		mOwnerGO->SendMessage(msg);
	}
}
void SGTGOCTrigger::onLeave(SGTGameObject *object)
{
	if (mOwnerGO)
	{
		Ogre::SharedPtr<SGTObjectMsg> msg;
		msg->mName = "TRIGGER_LEAVE";
		msg->rawData = object;
		mOwnerGO->SendMessage(msg);
	}
}

void SGTGOCTrigger::UpdatePosition(Ogre::Vector3 position)
{
	mActor->setGlobalPosition(position);
}
void SGTGOCTrigger::UpdateOrientation(Ogre::Quaternion orientation)
{
	mActor->setGlobalOrientation(orientation);
}
void SGTGOCTrigger::UpdateScale(Ogre::Vector3 scale)
{
	if (mActor) SGTMain::Instance().GetPhysXScene()->destroyActor(mActor);
	Create(scale);
	mActor->setGlobalOrientation(mOwnerGO->GetGlobalOrientation());
	mActor->setGlobalPosition(mOwnerGO->GetGlobalPosition());
}

void SGTGOCTrigger::SetOwner(SGTGameObject *go)
{
	mOwnerGO = go;
	if (mActor) SGTMain::Instance().GetPhysXScene()->destroyActor(mActor);
	Create(mOwnerGO->GetGlobalScale());
	mActor->setGlobalOrientation(mOwnerGO->GetGlobalOrientation());
	mActor->setGlobalPosition(mOwnerGO->GetGlobalPosition());
}

void SGTGOCTrigger::CreateFromDataMap(SGTDataMap *parameters)
{
	Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
	scale = parameters->GetOgreVec3("Scale");
	mShapeType = (TriggerShapes)parameters->GetInt("ShapeType");
	mBoxDimensions = parameters->GetOgreVec3("BoxSize");
	mSphereRadius = parameters->GetFloat("Radius");
	if (mOwnerGO) Create(scale);
}
void SGTGOCTrigger::GetParameters(SGTDataMap *parameters)
{
	parameters->AddInt("ShapeType", mShapeType);
	parameters->AddOgreVec3("BoxSize", mBoxDimensions);
	parameters->AddFloat("Radius", mSphereRadius);
}
void SGTGOCTrigger::GetDefaultParameters(SGTDataMap *parameters)
{
	parameters->AddInt("ShapeType", 0);
	parameters->AddOgreVec3("BoxSize", Ogre::Vector3(1,1,1));
	parameters->AddFloat("Radius", 0);
}
void SGTGOCTrigger::AttachToGO(SGTGameObject *go)
{
	go->RemoveComponent(GetFamilyID());
	go->AddComponent(this);
}

void SGTGOCTrigger::Save(SGTSaveSystem& mgr)
{
	mgr.SaveAtom("int", &mShapeType, "ShapeType");
	mgr.SaveAtom("Ogre::Vector3", &mBoxDimensions, "BoxSize");
	mgr.SaveAtom("float", &mSphereRadius, "Radius");
}
void SGTGOCTrigger::Load(SGTLoadSystem& mgr)
{
	mgr.LoadAtom("int", &mShapeType);
	mgr.LoadAtom("Ogre::Vector3", &mBoxDimensions);
	mgr.LoadAtom("float", &mSphereRadius);
}