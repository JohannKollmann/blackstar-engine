
#include "IceGOComponent.h"
#include "IceGameObject.h"

namespace Ice
{

	void GOComponent::SetOwner(GameObject *go)
	{
		mOwnerGO = go;
		UpdatePosition(go->GetGlobalPosition());
		UpdateOrientation(go->GetGlobalOrientation());
		UpdateScale(go->GetGlobalScale());
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

	void GOComponent::SetOwnerPosition(const Ogre::Vector3 &position)
	{
		if (!mOwnerGO) return;
		mTransformingOwner = true;
		mOwnerGO->SetGlobalPosition(position);
		mTransformingOwner = false;
	}

	void GOComponent::SetOwnerOrientation(const Ogre::Quaternion &orientation)
	{
		if (!mOwnerGO) return;
		mTransformingOwner = true;
		mOwnerGO->SetGlobalOrientation(orientation);
		mTransformingOwner = false;
	}

};