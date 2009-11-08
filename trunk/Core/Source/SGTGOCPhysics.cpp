
#include "SGTGOCPhysics.h"
#include "SGTMain.h"
#include "SGTSceneManager.h"
#include "SGTGameObject.h"


void SGTGOCRenderable::setPose(const NxOgre::Pose& p)
{
	mOwner->UpdateTransform(p, p);
}
NxOgre::Pose SGTGOCRenderable::getPose() const
{
	return NxOgre::Pose(mOwner->GetGlobalPosition(), mOwner->GetGlobalOrientation());
}


SGTGOCRigidBody::SGTGOCRigidBody(Ogre::String collision_mesh, float density, int shapetype)
{
	mCollisionMeshName = collision_mesh;
	mDensity = density;
	mShapeType = shapetype;
	mActor = 0;
	mRenderable = 0;
}

SGTGOCRigidBody::~SGTGOCRigidBody(void)
{
	if (mRenderable)
	{
		SGTMain::Instance().GetNxScene()->getSceneRenderer()->unregisterSource(this);
		delete mRenderable;
		mRenderable = 0;
		SGTMain::Instance().GetNxScene()->destroyActor(mActor->getName());
	}
}

void SGTGOCRigidBody::Create(Ogre::String collision_mesh, float density, int shapetype, Ogre::Vector3 scale)
{
	Ogre::String internname = "RigidBody" + Ogre::StringConverter::toString(SGTSceneManager::Instance().RequestID());
	mCollisionMeshName = collision_mesh;
	mDensity = density;
	mShapeType = shapetype;
	NxOgre::ActorParams ap;
	ap.setToDefault();
	ap.mDensity = mDensity;
	//ap.mMass = 0;
	NxOgre::ShapeParams sp;
	sp.setToDefault();
	sp.mDensity = mDensity;
	sp.mGroup = "Collidable";
	//sp.mMass = 0;
	if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", mCollisionMeshName))
	{
		Ogre::LogManager::getSingleton().logMessage("Error: Resource \"" + mCollisionMeshName + "\" does not exist. Loading dummy Resource...");
		mCollisionMeshName = "DummyMesh.mesh";
	}
	Ogre::Entity *entity = SGTMain::Instance().GetOgreSceneMgr()->createEntity(internname, mCollisionMeshName);
	//if (mShapeType == SGTShapes::SHAPE_CONVEX) mActor = SGTMain::Instance().GetNxScene()->createActor(internanem, shape, NxOgre::Pose(), ap);
	if (mShapeType ==  SGTShapes::SHAPE_BOX)
	{
		Ogre::Vector3 cubeShapeSize = entity->getBoundingBox().getSize();
		mActor = SGTMain::Instance().GetNxScene()->createActor(internname, new NxOgre::Cube(cubeShapeSize.x * scale.x, cubeShapeSize.y  * scale.y, cubeShapeSize.z * scale.z, sp), NxOgre::Pose(), ap);
	}
	if (mShapeType ==  SGTShapes::SHAPE_SPHERE)
	{
		float sphereShapeRadius = entity->getBoundingRadius();
		mActor = SGTMain::Instance().GetNxScene()->createActor(internname, new NxOgre::Sphere(sphereShapeRadius * ((scale.x + scale.y + scale.z) / 3), sp), NxOgre::Pose(), ap);
	}
	if (mShapeType == SGTShapes::SHAPE_CAPSULE)
	{
		Ogre::Vector3 cubeShapeSize = entity->getBoundingBox().getSize();
		cubeShapeSize = cubeShapeSize * scale;
		float capsule_radius = cubeShapeSize.x > cubeShapeSize.z ? cubeShapeSize.x : cubeShapeSize.z;
		float offset = 0.0f;
		if (cubeShapeSize.y - capsule_radius > 0.0f) offset = (cubeShapeSize.y / capsule_radius) * 0.1;
		mActor = SGTMain::Instance().GetNxScene()->createActor(internname, new NxOgre::Capsule(capsule_radius * 0.5, cubeShapeSize.y * 0.5 + offset, sp), NxOgre::Pose(), ap);
	}
	SGTMain::Instance().GetOgreSceneMgr()->destroyEntity(entity);
	mActor->getVoidPointer()->RenderPtr = this;
	setRenderMode(RM_Interpolate);
}

