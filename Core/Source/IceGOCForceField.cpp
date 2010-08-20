
#include "IceGOCForceField.h"
#include "IceMain.h"
#include "IceGameObject.h"

namespace Ice
{

	GOCForceField::GOCForceField(void)
	{
		mForceField = nullptr;
	}

	GOCForceField::~GOCForceField(void)
	{
		_clear();
	}

	void GOCForceField::_clear()
	{
		if (mForceField) Main::Instance().GetPhysXScene()->destroyForcefield(mForceField);
		mForceField = nullptr;
	}
	void GOCForceField::_create()
	{
		_clear();
		if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", mCollisionMeshName))
		{
			Ogre::LogManager::getSingleton().logMessage("Error: Resource \"" + mCollisionMeshName + "\" does not exist. Loading dummy Resource...");
			mCollisionMeshName = "DummyMesh.mesh";
		}
		Ogre::Entity *entity = Main::Instance().GetOgreSceneMgr()->createEntity("tmpCollisionEnt", mCollisionMeshName);
		Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
		NxForceFieldDesc fieldDesc;
		fieldDesc.setToDefault();
		if (mOwnerGO) scale = mOwnerGO->GetGlobalScale();
		if (mShapeType ==  Shapes::SHAPE_SPHERE)
		{
			float sphereShapeRadius = entity->getBoundingRadius();
			OgrePhysX::SphereShape shape(sphereShapeRadius * ((scale.x + scale.y + scale.z) / 3));
			shape.group(CollisionGroups::DEFAULT);
			mForceField = Main::Instance().GetPhysXScene()->createForceField(
				shape, fieldDesc, mFieldLinearKernelDesc);
		}
		else if (mShapeType == Shapes::SHAPE_CAPSULE)
		{
			Ogre::Vector3 cubeShapeSize = entity->getBoundingBox().getSize();
			cubeShapeSize = cubeShapeSize * scale;
			float capsule_radius = cubeShapeSize.x > cubeShapeSize.z ? cubeShapeSize.x : cubeShapeSize.z;
			float offset = 0.0f;
			OgrePhysX::CapsuleShape shape(capsule_radius * 0.5f, cubeShapeSize.y * 0.5f + offset);
			shape.group(CollisionGroups::DEFAULT);
			if (cubeShapeSize.y - capsule_radius > 0.0f) offset = (cubeShapeSize.y / capsule_radius) * 0.1f;
			mForceField = Main::Instance().GetPhysXScene()->createForceField(
				shape, fieldDesc, mFieldLinearKernelDesc);
		}
		else if (mShapeType == Shapes::SHAPE_CONVEX)
		{
			OgrePhysX::RTConvexMeshShape shape(entity->getMesh());
			shape.group(CollisionGroups::DEFAULT);
			shape.scale(scale);
			mForceField = Main::Instance().GetPhysXScene()->createForceField(
				shape, fieldDesc, mFieldLinearKernelDesc);
		}
		else		//Default: Box
		{
			OgrePhysX::BoxShape shape(entity, scale);
			shape.group(CollisionGroups::DEFAULT);
			mForceField = Main::Instance().GetPhysXScene()->createForceField(
				shape, fieldDesc, mFieldLinearKernelDesc);
		}
		Main::Instance().GetOgreSceneMgr()->destroyEntity(entity);
	}

	void GOCForceField::OnSetParameters()
	{
	}

	void GOCForceField::UpdatePosition(Ogre::Vector3 position)
	{
		mForceField->setPose(OgrePhysX::Convert::toNx(position, mOwnerGO->GetGlobalOrientation()));
	}
	void GOCForceField::UpdateOrientation(Ogre::Quaternion orientation)
	{
		mForceField->setPose(OgrePhysX::Convert::toNx(mOwnerGO->GetGlobalPosition(), orientation));
	}
	void GOCForceField::UpdateScale(Ogre::Vector3 scale)
	{
		_create();
	}

	void GOCForceField::Save(LoadSave::SaveSystem& mgr)
	{
	}
	void GOCForceField::Load(LoadSave::LoadSystem& mgr)
	{
	}

}