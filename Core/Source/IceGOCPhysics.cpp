
#include "IceGOCPhysics.h"
#include "IceMain.h"
#include "IceSceneManager.h"
#include "IceGameObject.h"
#include "IceObjectMessageIDs.h"
#include "IceMaterialTable.h"
#include "IceCollisionCallback.h"

namespace Ice
{
	GOCRigidBody::GOCRigidBody() : mIsKinematic(false), mRenderBinding(nullptr), mIsFreezed(false)
	{
	}
	GOCRigidBody::GOCRigidBody(Ogre::String collision_mesh, float density, int shapetype)
	{
		mCollisionMeshName = collision_mesh;
		mDensity = density;
		mShapeType = shapetype;
		mRenderBinding = nullptr;
		mIsKinematic = false;
	}

	GOCRigidBody::~GOCRigidBody(void)
	{
		_clear();
	}
	void GOCRigidBody::_clear()
	{
		if (mRenderBinding != nullptr)
		{
			Msg msg;
			msg.typeID = GlobalMessageIDs::ACTOR_ONWAKE;
			msg.rawData = mActor.getPxActor();
			Ice::MessageSystem::Instance().MulticastMessage(msg, true);

			Main::Instance().GetPhysXScene()->destroyRenderableBinding(mRenderBinding);
			mRenderBinding = nullptr;
			Main::Instance().GetPhysXScene()->removeActor(mActor);
		}
	}

	void GOCRigidBody::Create(Ogre::String collision_mesh, float density, int shapetype, Ogre::Vector3 scale)
	{
		Ogre::String internname = "RigidBody" + Ogre::StringConverter::toString(SceneManager::Instance().RequestID());
		mCollisionMeshName = collision_mesh;
		mDensity = density;
		PxMaterial *pxMat = MaterialTable::Instance().GetMaterialByName(mMaterialName);	//get material by profile name
		if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", mCollisionMeshName))
		{
			Log::Instance().LogMessage("Error: Resource \"" + mCollisionMeshName + "\" does not exist. Loading dummy Resource...");
			mCollisionMeshName = "DummyMesh.mesh";
		}
		Ogre::Entity *entity = Main::Instance().GetOgreSceneMgr()->createEntity(internname, mCollisionMeshName);

		PxTransform shapeOffset = PxTransform::createIdentity();

		if (mShapeType == Shapes::SHAPE_SPHERE)
		{
			mActor = Main::Instance().GetPhysXScene()->createRigidDynamic(OgrePhysX::Geometry::sphereGeometry(entity, shapeOffset, (scale.x+scale.y+scale.z)/3), mDensity, *pxMat);
		}
		else if (mShapeType == Shapes::SHAPE_CONVEX)
		{
			mActor = Main::Instance().GetPhysXScene()->createRigidDynamic((OgrePhysX::Geometry::convexMeshGeometry(entity->getMesh(), OgrePhysX::Cooker::Params().scale(scale))), mDensity, *pxMat);
		}
		else		//Default: Box
		{
			mActor = Main::Instance().GetPhysXScene()->createRigidDynamic(OgrePhysX::Geometry::boxGeometry(entity, shapeOffset, scale), mDensity, *pxMat);
		}
		mActor.getPxActor()->userData = mOwnerGO.lock().get();

		mActor.getPxActor()->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, mIsKinematic);

		mActor.getPxActor()->setSolverIterationCounts(8);

		mActor.getFirstShape()->setSimulationFilterData(PhysXFilterData::Instance().DynamicBody);

		mActor.getFirstShape()->setQueryFilterData(PhysXFilterData::Instance().DynamicBody);

		mRenderBinding = Main::Instance().GetPhysXScene()->createRenderedActorBinding(mActor, this);

