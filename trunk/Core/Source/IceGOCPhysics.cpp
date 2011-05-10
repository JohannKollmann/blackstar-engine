
#include "IceGOCPhysics.h"
#include "IceMain.h"
#include "IceSceneManager.h"
#include "IceGameObject.h"
#include "IceObjectMessageIDs.h"

namespace Ice
{

	std::vector<ScriptParam> GOCPhysics::Body_GetSpeed(Script& caller, std::vector<ScriptParam> &vParams)
	{
		IceAssert(GetActor());
		if (GetActor()->getNxActor()->isDynamic()) SCRIPT_RETURNVALUE(GetActor()->getNxActor()->getLinearVelocity().normalize())
		else SCRIPT_RETURNERROR("Actor is static!")
	}
	std::vector<ScriptParam> GOCPhysics::Body_AddImpulse(Script& caller, std::vector<ScriptParam> &vParams)
	{
		IceAssert(GetActor());
		if (GetActor()->getNxActor()->isDynamic())
		{
			GetActor()->getNxActor()->addForce(NxVec3(vParams[0].getFloat(), vParams[1].getFloat(), vParams[2].getFloat()), NxForceMode::NX_IMPULSE);
			SCRIPT_RETURN()
		}
		else SCRIPT_RETURNERROR("Actor is static!")
	}

	void GOPhysXRenderable::setTransform(Ogre::Vector3 position, Ogre::Quaternion rotation)
	{
		if (mBody)
		{
			mBody->SetOwnerTransform(position, rotation);
		}
	}


	GOCRigidBody::GOCRigidBody(Ogre::String collision_mesh, float density, int shapetype)
	{
		mCollisionMeshName = collision_mesh;
		mDensity = density;
		mShapeType = shapetype;
		mActor = nullptr;
		mRenderable = nullptr;
		mIsKinematic = false;
	}

	GOCRigidBody::~GOCRigidBody(void)
	{
		_clear();
	}
	void GOCRigidBody::_clear()
	{
		if (mActor)
		{
			Msg msg;
			msg.typeID = GlobalMessageIDs::ACTOR_ONWAKE;
			msg.rawData = mActor->getNxActor();
			MulticastMessage(msg);
			Main::Instance().GetPhysXScene()->destroyRenderedActor((OgrePhysX::RenderedActor*)mActor);
			mActor = nullptr;
			//This also destroys the renderable!
			mRenderable = nullptr;
		}
	}

