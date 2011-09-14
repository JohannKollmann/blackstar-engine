
#include "IceCollisionCallback.h"
#include "IceMain.h"
#include "IceGameObject.h"
#include "IceGOCPhysics.h"
#include "IceMessageSystem.h"
#include "IceSceneManager.h"
#include "IceIncludes.h"
#include "OgreOggSound.h"
#include "IceScriptSystem.h"
#include "IceGOCView.h"

namespace Ice
{

		void PhysXContactReport::onContact(PxContactPair &pair, PxU32 events)
		{
			// Iterate through contact points
			PxContactStreamIterator i(pair.stream);
			//user can call getNumPairs() here
			while(i.goNextPair())
			{
				PxShape *shape1 = i.getShape(0);
				PxShape *shape2 = i.getShape(1);

				//First material callback
				shape1->getMaterialFromInternalFaceIndex(0);
				shape2->getMaterialFromInternalFaceIndex(0);

				float summed_force = 0.0f;
				Ogre::Vector3 contactPoint;
				while(i.goNextPatch())
				{
					//user can also call getPatchNormal() and getNumPoints() here
					const PxVec3& contactNormal = i.getPatchNormal();
					bool once = true;
					while(i.goNextPoint())
					{
						//user can also call getPoint() and getSeparation() here
						contactPoint = OgrePhysX::Convert::toOgre(i.getPoint());
						summed_force += i.getPointNormalForce();
						if (!once) continue;
						once = false;

						/*PxTriangleMeshShape *meshShape = 0;
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
							if (meshShape->getActor().userData)
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
						}*/
					}
				}
				//onMaterialContact(material1, material2, contactPoint, summed_force);
			}
		}

		void PhysXContactReport::onSleep(PxActor** actors, PxU32 count)
		{
			for (unsigned int i = 0; i < count; i++)
			{
				Msg msg;
				msg.typeID = GlobalMessageIDs::ACTOR_ONSLEEP;
				msg.rawData = actors[i];
				MessageSystem::Instance().MulticastMessage(msg);
			}
		}

		void PhysXContactReport::onWake(PxActor** actors, PxU32 count)
		{
			for (unsigned int i = 0; i < count; i++)
			{
				Msg msg;
				msg.typeID = GlobalMessageIDs::ACTOR_ONWAKE;
				msg.rawData = actors[i];
				MessageSystem::Instance().MulticastMessage(msg);
			}			
		}

		void PhysXContactReport::onTrigger(PxTriggerPair *pairs, PxU32 count)
		{
			if (pairs->triggerShape->getActor().userData && pairs->otherShape->getActor().userData)
			{
				GameObject *triggerObject = (GameObject*)pairs->triggerShape->getActor().userData;
				IceAssert(triggerObject)
				GOCTrigger *trigger = triggerObject->GetComponent<GOCTrigger>();
				if (trigger)
				{
					GameObject *otherObject = (GameObject*)pairs->otherShape->getActor().userData;

					if (pairs->status == PxPairFlag::eNOTIFY_TOUCH_FOUND) trigger->onEnter(otherObject);
					else if (pairs->status == PxPairFlag::eNOTIFY_TOUCH_LOST) trigger->onLeave(otherObject);
				}
			}
		}

		void PhysXContactReport::onConstraintBreak(PxConstraintInfo *constraints, PxU32 count)
		{
		}


		void PhysXContactReport::onMaterialContact(Ogre::String material1, Ogre::String material2, Ogre::Vector3 position, float force)
		{
			Msg msg;
			msg.typeID = GlobalMessageIDs::MATERIAL_ONCONTACT;
			msg.params.AddOgreVec3("Position", position);
			msg.params.AddOgreString("Material1", material1);
			msg.params.AddOgreString("Material2", material2);
			msg.params.AddFloat("Force", force);
			MessageSystem::Instance().MulticastMessage(msg);
		}

};