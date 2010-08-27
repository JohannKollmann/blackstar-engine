
#include "IceGOComponent.h"
#include "IceGameObject.h"

namespace Ice
{

	void GOComponent::SetOwner(GameObject *go)
	{
		mOwnerGO = go;
		NotifyOwnerGO();
	}
	void GOComponent::NotifyOwnerGO()
	{
		if (!mOwnerGO) return;
		UpdatePosition(mOwnerGO->GetGlobalPosition());
		UpdateOrientation(mOwnerGO->GetGlobalOrientation());
		UpdateScale(mOwnerGO->GetGlobalScale());
	}

	void GOComponent::_updatePosition(const Ogre::Vector3 &position)
	{
		if (!mTransformingOwner) UpdatePosition(position);
	}
	void GOComponent::_updateOrientation(const Ogre::Quaternion &quat)
	{
		if (!mTransformingOwner) UpdateOrientation(quat);
	}
	void GOComponent::_updateScale(const Ogre::Vector3 &scale)
	{
		if (!mTransformingOwner) UpdateScale(scale);
	}

	void GOComponent::SetOwnerPosition(const Ogre::Vector3 &position, bool updateChildren)
	{
		if (!mOwnerGO) return;
		mTransformingOwner = true;
		mOwnerGO->SetGlobalPosition(position, updateChildren);
		mTransformingOwner = false;
	}

	void GOComponent::SetOwnerOrientation(const Ogre::Quaternion &orientation, bool updateChildren)
	{
		if (!mOwnerGO) return;
		mTransformingOwner = true;
		mOwnerGO->SetGlobalOrientation(orientation, updateChildren);
		mTransformingOwner = false;
	}

};