		Main::Instance().GetOgreSceneMgr()->destroyEntity(entity);
	}

	void GOCRigidBody::Freeze(bool freeze)
	{
		mIsFreezed = freeze;
		if (!mActor.getPxActor()) return;

		mActor.setFreezed(freeze);

		if (!freeze)
			mActor.setGlobalPosition(GetOwner()->GetGlobalPosition());
	}

	void GOCRigidBody::UpdatePosition(const Ogre::Vector3 &position)
	{
		if (mActor.getPxActor()->getRigidDynamicFlags() & PxRigidDynamicFlag::eKINEMATIC)
			mActor.getPxActor()->moveKinematic(PxTransform(OgrePhysX::toPx(position), mActor.getPxActor()->getGlobalPose().q));
		else GOCPhysXActor::UpdatePosition(position);
		//mActor.getPxActor()->wakeUp();
	}

	void GOCRigidBody::UpdateScale(const Ogre::Vector3 &scale)
	{
		bool freezed = mActor.getPxActor()->getRigidDynamicFlags() & PxRigidDynamicFlag::eKINEMATIC;
		_clear();
		Create(mCollisionMeshName, mDensity, mShapeType, scale);
		mActor.setGlobalOrientation(mOwnerGO.lock()->GetGlobalOrientation());
		mActor.setGlobalPosition(mOwnerGO.lock()->GetGlobalPosition());
		mActor.getPxActor()->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, freezed);
	}

	void GOCRigidBody::SetOwner(std::weak_ptr<GameObject> go)
	{
		mOwnerGO = go;
		GameObjectPtr owner = mOwnerGO.lock();
		if (!owner.get()) return;
		if (mActor.getPxActor())
		{
			UpdateScale(owner->GetGlobalScale());
			mActor.getPxActor()->userData = owner.get();
			mActor.setGlobalOrientation(owner->GetGlobalOrientation());
			mActor.setGlobalPosition(owner->GetGlobalPosition());
		}
		else if (mCollisionMeshName != "") Create(mCollisionMeshName, mDensity, mShapeType, owner->GetGlobalScale());
	}

	Ogre::String GOCRigidBody::GetVisualObjectDescription()
	{
		return GetOwner()->GetName();
	}
	void GOCRigidBody::GetTrackPoints(std::vector<Ogre::Vector3> &outPoints)
	{
		outPoints.push_back(GetOwner()->GetGlobalPosition());
	}

	std::vector<ScriptParam> GOCRigidBody::Body_GetSpeed(Script& caller, std::vector<ScriptParam> &vParams)
	{
		SCRIPT_RETURNVALUE(mActor.getPxActor()->getLinearVelocity().normalize())
	}
	std::vector<ScriptParam> GOCRigidBody::Body_AddImpulse(Script& caller, std::vector<ScriptParam> &vParams)
	{
		mActor.getPxActor()->addForce(PxVec3(vParams[0].getFloat(), vParams[1].getFloat(), vParams[2].getFloat()), PxForceMode::eIMPULSE);
		SCRIPT_RETURN()
	}

	void GOCRigidBody::setTransform(const Ogre::Vector3 &position, const Ogre::Quaternion &rotation)
	{
		if (!mIsFreezed)
			SetOwnerTransform(position, rotation);
	}

	void GOCRigidBody::SetParameters(DataMap *parameters)
	{
		_clear();
		mCollisionMeshName = parameters->GetOgreString("CollisionMeshFile");
		mMaterialName = parameters->GetOgreString("Material");
		Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
		scale = parameters->GetOgreVec3("Scale");
		mDensity = parameters->GetFloat("Density");
		mShapeType = parameters->GetEnum("ShapeType").selection;
		mIsKinematic = parameters->GetValue<bool>("Kinematic", false);
		if (!mOwnerGO.expired()) Create(mCollisionMeshName, mDensity, mShapeType, scale);
	}
	void GOCRigidBody::GetParameters(DataMap *parameters)
	{
		parameters->AddOgreString("CollisionMeshFile", mCollisionMeshName);
		parameters->AddOgreString("Material", mMaterialName);
		parameters->AddFloat("Density", mDensity);
		std::vector<Ogre::String> shape_types;
		shape_types.push_back("Box"); shape_types.push_back("Sphere"); shape_types.push_back("Convex"); shape_types.push_back("NXS"); shape_types.push_back("Capsule");
		parameters->AddEnum("ShapeType", shape_types, mShapeType);
		parameters->AddBool("Kinematic", mIsKinematic);
	}
	void GOCRigidBody::GetDefaultParameters(DataMap *parameters)
	{
		parameters->AddOgreString("CollisionMeshFile", "");
		parameters->AddOgreString("mMaterialName", "Wood");
		parameters->AddFloat("Density", 100.0f);
		std::vector<Ogre::String> shape_types;
		shape_types.push_back("Box"); shape_types.push_back("Sphere"); shape_types.push_back("Convex"); shape_types.push_back("NXS"); shape_types.push_back("Capsule");
		parameters->AddEnum("ShapeType", shape_types, 0);
		parameters->AddBool("Kinematic", false);
	}

	void GOCRigidBody::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("Ogre::String", &mCollisionMeshName, "CollisionMeshFile");
		mgr.SaveAtom("Ogre::Vector3", &mOwnerGO.lock()->GetGlobalScale(), "Scale");
		mgr.SaveAtom("float", &mDensity, "Density");
		mgr.SaveAtom("int", &mShapeType, "ShapeType");
		mgr.SaveAtom("Ogre::String", &mMaterialName, "mMaterialName");
		mgr.SaveAtom("bool", &mIsKinematic, "IsKinematic");
	}
	void GOCRigidBody::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("Ogre::String", &mCollisionMeshName);
		Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
		mgr.LoadAtom("Ogre::Vector3", &scale);
		mgr.LoadAtom("float", &mDensity);
		mgr.LoadAtom("int", &mShapeType);
		mgr.LoadAtom("Ogre::String", &mMaterialName);
		mgr.LoadAtom("bool", &mIsKinematic);
		Create(mCollisionMeshName, mDensity, mShapeType, scale);
	}

	//========================================================================================
	//==================================Destructible==========================================
	//========================================================================================

	GOCDestructible::~GOCDestructible()
	{
		_clear();
	}

	void GOCDestructible::_create()
	{
		_clear();

		GameObjectPtr owner = mOwnerGO.lock();

		IceAssert(owner.get())

		PxMaterial *pxMat = MaterialTable::Instance().GetMaterialByName(mMaterialName);

		mDestructible = Main::Instance().GetPhysXScene()->createDestructible(mConfigFile, *pxMat, mMaxForce, mMaxTorque, mDensity, mOwnerGO.lock()->GetGlobalScale());

		mDestructible->setSimulationFilterData(PhysXFilterData::Instance().DynamicBody);
		mDestructible->setQueryFilterData(PhysXFilterData::Instance().DynamicBody);

		mDestructible->setUserData(mOwnerGO.lock().get());

		mDestructible->setPointRenderable(this);

		UpdateOrientation(mOwnerGO.lock()->GetGlobalOrientation());
		UpdatePosition(mOwnerGO.lock()->GetGlobalPosition());
	}

	void GOCDestructible::_clear()
	{
		if (mDestructible)
		{
			Main::Instance().GetPhysXScene()->destroyRenderableBinding(mDestructible);
			mDestructible = nullptr;
		}
	}

	void GOCDestructible::Freeze(bool freeze)
	{
		mIsFreezed = freeze;
		mDestructible->setFreezed(freeze);
	}

	void GOCDestructible::UpdatePosition(const Ogre::Vector3 &position)
	{
		mDestructible->setGlobalPosition(position);
	}

	void GOCDestructible::UpdateScale(const Ogre::Vector3 &scale)
	{
		_clear();
		_create();
	}

	void GOCDestructible::setTransform(const Ogre::Vector3 &position, const Ogre::Quaternion &rotation)
	{
		if (!mIsFreezed)
			SetOwnerTransform(position, rotation);
	}

	void GOCDestructible::NotifyOwnerGO()
	{
		GameObjectPtr owner = mOwnerGO.lock();
		if (!owner.get()) return;
		if (mDestructible)
		{
			mDestructible->setUserData(owner.get());
			UpdateOrientation(mOwnerGO.lock()->GetGlobalOrientation());
			UpdatePosition(mOwnerGO.lock()->GetGlobalPosition());
		}
		else if (mConfigFile != "") _create();
	}

	Ogre::String GOCDestructible::GetVisualObjectDescription()
	{
		return GetOwner()->GetName();
	}
	void GOCDestructible::GetTrackPoints(std::vector<Ogre::Vector3> &outPoints)
	{
		outPoints.push_back(GetOwner()->GetGlobalPosition());
	}

	void GOCDestructible::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("string", &mConfigFile, "Config File");
		mgr.SaveAtom("string", &mMaterialName, "MaterialName");
		mgr.SaveAtom("float", &mDensity, "Density");
		mgr.SaveAtom("float", &mMaxForce, "Max Force");
		mgr.SaveAtom("float", &mMaxTorque, "Max Torque");
	}
	void GOCDestructible::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("string", &mConfigFile);
		mgr.LoadAtom("string", &mMaterialName);
		mgr.LoadAtom("float", &mDensity);
		mgr.LoadAtom("float", &mMaxForce);
		mgr.LoadAtom("float", &mMaxTorque);
	}

	//========================================================================================
	//==================================Static body===========================================
	//========================================================================================

	GOCStaticBody::GOCStaticBody(Ogre::String collision_mesh)
	{
		mCollisionMeshName = collision_mesh;
	}

	GOCStaticBody::~GOCStaticBody(void)
	{
		_clear();
	}
	void GOCStaticBody::_clear()
	{
		if (mActor.getPxActor())
		{
			Main::Instance().GetPhysXScene()->removeActor(mActor);
		}
	}

	void GOCStaticBody::Create(Ogre::String collision_mesh, Ogre::Vector3 scale)
	{
		_clear();
		Ogre::String internname = "StaticBody" + Ogre::StringConverter::toString(SceneManager::Instance().RequestID());
		mCollisionMeshName = collision_mesh;
		if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", mCollisionMeshName))
		{
			Log::Instance().LogMessage("Error: Resource \"" + mCollisionMeshName + "\" does not exist. Loading dummy Resource...");
			mCollisionMeshName = "DummyMesh.mesh";
		}
		Ogre::Entity *entity = Main::Instance().GetOgreSceneMgr()->createEntity("tempCollisionModell", mCollisionMeshName);

		GameObjectPtr owner = mOwnerGO.lock();
		mActor = Main::Instance().GetPhysXScene()->createRigidStatic(entity, OgrePhysX::Cooker::Params().scale(scale));

		mActor.getPxActor()->userData = mOwnerGO.lock().get();

		mActor.getFirstShape()->setSimulationFilterData(PhysXFilterData::Instance().StaticBody);

		mActor.getFirstShape()->setQueryFilterData(PhysXFilterData::Instance().StaticBody);

		Main::Instance().GetOgreSceneMgr()->destroyEntity(entity);
	}

	void GOCStaticBody::UpdateScale(const Ogre::Vector3 &scale)
	{
		Create(mCollisionMeshName, scale);
		mActor.setGlobalOrientation(mOwnerGO.lock()->GetGlobalOrientation());
		mActor.setGlobalPosition(mOwnerGO.lock()->GetGlobalPosition());
	}

	void GOCStaticBody::SetOwner(std::weak_ptr<GameObject> go)
	{
		mOwnerGO = go;
		GameObjectPtr owner = mOwnerGO.lock();
		if (!owner.get()) return;
		if (mCollisionMeshName == "") return;

		Create(mCollisionMeshName, owner->GetGlobalScale());
	}

	Ogre::String GOCStaticBody::GetVisualObjectDescription()
	{
		return GetOwner()->GetName();
	}
	void GOCStaticBody::GetTrackPoints(std::vector<Ogre::Vector3> &outPoints)
	{
		outPoints.push_back(GetOwner()->GetGlobalPosition());
	}

	void GOCStaticBody::SetParameters(DataMap *parameters)
	{
		_clear();
		mCollisionMeshName = parameters->GetOgreString("CollisionMeshFile");
		Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
		scale = parameters->GetOgreVec3("Scale");
		if (!mOwnerGO.expired()) Create(mCollisionMeshName, scale);
	}
	void GOCStaticBody::GetParameters(DataMap *parameters)
	{
		parameters->AddOgreString("CollisionMeshFile", mCollisionMeshName);
	}
	void GOCStaticBody::GetDefaultParameters(DataMap *parameters)
	{
		parameters->AddOgreString("CollisionMeshFile", "");
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
		mActive = true;
	}

	GOCTrigger::GOCTrigger(float sphereRadius)
	{
		mShapeType = TriggerShapes::SPHERE;
		mSphereRadius = sphereRadius;
		mActive = true;
	}

	GOCTrigger::~GOCTrigger(void)
	{
		_clear();
	}
	void GOCTrigger::_clear()
	{
		if (mActor.getPxActor())
		{
			Main::Instance().GetPhysXScene()->removeActor(mActor);
		}
	}

	void GOCTrigger::Create(Ogre::Vector3 scale)
	{
		GameObjectPtr owner = mOwnerGO.lock();
		mActor.setPxActor(OgrePhysX::getPxPhysics()->createRigidStatic(PxTransform(OgrePhysX::Convert::toPx(owner->GetGlobalPosition()), OgrePhysX::Convert::toPx(owner->GetGlobalOrientation()))));

		mActor.getPxActor()->userData = owner.get();

		PxShape *shape;

		if (mShapeType == TriggerShapes::BOX)
		{
			shape = mActor.getPxActor()->createShape(OgrePhysX::Geometry::boxGeometry(mBoxDimensions * scale), OgrePhysX::World::getSingleton().getDefaultMaterial());		
		}
		else
		{
			shape = mActor.getPxActor()->createShape(PxSphereGeometry(mSphereRadius), OgrePhysX::World::getSingleton().getDefaultMaterial());
		}

		shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);

		Main::Instance().GetPhysXScene()->getPxScene()->addActor(*mActor.getPxActor());
	}

	void GOCTrigger::onEnter(GameObject *object)
	{
		GameObjectPtr owner = mOwnerGO.lock();
		if (owner.get() && mActive)
		{
			Msg msg;
			msg.typeID = ObjectMessageIDs::TRIGGER_ENTER;
			msg.params.AddInt("OBJ_ID", object->GetID());
			msg.rawData = object;
			BroadcastObjectMessage(msg);
		}
	}
	void GOCTrigger::onLeave(GameObject *object)
	{
		GameObjectPtr owner = mOwnerGO.lock();
		if (owner.get() && mActive)
		{
			Msg msg;
			msg.typeID = ObjectMessageIDs::TRIGGER_LEAVE;
			msg.params.AddInt("OBJ_ID", object->GetID());
			msg.rawData = object;
			BroadcastObjectMessage(msg);
		}
	}

	void GOCTrigger::UpdateScale(const Ogre::Vector3 &scale)
	{
		Create(scale);
		mActor.setGlobalOrientation(mOwnerGO.lock()->GetGlobalOrientation());
		mActor.setGlobalPosition(mOwnerGO.lock()->GetGlobalPosition());
	}

	void GOCTrigger::SetOwner(std::weak_ptr<GameObject> go)
	{
		mOwnerGO = go;
		GameObjectPtr owner = mOwnerGO.lock();
		if (!owner.get()) return;

		Create(owner->GetGlobalScale());
	}

	void GOCTrigger::SetParameters(DataMap *parameters)
	{
		_clear();
		Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
		scale = parameters->GetOgreVec3("Scale");
		mShapeType = (TriggerShapes)parameters->GetEnum("ShapeType").selection;
		mBoxDimensions = parameters->GetOgreVec3("BoxSize");
		mSphereRadius = parameters->GetFloat("Radius");
		mActive = parameters->GetValue<bool>("Active", true);
		if (!mOwnerGO.expired()) Create(scale);
	}
	void GOCTrigger::GetParameters(DataMap *parameters)
	{
		std::vector<Ogre::String> shape_types;
		shape_types.push_back("Box"); shape_types.push_back("Sphere");
		parameters->AddEnum("ShapeType", shape_types, 0);
		parameters->AddOgreVec3("BoxSize", mBoxDimensions);
		parameters->AddFloat("Radius", mSphereRadius);
		parameters->AddBool("Active", mActive);
	}
	void GOCTrigger::GetDefaultParameters(DataMap *parameters)
	{
		std::vector<Ogre::String> shape_types;
		shape_types.push_back("Box"); shape_types.push_back("Sphere");
		parameters->AddEnum("ShapeType", shape_types, 0);
		parameters->AddOgreVec3("BoxSize", Ogre::Vector3(1,1,1));
		parameters->AddFloat("Radius", 0);
		parameters->AddBool("Active", true);
	}

	void GOCTrigger::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("int", &mShapeType, "ShapeType");
		mgr.SaveAtom("Ogre::Vector3", &mBoxDimensions, "BoxSize");
		mgr.SaveAtom("float", &mSphereRadius, "Radius");
		mgr.SaveAtom("bool", &mActive, "Active");
	}
	void GOCTrigger::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("int", &mShapeType);
		mgr.LoadAtom("Ogre::Vector3", &mBoxDimensions);
		mgr.LoadAtom("float", &mSphereRadius);
		mgr.LoadAtom("bool", &mActive);
	}

	std::vector<ScriptParam> GOCTrigger::Trigger_SetActive(Script& caller, std::vector<ScriptParam> &vParams)
	{
		mActive = vParams[0].getBool();
		SCRIPT_RETURN()
	}

	void GOCParticleChain::UpdatePosition(const Ogre::Vector3 &position)
	{
		if (mParticleChain)
			mParticleChain->setPosition(position);
	}

	void GOCParticleChain::_create()
	{
		_clear();
		if (mSimulationMethod.selection == 1)
		{
			mParticleChain = Main::Instance().GetPhysXScene()->createSpringParticleChain(mOwnerGO.lock()->GetGlobalPosition(), mOwnerGO.lock()->GetGlobalPosition() + Ogre::Vector3(0, -1, 0) * mChainLength, mNumParticles);
			((OgrePhysX::SpringParticleChain*)mParticleChain)->setSpringStiffness(mSpringStiffness);
			((OgrePhysX::SpringParticleChain*)mParticleChain)->setSpringDamping(mSpringDamping);
		}
		else if (mSimulationMethod.selection == 2)
		{
			mParticleChain = Main::Instance().GetPhysXScene()->createPBDParticleChain(mOwnerGO.lock()->GetGlobalPosition(), mOwnerGO.lock()->GetGlobalPosition() + Ogre::Vector3(0, -1, 0) * mChainLength, mNumParticles);
			((OgrePhysX::PBDParticleChain*)mParticleChain)->setIterationCount(mIterationCount);
			((OgrePhysX::PBDParticleChain*)mParticleChain)->setOverRelaxation(mOverRelaxation);
		}
		else
		{
			mParticleChain = Main::Instance().GetPhysXScene()->createFTLParticleChain(mOwnerGO.lock()->GetGlobalPosition(), mOwnerGO.lock()->GetGlobalPosition() + Ogre::Vector3(0, -1, 0) * mChainLength, mNumParticles);
			((OgrePhysX::FTLParticleChain*)mParticleChain)->setFTLDamping(mFTLDamping);
		}
		mParticleChain->setParticleMass(mParticleMass);
		mParticleChain->setChainStiffness(mChainStiffness);
		mParticleChain->setPointDamping(mPointDamping);
		mParticleChain->setPBDPointDamping(mPBDPointDamping);
		mParticleChain->setFriction(mFriction);

		mDebugVisual = Main::Instance().GetPhysXScene()->createParticleChainDebugVisual(mParticleChain);
		auto ents = mDebugVisual->getNodesAndEntities();
		for (auto i = ents.begin(); i != ents.end(); ++i)
			i->second->setVisibilityFlags(Ice::VisibilityFlags::V_DEFAULT);
	}

	void GOCParticleChain::_clear()
	{
		if (!mParticleChain) return;
		Main::Instance().GetPhysXScene()->destroyRenderableBinding(mDebugVisual);
		Main::Instance().GetPhysXScene()->destroyParticleChain(mParticleChain);
		mParticleChain = nullptr;
	}

	void GOCParticleChain::NotifyPostInit()
	{
		if (mOwnerGO.lock())
			_create();
	}
};