void SGTGOCRigidBody::Freeze(bool freeze)
{
	if (freeze)
	{
		mActor->raiseBodyFlag(NX_BF_DISABLE_GRAVITY);	
		mActor->raiseBodyFlag(NX_BF_FROZEN);	
	}
	else
	{
		mActor->clearBodyFlag(NX_BF_DISABLE_GRAVITY);	
		mActor->clearBodyFlag(NX_BF_FROZEN);	
		mActor->wakeUp();
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
	if (mActor) SGTMain::Instance().GetNxScene()->destroyActor(mActor->getName());
	Create(mCollisionMeshName, mDensity, mShapeType, scale);
	mActor->setGlobalPose(NxOgre::Pose(mOwnerGO->GetGlobalPosition(), mOwnerGO->GetGlobalOrientation()));
}

void SGTGOCRigidBody::SetOwner(SGTGameObject *go)
{
	mOwnerGO = go;
	if (mRenderable) delete mRenderable;
	else SGTMain::Instance().GetNxScene()->getSceneRenderer()->registerSource(this);
	mRenderable = new SGTGOCRenderable(mOwnerGO);
	if (mActor) SGTMain::Instance().GetNxScene()->destroyActor(mActor->getName());
	Create(mCollisionMeshName, mDensity, mShapeType, mOwnerGO->GetGlobalScale());
	mActor->setGlobalPose(NxOgre::Pose(mOwnerGO->GetGlobalPosition(), mOwnerGO->GetGlobalOrientation()));
}

void SGTGOCRigidBody::CreateFromDataMap(SGTDataMap *parameters)
{
	mCollisionMeshName = parameters->GetOgreString("CollisionMeshFile");
	Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
	scale = parameters->GetOgreVec3("Scale");
	mDensity = parameters->GetFloat("Density");
	mShapeType = parameters->GetInt("ShapeType");
	Create(mCollisionMeshName, mDensity, mShapeType, scale);
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
}

SGTGOCStaticBody::~SGTGOCStaticBody(void)
{
	if (mActor)
	{
		SGTMain::Instance().GetNxScene()->destroyActor(mActor->getName());
	}
}

void SGTGOCStaticBody::Create(Ogre::String collision_mesh)
{
	Ogre::String internname = "StaticBody" + Ogre::StringConverter::toString(SGTSceneManager::Instance().RequestID());
	mCollisionMeshName = collision_mesh;
	NxOgre::ActorParams ap;
	ap.setToDefault();
	ap.mDensity = 0.0f;
	ap.mMass = 0.0f;
	NxOgre::ShapeParams sp;
	sp.setToDefault();
	sp.mDensity = 0.0f;
	sp.mMass = 0.0f;
	sp.mGroup = "Collidable";
	if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", mCollisionMeshName))
	{
		Ogre::LogManager::getSingleton().logMessage("Error: Resource \"" + mCollisionMeshName + "\" does not exist. Loading dummy Resource...");
		mCollisionMeshName = "DummyMesh.mesh";
	}
	SGTSceneManager::Instance().BakeStaticMeshShape(mCollisionMeshName);
	mActor = SGTMain::Instance().GetNxScene()->createActor(internname, new NxOgre::TriangleMesh(NxOgre::Resources::ResourceSystem::getSingleton()->getMesh("Data/Media/Meshes/NXS/" + mCollisionMeshName + ".nxs"), sp), NxOgre::Pose(), ap);
}

void SGTGOCStaticBody::UpdatePosition(Ogre::Vector3 position)
{
	if (!mOwnerGO->GetTranformingComponents()) mActor->setGlobalPosition(position);
}
void SGTGOCStaticBody::UpdateOrientation(Ogre::Quaternion orientation)
{
	if (!mOwnerGO->GetTranformingComponents()) mActor->setGlobalOrientation(orientation);
}
void SGTGOCStaticBody::UpdateScale(Ogre::Vector3 scale)
{
	/*if (mActor) SGTMain::Instance().GetNxScene()->destroyActor(mActor->getName());
	Create(mCollisionMeshName, scale);
	mActor->setGlobalPose(NxOgre::Pose(mOwnerGO->GetGlobalPosition(), mOwnerGO->GetGlobalOrientation()));*/
}

void SGTGOCStaticBody::SetOwner(SGTGameObject *go)
{
	mOwnerGO = go;
	if (mActor) SGTMain::Instance().GetNxScene()->destroyActor(mActor->getName());
	Create(mCollisionMeshName);
	mActor->setGlobalPose(NxOgre::Pose(mOwnerGO->GetGlobalPosition(), mOwnerGO->GetGlobalOrientation()));
}

void SGTGOCStaticBody::CreateFromDataMap(SGTDataMap *parameters)
{
	mCollisionMeshName = parameters->GetOgreString("CollisionMeshFile");
}
void SGTGOCStaticBody::GetParameters(SGTDataMap *parameters)
{
	parameters->AddOgreString("CollisionMeshFile", mCollisionMeshName);
}
void SGTGOCStaticBody::GetDefaultParameters(SGTDataMap *parameters)
{
	parameters->AddOgreString("CollisionMeshFile", "");
}

void SGTGOCStaticBody::Save(SGTSaveSystem& mgr)
{
	mgr.SaveAtom("Ogre::String", &mCollisionMeshName, "CollisionMeshFile");
}
void SGTGOCStaticBody::Load(SGTLoadSystem& mgr)
{
	mgr.LoadAtom("Ogre::String", &mCollisionMeshName);
}