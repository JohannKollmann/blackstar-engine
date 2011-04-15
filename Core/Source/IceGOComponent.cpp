
#include "IceGOComponent.h"
#include "IceGameObject.h"

namespace Ice
{

	void GOComponent::SetOwner(std::weak_ptr<GameObject> go)
	{
		mOwnerGO = go;
		NotifyOwnerGO();
	}
	void GOComponent::NotifyOwnerGO()
	{
		GameObjectPtr ownerGO = mOwnerGO.lock();
		if (!ownerGO.get()) return;
		UpdatePosition(ownerGO->GetGlobalPosition());
		UpdateOrientation(ownerGO->GetGlobalOrientation());
		UpdateScale(ownerGO->GetGlobalScale());
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

	void GOComponent::SetOwnerPosition(const Ogre::Vector3 &position, bool updateReferences, bool updateChildren)
	{
		if (mOwnerGO.expired()) return;
		mTransformingOwner = true;
		mOwnerGO.lock()->SetGlobalPosition(position, updateReferences, updateChildren);
		mTransformingOwner = false;
	}

	void GOComponent::SetOwnerOrientation(const Ogre::Quaternion &orientation, bool updateReferences, bool updateChildren)
	{
		if (mOwnerGO.expired()) return;
		mTransformingOwner = true;
		mOwnerGO.lock()->SetGlobalOrientation(orientation, updateReferences, updateChildren);
		mTransformingOwner = false;
	}

};