	void GOCRigidBody::Create(Ogre::String collision_mesh, float density, int shapetype, Ogre::Vector3 scale)
	{
		if (!mRenderable)
		{
			if (!mIsKinematic) mRenderable = ICE_NEW GOPhysXRenderable(this);
			else mRenderable = ICE_NEW GOPhysXRenderable(nullptr);
		}

		Ogre::String internname = "RigidBody" + Ogre::StringConverter::toString(SceneManager::Instance().RequestID());
		mCollisionMeshName = collision_mesh;
		mDensity = density;
		NxMaterialIndex nxID = SceneManager::Instance().GetSoundMaterialTable().GetMaterialID(mMaterialName);
		if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", mCollisionMeshName))
		{
			Ogre::LogManager::getSingleton().logMessage("Error: Resource \"" + mCollisionMeshName + "\" does not exist. Loading dummy Resource...");
			mCollisionMeshName = "DummyMesh.mesh";
		}
		Ogre::Entity *entity = Main::Instance().GetOgreSceneMgr()->createEntity(internname, mCollisionMeshName);
		if (mShapeType ==  Shapes::SHAPE_SPHERE)
		{
			float sphereShapeRadius = entity->getBoundingRadius();
			mActor = Main::Instance().GetPhysXScene()->createRenderedActor(
				mRenderable,
				OgrePhysX::SphereShape(sphereShapeRadius * ((scale.x + scale.y + scale.z) / 3)).density(mDensity).group(CollisionGroups::DEFAULT).material(nxID));
		}
		else if (mShapeType == Shapes::SHAPE_CAPSULE)
		{
			Ogre::Vector3 cubeShapeSize = entity->getBoundingBox().getSize();
			cubeShapeSize = cubeShapeSize * scale;
			float capsule_radius = cubeShapeSize.x > cubeShapeSize.z ? cubeShapeSize.x : cubeShapeSize.z;
			float offset = 0.0f;
			if (cubeShapeSize.y - capsule_radius > 0.0f) offset = (cubeShapeSize.y / capsule_radius) * 0.1f;
			mActor = Main::Instance().GetPhysXScene()->createRenderedActor(
				mRenderable,
				OgrePhysX::CapsuleShape(capsule_radius * 0.5f, cubeShapeSize.y * 0.5f + offset).density(mDensity).group(CollisionGroups::DEFAULT).material(nxID));
		}
		else if (mShapeType == Shapes::SHAPE_CONVEX)
		{
			mActor = Main::Instance().GetPhysXScene()->createRenderedActor(
				mRenderable,
				OgrePhysX::RTConvexMeshShape(entity->getMesh()).scale(scale).density(mDensity).group(CollisionGroups::DEFAULT).material(nxID));
		}
		else		//Default: Box
		{
			mActor = Main::Instance().GetPhysXScene()->createRenderedActor(
				mRenderable,
				OgrePhysX::BoxShape(entity, scale).density(mDensity).group(CollisionGroups::DEFAULT).material(nxID));
		}
		mActor->getNxActor()->userData = mOwnerGO.lock().get();
		mActor->getNxActor()->setGroup(CollisionGroups::DEFAULT);

		if (mIsKinematic) mActor->getNxActor()->raiseBodyFlag(NxBodyFlag::NX_BF_KINEMATIC);

		mActor->getNxActor()->setSolverIterationCount(8);

		/*mActor->getNxActor()->getShapes()[0]->setFlag(NxShapeFlag::NX_SF_DYNAMIC_DYNAMIC_CCD, true);
		OgrePhysX::World::getSingleton().getSDK()->createCCDSkeleton();*/

		Main::Instance().GetOgreSceneMgr()->destroyEntity(entity);
	}

	void GOCRigidBody::Freeze(bool freeze)
	{
		if (!mActor) return;
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
		if (mActor) mActor->setGlobalPosition(position);
	}
	void GOCRigidBody::UpdateOrientation(Ogre::Quaternion orientation)
	{
		if (mActor) mActor->setGlobalPose(mOwnerGO.lock()->GetGlobalPosition(), orientation);
	}
	void GOCRigidBody::UpdateScale(Ogre::Vector3 scale)
	{
		if (mActor) Main::Instance().GetPhysXScene()->destroyRenderedActor((OgrePhysX::RenderedActor*)mActor);
		mRenderable = nullptr;
		Create(mCollisionMeshName, mDensity, mShapeType, scale);
		mActor->setGlobalOrientation(mOwnerGO.lock()->GetGlobalOrientation());
		mActor->setGlobalPosition(mOwnerGO.lock()->GetGlobalPosition());
	}

	void GOCRigidBody::SetOwner(std::weak_ptr<GameObject> go)
	{
		mOwnerGO = go;
		GameObjectPtr owner = mOwnerGO.lock();
		if (!owner.get()) return;
		if (mActor)
		{
			UpdateScale(owner->GetGlobalScale());
			mActor->getNxActor()->userData = owner.get();
			mActor->setGlobalOrientation(owner->GetGlobalOrientation());
			mActor->setGlobalPosition(owner->GetGlobalPosition());
		}
		else if (mCollisionMeshName != "") Create(mCollisionMeshName, mDensity, mShapeType, owner->GetGlobalScale());
	}

