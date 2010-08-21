
#include "IceGOCForceField.h"
#include "IceMain.h"
#include "IceGameObject.h"

namespace Ice
{

	GOCForceField::GOCForceField(void)
	{
		mForceField = nullptr;
		mEditorVisual = nullptr;
		mEditorVisual2 = nullptr;
		mActive = false;
		mFieldLinearKernelDesc.setToDefault();
	}

	GOCForceField::~GOCForceField(void)
	{
		_clear();
	}

	void GOCForceField::_clear()
	{
		ShowEditorVisual(false);
		if (mForceField) Main::Instance().GetPhysXScene()->destroyForcefield(mForceField);
		mForceField = nullptr;
	}
	void GOCForceField::_create()
	{
		_clear();
		if (!mActive) return;
		if (!Ogre::ResourceGroupManager::getSingleton().resourceExists("General", mCollisionMeshName))
		{
			Ogre::LogManager::getSingleton().logMessage("Error: Resource \"" + mCollisionMeshName + "\" does not exist. Loading dummy Resource...");
			mCollisionMeshName = "DummyMesh.mesh";
		}
		Ogre::Entity *entity = Main::Instance().GetOgreSceneMgr()->createEntity("tmpCollisionEnt", mCollisionMeshName);
		Ogre::Vector3 scale = Ogre::Vector3(1,1,1);
		if (mOwnerGO) scale = mOwnerGO->GetGlobalScale();
		NxForceFieldDesc fieldDesc;
		fieldDesc.setToDefault();
		fieldDesc.actor = nullptr;
		fieldDesc.coordinates = NX_FFC_CARTESIAN;
		if (mOwnerGO)
			fieldDesc.pose = OgrePhysX::Convert::toNx(mOwnerGO->GetGlobalPosition(), mOwnerGO->GetGlobalOrientation());
		mFieldLinearKernelDesc.falloffLinear = NxVec3(mFalloff, mFalloff, mFalloff);
		mFieldLinearKernelDesc.constant = OgrePhysX::Convert::toNx(mForceDirection * mForceMultiplier);
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
		_create();
	}

	void GOCForceField::UpdatePosition(Ogre::Vector3 position)
	{
		bool show = (mEditorVisual != nullptr);
		_create();
		if (show) ShowEditorVisual(show);
		//mForceField->setPose(OgrePhysX::Convert::toNx(position, mOwnerGO->GetGlobalOrientation()));
	}
	void GOCForceField::UpdateOrientation(Ogre::Quaternion orientation)
	{
		bool show = (mEditorVisual != nullptr);
		_create();
		if (show) ShowEditorVisual(show);
		//mForceField->setPose(OgrePhysX::Convert::toNx(mOwnerGO->GetGlobalPosition(), orientation));
	}
	void GOCForceField::UpdateScale(Ogre::Vector3 scale)
	{
		bool show = (mEditorVisual != nullptr);
		_create();
		if (show) ShowEditorVisual(show);
	}

	std::vector<ScriptParam> GOCForceField::Forcefield_Activate(Script& caller, std::vector<ScriptParam> &vParams)
	{
		mActive = vParams[0].getBool();
		_create();
		return std::vector<ScriptParam>();
	}

	void GOCForceField::ShowEditorVisual(bool show)
	{
		if (show)
		{
			if (!mEditorVisual && mCollisionMeshName != "")
			{
				mEditorVisual = Main::Instance().GetOgreSceneMgr()->createEntity(mCollisionMeshName);
				GetNode()->attachObject(mEditorVisual);
				for (unsigned int i = 0; i < mEditorVisual->getNumSubEntities(); i++)
					mEditorVisual->getSubEntity(i)->setMaterialName("Editor_TransparentBlue");

				mEditorVisual2 = Main::Instance().GetOgreSceneMgr()->createEntity("blockpfeil.mesh");
				mEditorVisual2->setUserAny(Ogre::Any(mOwnerGO));
				mArrowNode = GetNode()->createChildSceneNode();
				//mArrowNode->setInheritOrientation(false);
				mArrowNode->setOrientation(Ogre::Vector3::UNIT_X.getRotationTo(mForceDirection));
				mArrowNode->attachObject(mEditorVisual2);
			}
		}
		else
		{
			if (mEditorVisual)
			{
				GetNode()->detachObject(mEditorVisual);
				mArrowNode->detachObject(mEditorVisual2);
				Main::Instance().GetOgreSceneMgr()->destroySceneNode(mArrowNode);
				Main::Instance().GetOgreSceneMgr()->destroyEntity(mEditorVisual);
				Main::Instance().GetOgreSceneMgr()->destroyEntity(mEditorVisual2);
			}
			mEditorVisual = nullptr;
			mEditorVisual2 = nullptr;
			mArrowNode = nullptr;
		}
	}

	void GOCForceField::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("float", &mForceMultiplier, "ForceMultipler");
		mgr.SaveAtom("float", &mFalloff, "Falloff");
	}
	void GOCForceField::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("float", &mForceMultiplier);
		mgr.LoadAtom("float", &mFalloff);
	}

}