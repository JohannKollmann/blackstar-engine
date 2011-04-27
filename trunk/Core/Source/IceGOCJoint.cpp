
#include "IceGOCJoint.h"
#include "IceGOCPhysics.h"
#include "IceMain.h"

namespace Ice
{

	GOCJoint::~GOCJoint(void)
	{
		clear();
	}

	void GOCJoint::createJoint(NxJointDesc &desc)
	{
		clear();
		GameObjectPtr owner = mOwnerGO.lock();
		IceAssert(owner.get());
		std::vector<GameObjectPtr> jointActors;
		owner->GetReferencedObjects(ReferenceTypes::JOINTACTOR, jointActors);
		IceAssert(jointActors.size() == 2)
		GOCRigidBody *body1 = jointActors[0]->GetComponent<GOCRigidBody>();
		GOCRigidBody *body2 = jointActors[1]->GetComponent<GOCRigidBody>();
		IceAssert(body1 && body2)
		IceAssert(body1 != body2)
		body1->GetActor()->getNxActor()->setSolverIterationCount(16);
		body2->GetActor()->getNxActor()->setSolverIterationCount(16);
		desc.actor[0] = body1->GetActor()->getNxActor();
		desc.actor[1] = body2->GetActor()->getNxActor();
		desc.maxForce = mMaxForce;
		desc.maxTorque = mMaxTorque;
		mPhysXJoint = Main::Instance().GetPhysXScene()->getNxScene()->createJoint(desc);
		mPhysXJoint->userData = owner.get();
	}

	void GOCJoint::clear()
	{
		if (mPhysXJoint)
		{
			Main::Instance().GetPhysXScene()->getNxScene()->releaseJoint(*mPhysXJoint);
			mPhysXJoint = nullptr;
		}
	}

	void GOCJoint::SetActorObjects(GameObjectPtr actor1, GameObjectPtr actor2)
	{
		GameObjectPtr owner = GetOwner();
		IceAssert(owner.get());

		owner->RemoveObjectReferences(ReferenceTypes::JOINTACTOR);

		actor1->AddObjectReference(owner, ObjectReference::PERSISTENT|ObjectReference::OWNER|ObjectReference::MOVEIT_USER, ReferenceTypes::JOINT);
		actor2->AddObjectReference(owner, ObjectReference::PERSISTENT|ObjectReference::OWNER|ObjectReference::MOVEIT_USER, ReferenceTypes::JOINT);
		owner->AddObjectReference(actor1, ObjectReference::PERSISTENT|ObjectReference::OWNED, ReferenceTypes::JOINTACTOR);
		owner->AddObjectReference(actor2, ObjectReference::PERSISTENT|ObjectReference::OWNED, ReferenceTypes::JOINTACTOR);
	}

	std::vector<ScriptParam> GOCJoint::SetActorObjects(Script &caller, std::vector<ScriptParam> &params)
	{
		GameObjectPtr obj1 = SceneManager::Instance().GetObjectByInternID(params[0].getInt());
		GameObjectPtr obj2 = SceneManager::Instance().GetObjectByInternID(params[1].getInt());
		if (!obj1.get() || !obj2.get())
		{
			SCRIPT_RETURNERROR("Invalid object ID!")
		}
		GOCRigidBody *body1 = obj1->GetComponent<GOCRigidBody>();
		GOCRigidBody *body2 =obj2->GetComponent<GOCRigidBody>();
		if (!body1 || !body2 || body1 == body2)
		{
			SCRIPT_RETURNERROR("Invalid objects!")
		}
		SetActorObjects(obj1, obj2);
		NotifyPostInit();
		SCRIPT_RETURN()
	}

	void GOCJoint::Save(LoadSave::SaveSystem& mgr)
	{
		mgr.SaveAtom("float", &mMaxForce, "Max Force");
		mgr.SaveAtom("float", &mMaxTorque, "Max Torque");
	}
	void GOCJoint::Load(LoadSave::LoadSystem& mgr)
	{
		mgr.LoadAtom("float", &mMaxForce);
		mgr.LoadAtom("float", &mMaxTorque);
	}

	void GOCFixedJoint::NotifyPostInit()
	{
		clear();
		if (!mOwnerGO.expired())
		{
			NxFixedJointDesc desc;
			createJoint(desc);
		}
	}

	void GOCFixedJoint::UpdatePosition(Ogre::Vector3 position)
	{
		/*GameObjectPtr owner = mOwnerGO.lock();
		if (owner.get())
		{
			std::vector<GameObjectPtr> jointActors;
			owner->GetReferencedObjects(ReferenceTypes::JOINTACTOR, jointActors);
			if (jointActors.size() == 2)
			{
				SetOwnerPosition(jointActors[0]->GetGlobalPosition() + ((jointActors[1]->GetGlobalPosition() - jointActors[0]->GetGlobalPosition()) * 0.5f));
			}
		}*/
	}

}