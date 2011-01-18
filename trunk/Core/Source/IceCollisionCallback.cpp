
#include "IceCollisionCallback.h"
#include "IceMain.h"
#include "IceGameObject.h"
#include "IceGOCPhysics.h"
#include "IceMessageSystem.h"
#include "IceSceneManager.h"
#include "IceLevelMesh.h"
#include "IceIncludes.h"
#include "OgreOggSound.h"
#include "IceScriptSystem.h"
#include "IceGOCView.h"

namespace Ice
{

	void TriggerCallback::onEnter(NxActor &trigger, NxActor &other)
	{
		if (trigger.userData && other.userData)
		{
			GameObject *triggerObject = (GameObject*)trigger.userData;
			GOCTrigger *trigger = (GOCTrigger*)triggerObject->GetComponent("Trigger");
			if (trigger)
			{
				GameObject *otherObject = (GameObject*)other.userData;
				trigger->onEnter(otherObject);
			}
		}
	}
	void TriggerCallback::onLeave(NxActor &trigger, NxActor &other)
	{
		if (trigger.userData && other.userData)
		{
			GameObject *triggerObject = (GameObject*)trigger.userData;
			GOCTrigger *trigger = (GOCTrigger*)triggerObject->GetComponent("Trigger");
			if (trigger)
			{
				GameObject *otherObject = (GameObject*)other.userData;
				trigger->onLeave(otherObject);
			}
		}
	}


	void PhysXUserCallback::onSleep(NxActor** actors, NxU32 count)
	{
		for (unsigned int i = 0; i < count; i++)
		{
			Msg msg;
			msg.type = "ACTOR_ONSLEEP";
			msg.rawData = actors[i];
			MessageSystem::Instance().SendInstantMessage(msg);
		}
	}

	void PhysXUserCallback::onWake(NxActor** actors, NxU32 count)
	{
		for (unsigned int i = 0; i < count; i++)
		{
			Msg msg;
			msg.type = "ACTOR_ONWAKE";
			msg.rawData = actors[i];
			MessageSystem::Instance().SendInstantMessage(msg);
		}
	}

	bool PhysXUserCallback::onJointBreak(NxReal breakingImpulse, NxJoint& brokenJoint)
	{
		return true;
	}


	void ActorContactReport::onMaterialContact(Ogre::String material1, Ogre::String material2, Ogre::Vector3 position, float force )
	{
		//Ogre::LogManager::getSingleton().logMessage("OnMaterialContact: " + material1 + " - " + material2 + "  Force: " + Ogre::StringConverter::toString(force));
		Msg msg;
		msg.type = "MATERIAL_ONCONTACT";
		msg.params.AddOgreVec3("Position", position);
		msg.params.AddOgreString("Material1", material1);
		msg.params.AddOgreString("Material2", material2);
		msg.params.AddFloat("Force", force);
		MessageSystem::Instance().SendInstantMessage(msg);
	}


	void ActorContactReport::onContactNotify(NxContactPair &pair, NxU32 events)
	{
		// Iterate through contact points
		NxContactStreamIterator i(pair.stream);
		//user can call getNumPairs() here
		while(i.goNextPair())
		{
			NxShape *shape1 = i.getShape(0);
			NxShape *shape2 = i.getShape(1);
			if	((shape1->getGroup() == CollisionGroups::BONE || shape1->getGroup() == CollisionGroups::AI)
				&& (shape2->getGroup() == CollisionGroups::BONE || shape2->getGroup() == CollisionGroups::AI))
					continue;

			//First material callback
			Ogre::String material1 = SceneManager::Instance().GetSoundMaterialTable().GetMaterialName(shape1->getMaterial());
			Ogre::String material2 = SceneManager::Instance().GetSoundMaterialTable().GetMaterialName(shape2->getMaterial());

			float summed_force = 0.0f;
			Ogre::Vector3 contactPoint;
			while(i.goNextPatch())
			{
				//user can also call getPatchNormal() and getNumPoints() here
				const NxVec3& contactNormal = i.getPatchNormal();
				bool once = true;
				while(i.goNextPoint())
				{
					//user can also call getPoint() and getSeparation() here
					contactPoint = OgrePhysX::Convert::toOgre(i.getPoint());
					summed_force += i.getPointNormalForce();
					if (!once) continue;
					once = false;

					NxTriangleMeshShape *meshShape = 0;
					int triIndex = 0;
					bool mat1 = true;
					if (shape1->getType() == NxShapeType::NX_SHAPE_MESH)
					{
						meshShape = (NxTriangleMeshShape*)shape1;
						triIndex = i.getFeatureIndex0();
					}
					else if (shape2->getType() == NxShapeType::NX_SHAPE_MESH)
					{
						mat1 = false;
						meshShape = (NxTriangleMeshShape*)shape2;
						triIndex = i.getFeatureIndex1();
					}
					if (meshShape)
					{
						Ogre::Entity *ent = nullptr;
						if (!meshShape->getActor().userData)
						{
							ent = SceneManager::Instance().GetLevelMesh()->GetEntity();
						}
						else
						{
							GameObject *object = (GameObject*)meshShape->getActor().userData;
							GOCMeshRenderable *gocMesh = nullptr;
							if (gocMesh = object->GetComponent<GOCMeshRenderable>())
								ent = gocMesh->GetEntity();
						}
						if (ent)
						{
							int triCount = 0;
							for (unsigned i = 0; i < ent->getMesh()->getNumSubMeshes(); i++)
							{
								Ogre::SubMesh *subMesh = ent->getMesh()->getSubMesh(i);
								int subMeshTriCount = subMesh->indexData->indexCount / 3;
								if (triIndex >= triCount && triIndex < triCount+subMeshTriCount)
								{
									Ogre::String meshMat = SceneManager::Instance().GetSoundMaterialTable().GetMaterialName(subMesh->getMaterialName());
									if (mat1) material1 = meshMat;
									else material2 = meshMat;
									break;
								}
								triCount += subMeshTriCount;
							}
						}
					}
				}
			}
			onMaterialContact(material1, material2, contactPoint, summed_force);
		}
	}

	ActorContactReport::ActorContactReport()
	{
	}

};