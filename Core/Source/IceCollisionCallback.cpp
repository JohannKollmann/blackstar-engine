
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
#include "IceMaterialTable.h"

namespace Ice
{

	PhysXFilterData& PhysXFilterData::Instance()
	{
		static PhysXFilterData TheOneAndOnly;
		return TheOneAndOnly;
	};

	PhysXFilterData::PhysXFilterData()
	{
		DynamicBody.word0 = CollisionGroups::DYNAMICBODY;
		DynamicBody.word1 = ~0;
		DynamicBody.word2 = ~0;

		StaticBody.word0 = CollisionGroups::STATICBODY;
		StaticBody.word1 = ~0;
		StaticBody.word2 = ~0;

		Character.word0 = CollisionGroups::CHARACTER;
		Character.word1 = ~0;
		Character.word2 = ~0;

		Bone.word0 = CollisionGroups::BONE;
		Bone.word1 = CollisionGroups::DYNAMICBODY | CollisionGroups::STATICBODY;
		Bone.word2 = CollisionGroups::DYNAMICBODY | CollisionGroups::STATICBODY;

		Intern.word0 = CollisionGroups::INTERN;
		Intern.word1 = 0;
		Intern.word2 = 0;
	}

	PxFilterFlags PhysXSimulationFilterShader (	
		PxFilterObjectAttributes attributes0,
		PxFilterData filterData0, 
		PxFilterObjectAttributes attributes1,
		PxFilterData filterData1,
		PxPairFlags& pairFlags,
		const void* constantBlock,
		PxU32 constantBlockSize)
	{
		// let triggers through
		if(PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
			return PxFilterFlags();
		}

		//word0 stores the group id, word1 a bitmask that determines for which groups collision is enabled.
		if (filterData0.word0 & filterData1.word1 && filterData1.word0 & filterData0.word1)
		{
			pairFlags = PxPairFlag::eCONTACT_DEFAULT;

			//word2 is a bitmask that determines for which groups contact notification is enabled.
			if ((filterData0.word0 & filterData1.word2 && filterData1.word0 & filterData0.word2))
			{
				pairFlags = pairFlags	| PxPairFlag::eNOTIFY_TOUCH_FOUND
										| PxPairFlag::eNOTIFY_CONTACT_POINTS
										| PxPairFlag::eNOTIFY_CONTACT_POINTS
										| PxPairFlag::eNOTIFY_CONTACT_FORCES
										| PxPairFlag::eNOTIFY_CONTACT_FORCE_PER_POINT
										| PxPairFlag::eNOTIFY_CONTACT_FEATURE_INDICES_PER_POINT;
			}

			return PxFilterFlags();
		}
		else return PxFilterFlag::eSUPPRESS;		//no collision
	}

	void PhysXSimulationEventCallback::onContact(PxContactPair &pair, PxU32 events)
	{
		// Iterate through contact points
		PxContactStreamIterator i(pair.stream);
		//user can call getNumPairs() here
		while(i.goNextPair())
		{
			PxShape *shape1 = i.getShape(0);
			PxShape *shape2 = i.getShape(1);

			PxMaterial *mat1 = nullptr, *mat2 = nullptr;

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

					//IceNote("Feature Index 0: " + Ogre::StringConverter::toString(i.getFeatureIndex0()) + " Feature Index 1: " + Ogre::StringConverter::toString(i.getFeatureIndex1()))

					if (! (shape1->getNbMaterials() > 0 && shape2->getNbMaterials() > 0) )
						continue;

					PxMaterial *matBuffer[1];
					shape1->getMaterials(matBuffer, 1);
					mat1 = matBuffer[0];
					shape2->getMaterials(matBuffer, 1);
					mat2 = matBuffer[0];

					if (shape1->getGeometryType() == PxGeometryType::eTRIANGLEMESH)
					{
						PxU32 featureIndex = i.getFeatureIndex0();
						mat1 = shape1->getMaterialFromInternalFaceIndex(featureIndex);

					}
					else if (shape2->getGeometryType() == PxGeometryType::eTRIANGLEMESH)
					{
						PxU32 featureIndex = i.getFeatureIndex1();
						mat2 = shape2->getMaterialFromInternalFaceIndex(featureIndex);
					}

					/*if (meshShape)
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
					}*/
				}
			}

			if (mat1 && mat2)
				onMaterialContact(mat1, mat2, contactPoint, summed_force);
		}
	}

	void PhysXSimulationEventCallback::onSleep(PxActor** actors, PxU32 count)
	{
		for (unsigned int i = 0; i < count; i++)
		{
			Msg msg;
			msg.typeID = GlobalMessageIDs::ACTOR_ONSLEEP;
			msg.rawData = actors[i];
			MessageSystem::Instance().MulticastMessage(msg);
		}
	}

	void PhysXSimulationEventCallback::onWake(PxActor** actors, PxU32 count)
	{
		for (unsigned int i = 0; i < count; i++)
		{
			Msg msg;
			msg.typeID = GlobalMessageIDs::ACTOR_ONWAKE;
			msg.rawData = actors[i];
			MessageSystem::Instance().MulticastMessage(msg);
		}			
	}

	void PhysXSimulationEventCallback::onTrigger(PxTriggerPair *pairs, PxU32 count)
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

	void PhysXSimulationEventCallback::onConstraintBreak(PxConstraintInfo *constraints, PxU32 count)
	{
	}


	void PhysXSimulationEventCallback::onMaterialContact(PxMaterial *mat1, PxMaterial *mat2, const Ogre::Vector3 &position, float force)
	{
		Msg msg;
		msg.typeID = GlobalMessageIDs::MATERIAL_ONCONTACT;
		msg.params.AddOgreVec3("Position", position);
		msg.params.AddOgreString("Material1", MaterialTable::Instance().GetMaterialNameByPx(mat1));
		msg.params.AddOgreString("Material2", MaterialTable::Instance().GetMaterialNameByPx(mat2));
		msg.params.AddFloat("Force", force);
		MessageSystem::Instance().MulticastMessage(msg);
	}

};