
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
		if (!mTranformingOwner) UpdatePosition(position);
	}
	void GOComponent::_updateOrientation(const Ogre::Quaternion &quat)
	{
		if (!mTranformingOwner) UpdateOrientation(quat);
	}
	void GOComponent::_updateScale(const Ogre::Vector3 &scale)
	{
		if (!mTranformingOwner) UpdateScale(scale);
	}

	void GOComponent::SetOwnerPosition(const Ogre::Vector3 &position)
	{
		if (!mOwnerGO) return;
		mTranformingOwner = true;
		mOwnerGO->SetGlobalPosition(position);
		mTranformingOwner = false;
	}

	void GOComponent::SetOwnerOrientation(const Ogre::Quaternion &orientation)
	{
		if (!mOwnerGO) return;
		mTranformingOwner = true;
		mOwnerGO->SetGlobalOrientation(orientation);
		mTranformingOwner = false;
	}

};