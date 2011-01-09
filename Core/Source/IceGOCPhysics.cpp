
#include "IceGOCPhysics.h"
#include "IceMain.h"
#include "IceSceneManager.h"
#include "IceGameObject.h"

namespace Ice
{

	std::vector<ScriptParam> GOCPhysics::Body_GetSpeed(Script& caller, std::vector<ScriptParam> &vParams)
	{
		IceAssert(mActor);
		if (mActor->getNxActor()->isDynamic()) SCRIPT_RETURNVALUE(mActor->getNxActor()->computeKineticEnergy())
		else SCRIPT_RETURNERROR("Actor is static!")
	}
	std::vector<ScriptParam> GOCPhysics::Body_AddImpulse(Script& caller, std::vector<ScriptParam> &vParams)
	{
		IceAssert(mActor);
		if (mActor->getNxActor()->isDynamic())
		{
			mActor->getNxActor()->addForce(NxVec3(vParams[0].getFloat(), vParams[1].getFloat(), vParams[2].getFloat()), NxForceMode::NX_IMPULSE);
			SCRIPT_RETURN()
		}
		else SCRIPT_RETURNERROR("Actor is static!")
	}

	void GOPhysXRenderable::setTransform(Ogre::Vector3 position, Ogre::Quaternion rotation)
	{
		if (mBody)
		{
			mBody->SetOwnerPosition(position);
			mBody->SetOwnerOrientation(rotation);
		}
	}


	GOCRigidBody::GOCRigidBody(Ogre::String collision_mesh, float density, int shapetype)
	{
		mCollisionMeshName = collision_mesh;
		mDensity = density;
		mShapeType = shapetype;
		mActor = nullptr;
		mRenderable = nullptr;
		mOwnerGO = nullptr;
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
			msg.type = "ACTOR_ONWAKE";
			msg.rawData = mActor->getNxActor();
			MessageSystem::Instance().SendInstantMessage(msg);
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
		mActor->getNxActor()->userData = mOwnerGO;
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
		if (mActor) mActor->setGlobalPose(mOwnerGO->GetGlobalPosition(), orientation);
	}
	void GOCRigidBody::UpdateScale(Ogre::Vector3 scale)
	{
		if (mActor) Main::Instance().GetPhysXScene()->destroyRenderedActor((OgrePhysX::RenderedActor*)mActor);
		mRenderable = nullptr;
		Create(mCollisionMeshName, mDensity, mShapeType, scale);
		mActor->setGlobalOrientation(mOwnerGO->GetGlobalOrientation());
		mActor->setGlobalPosition(mOwnerGO->GetGlobalPosition());
	}

	void GOCRigidBody::SetOwner(GameObject *go)
	{
		mOwnerGO = go;
		if (!mOwnerGO) return;
		if (mActor)
		{
			UpdateScale(mOwnerGO->GetGlobalScale());
			mActor->getNxActor()->userData = mOwnerGO;
			mActor->setGlobalOrientation(mOwnerGO->GetGlobalOrientation());
			mActor->setGlobalPosition(mOwnerGO->GetGlobalPosition());
		}
		else if (mCollisionMeshName != "") Create(mCollisionMeshName, mDensity, mShapeType, mOwnerGO->GetGlobalScale());
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
		if (mOwnerGO) Create(mCollisionMeshName, mDensity, mShapeType, scale);
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
		mgr.SaveAtom("Ogre::Vector3", &mOwnerGO->GetGlobalScale(), "Scale");
		mgr.SaveAtom("float", &mDensity, "Density");
		mgr.SaveAtom("int", &mShapeType, "ShapeType");
		mgr.SaveAtom("Ogre::String", &mMaterialName, "mMaterialName");
	}
	void GOCRigidBody::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("Ogre::String", &mCollisionMeshName);
		Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
		mgr.LoadAtom("Ogre::Vector3", &scale);
		mgr.LoadAtom("float", &mDensity);
		mgr.LoadAtom("int", &mShapeType);
		mgr.LoadAtom("Ogre::String", &mMaterialName);
		Create(mCollisionMeshName, mDensity, mShapeType, scale);
	}


	//Static Body

	GOCStaticBody::GOCStaticBody(Ogre::String collision_mesh)
	{
		mCollisionMeshName = collision_mesh;
		mActor = nullptr;
		mOwnerGO = nullptr;
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
			OgrePhysX::RTMeshShape(entity->getMesh()).materials(SceneManager::Instance().GetSoundMaterialTable().mOgreNxBinds).scale(scale).group(CollisionGroups::DEFAULT), mOwnerGO->GetGlobalPosition(), mOwnerGO->GetGlobalOrientation());
		mActor->getNxActor()->userData = mOwnerGO;
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
		mActor->getNxActor()->userData = mOwnerGO;
		mActor->setGlobalOrientation(mOwnerGO->GetGlobalOrientation());
		mActor->setGlobalPosition(mOwnerGO->GetGlobalPosition());
	}

	void GOCStaticBody::SetOwner(GameObject *go)
	{
		mOwnerGO = go;
		if (!mOwnerGO) return;
		if (mCollisionMeshName == "") return;

		if (mActor) Main::Instance().GetPhysXScene()->destroyActor(mActor);
		Create(mCollisionMeshName, mOwnerGO->GetGlobalScale());
		mActor->getNxActor()->userData = mOwnerGO;
	}

	void GOCStaticBody::SetParameters(DataMap *parameters)
	{
		_clear();
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
		mOwnerGO = nullptr;
	}

	GOCTrigger::GOCTrigger(float sphereRadius)
	{
		mShapeType = TriggerShapes::SPHERE;
		mSphereRadius = sphereRadius;
		mActor = nullptr;
		mOwnerGO = nullptr;
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
				OgrePhysX::BoxShape(mBoxDimensions * scale).setTrigger());
		}
		else
		{
			mActor = Main::Instance().GetPhysXScene()->createActor(
				OgrePhysX::SphereShape(mSphereRadius * scale.length()).setTrigger());
		}
		mActor->getNxActor()->userData = mOwnerGO;
	}

	void GOCTrigger::onEnter(GameObject *object)
	{
		if (mOwnerGO)
		{
			Msg msg;
			msg.type = "TRIGGER_ENTER";
			msg.rawData = object;
			mOwnerGO->SendMessage(msg);
		}
	}
	void GOCTrigger::onLeave(GameObject *object)
	{
		if (mOwnerGO)
		{
			Msg msg;
			msg.type = "TRIGGER_LEAVE";
			msg.rawData = object;
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
		if (mSphereRadius == -1) return;
		if (mActor) Main::Instance().GetPhysXScene()->destroyActor(mActor);
		Create(mOwnerGO->GetGlobalScale());
		mActor->getNxActor()->userData = mOwnerGO;
		mActor->setGlobalOrientation(mOwnerGO->GetGlobalOrientation());
		mActor->setGlobalPosition(mOwnerGO->GetGlobalPosition());
	}

	void GOCTrigger::SetParameters(DataMap *parameters)
	{
		_clear();
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