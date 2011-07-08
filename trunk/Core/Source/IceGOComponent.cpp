
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

	void GOComponent::ReceiveMessage(Msg &msg)
	{
		if (msg.typeID == GameObject::MessageIDs::UPDATE_COMPONENT_TRANSFORM)
		{
			GameObjectPtr owner = mOwnerGO.lock();
			if (owner)
			{
				UpdatePosition(owner->GetGlobalPosition());
				UpdateOrientation(owner->GetGlobalOrientation());
			}
		}
		if (msg.typeID == GameObject::MessageIDs::UPDATE_COMPONENT_POSITION)
			UpdatePosition(msg.params.GetValue<Ogre::Vector3>(0));
		else if (msg.typeID == GameObject::MessageIDs::UPDATE_COMPONENT_ORIENTATION)
			UpdateOrientation(msg.params.GetValue<Ogre::Quaternion>(0));
		else if (msg.typeID == GameObject::MessageIDs::UPDATE_COMPONENT_SCALE)
			UpdateScale(msg.params.GetValue<Ogre::Vector3>(0));
	}

	void GOComponent::SetOwnerPosition(const Ogre::Vector3 &position, bool updateReferences, bool updateChildren)
	{
		GameObjectPtr owner = mOwnerGO.lock();
		if (!owner.get()) return;
		owner->SetGlobalPosition(position, false, updateReferences, updateChildren);
		Msg msg; msg.typeID = GameObject::MessageIDs::UPDATE_COMPONENT_POSITION;
		msg.params.AddOgreVec3("Position", position);
		owner->BroadcastObjectMessage(msg, this);
	}

	void GOComponent::SetOwnerOrientation(const Ogre::Quaternion &orientation, bool updateReferences, bool updateChildren)
	{
		GameObjectPtr owner = mOwnerGO.lock();
		if (!owner.get()) return;
		owner->SetGlobalOrientation(orientation, false, updateReferences, updateChildren);
		Msg msg; msg.typeID = GameObject::MessageIDs::UPDATE_COMPONENT_ORIENTATION;
		msg.params.AddOgreQuat("Orientation", orientation);
		owner->BroadcastObjectMessage(msg, this);
	}

	void GOComponent::SetOwnerTransform(const Ogre::Vector3 &position, const Ogre::Quaternion &orientation, bool updateReferences, bool updateChildren)
	{
		GameObjectPtr owner = mOwnerGO.lock();
		if (!owner.get()) return;
		owner->SetGlobalPosition(position, false, updateReferences, updateChildren);
		owner->SetGlobalOrientation(orientation, false, updateReferences, updateChildren);
		Msg msg; msg.typeID = GameObject::MessageIDs::UPDATE_COMPONENT_TRANSFORM;
		owner->BroadcastObjectMessage(msg, this);
	}

	void GOComponent::SendObjectMessage(Msg &msg, GOComponent::FamilyID &familyID)
	{
		GameObjectPtr owner = mOwnerGO.lock();
		IceAssert(owner.get())
		owner->SendObjectMessage(msg, familyID);
	}

	void GOComponent::BroadcastObjectMessage(Msg &msg)
	{
		GameObjectPtr owner = mOwnerGO.lock();
		IceAssert(owner.get())
		owner->BroadcastObjectMessage(msg, this);
	}

};