	void GOCRigidBody::SetParameters(DataMap *parameters)
	{
		_clear();
		mCollisionMeshName = parameters->GetOgreString("CollisionMeshFile");
		mMaterialName = parameters->GetOgreString("mMaterialName");
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
		parameters->AddOgreString("mMaterialName", mMaterialName);
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
		parameters->AddFloat("Density", 10.0f);
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


	//Static Body

	GOCStaticBody::GOCStaticBody(Ogre::String collision_mesh)
	{
		mCollisionMeshName = collision_mesh;
		mActor = nullptr;
	}

	GOCStaticBody::~GOCStaticBody(void)
	{
		_clear();
	}
	void GOCStaticBody::_clear()
	{
		if (mActor)
		{
			Main::Instance().GetPhysXScene()->destroyActor(mActor);
			mActor = nullptr;
		}
	}

	void GOCStaticBody::Create(Ogre::String collision_mesh, Ogre::Vector3 scale)
	{
		_clear();
		Ogre::String internname = "StaticBody" + Ogre::StringConverter::toString(SceneManager::Instance().RequestID());
		mCollisionMeshName = collision_mesh;
		if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", mCollisionMeshName))
		{
			Ogre::LogManager::getSingleton().logMessage("Error: Resource \"" + mCollisionMeshName + "\" does not exist. Loading dummy Resource...");
			mCollisionMeshName = "DummyMesh.mesh";
		}
		Ogre::Entity *entity = Main::Instance().GetOgreSceneMgr()->createEntity("tempCollisionModell", mCollisionMeshName);
		mActor = Main::Instance().GetPhysXScene()->createActor(
			OgrePhysX::RTMeshShape(entity->getMesh()).materials(SceneManager::Instance().GetSoundMaterialTable().mOgreNxBinds).scale(scale).group(CollisionGroups::DEFAULT), mOwnerGO.lock()->GetGlobalPosition(), mOwnerGO.lock()->GetGlobalOrientation());
		mActor->getNxActor()->userData = mOwnerGO.lock().get();
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
		Create(mCollisionMeshName, scale);
		mActor->getNxActor()->userData = mOwnerGO.lock().get();
		mActor->setGlobalOrientation(mOwnerGO.lock()->GetGlobalOrientation());
		mActor->setGlobalPosition(mOwnerGO.lock()->GetGlobalPosition());
	}

	void GOCStaticBody::SetOwner(std::weak_ptr<GameObject> go)
	{
		mOwnerGO = go;
		GameObjectPtr owner = mOwnerGO.lock();
		if (!owner.get()) return;
		if (mCollisionMeshName == "") return;

		if (mActor) Main::Instance().GetPhysXScene()->destroyActor(mActor);
		Create(mCollisionMeshName, owner->GetGlobalScale());
		mActor->getNxActor()->userData = owner.get();
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
		mActor = nullptr;
		mActive = true;
	}

	GOCTrigger::GOCTrigger(float sphereRadius)
	{
		mShapeType = TriggerShapes::SPHERE;
		mSphereRadius = sphereRadius;
		mActor = nullptr;
		mActive = true;
	}

	GOCTrigger::~GOCTrigger(void)
	{
		_clear();
	}
	void GOCTrigger::_clear()
	{
		if (mActor)
		{
			Main::Instance().GetPhysXScene()->destroyActor(mActor);
			mActor = nullptr;
		}
	}

	void GOCTrigger::Create(Ogre::Vector3 scale)
	{
		if (mShapeType == TriggerShapes::BOX)
		{
			mActor = Main::Instance().GetPhysXScene()->createActor(
				OgrePhysX::BoxShape(mBoxDimensions * scale).setTrigger().group(CollisionGroups::TRIGGER));
		}
		else
		{
			mActor = Main::Instance().GetPhysXScene()->createActor(
				OgrePhysX::SphereShape(mSphereRadius * scale.length()).group(CollisionGroups::TRIGGER));
		}
		mActor->getNxActor()->userData = mOwnerGO.lock().get();
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
		mActor->setGlobalOrientation(mOwnerGO.lock()->GetGlobalOrientation());
		mActor->setGlobalPosition(mOwnerGO.lock()->GetGlobalPosition());
	}

	void GOCTrigger::SetOwner(std::weak_ptr<GameObject> go)
	{
		mOwnerGO = go;
		if (mSphereRadius == -1) return;
		if (mActor) Main::Instance().GetPhysXScene()->destroyActor(mActor);
		Create(mOwnerGO.lock()->GetGlobalScale());
		mActor->getNxActor()->userData = mOwnerGO.lock().get();
		mActor->setGlobalOrientation(mOwnerGO.lock()->GetGlobalOrientation());
		mActor->setGlobalPosition(mOwnerGO.lock()->GetGlobalPosition());
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

};