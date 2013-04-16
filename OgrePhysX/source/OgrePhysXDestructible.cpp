
#include "OgrePhysXDestructible.h"
#include "OgrePhysXActor.h"
#include "OgrePhysXNodeRenderable.h"
#include "../TinyXML/tinyxml.h"

namespace OgrePhysX
{
	Destructible::Destructible(Scene *scene, const Ogre::String &meshSplitConfigFile, float breakForce, float breakTorque, float density, PxMaterial &material, const Ogre::Vector3 &scale)
		: mPointRenderable(nullptr), mBreakForce(breakForce), mBreakTorque(breakTorque), mDensity(density), mScale(scale)
	{
		mScene = scene;

		mMaterial = &material;

		float breakForceScale = scale.x*scale.y*scale.z*mDensity;
		mBreakForce *= breakForceScale;
		mBreakTorque *= breakForceScale;

		TiXmlDocument document;
		Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource(meshSplitConfigFile);
		if (stream.isNull())
		{
			Ogre::LogManager::getSingleton().logMessage("[OgrePhysX] Could not open file: " + meshSplitConfigFile);
			return;
		}

		//Get the file contents
		Ogre::String data = stream->getAsString();

		//Parse the XML c
		document.Parse(data.c_str());
		stream->close();
		if (document.Error())
		{
			Ogre::LogManager::getSingleton().logMessage("[OgrePhysX] Could not parse file: " + meshSplitConfigFile);
			return;
		}

		TiXmlElement* rootElement = document.FirstChildElement();
		TiXmlElement* childElement = rootElement->FirstChildElement();
		while(childElement)
		{
			SplitPart *part1 = getOrCreatePart(childElement->Attribute("name"));
			TiXmlElement* connectionElement = childElement->FirstChildElement();
			while(connectionElement)
			{
				SplitPart *part2 = getOrCreatePart(connectionElement->Attribute("target"));

				TiXmlElement* attributeElement = connectionElement->FirstChildElement();
				float breakForce = mBreakForce;
				float breakTorque = mBreakTorque;
				while(attributeElement)
				{
					double d;
					if (attributeElement->ValueStr() == "rel_force")
					{
						 attributeElement->Attribute("value", &d);
						 breakForce *= d;
					}
					if (attributeElement->ValueStr() == "rel_torque")
					{
						 attributeElement->Attribute("value", &d);
						 breakTorque *= d;
					}

					attributeElement = attributeElement->NextSiblingElement();
				}

				//connect the parts
				createJoint(part1, part2, breakForce, breakTorque);

				connectionElement = connectionElement->NextSiblingElement();
			}

			childElement = childElement->NextSiblingElement();
		}
	}

	Destructible::SplitPart::SplitPart(Destructible *destructible, const Ogre::String &meshName)
	{
		mDestructible = destructible;

		mWasRendered = false;

		Ogre::SceneNode *node = mDestructible->mScene->getOgreSceneMgr()->getRootSceneNode()->createChildSceneNode();
		Ogre::Entity* ent = mDestructible->mScene->getOgreSceneMgr()->createEntity(meshName);
		node->attachObject(ent);
		node->setScale(mDestructible->mScale);

		PxTransform shapeOffset = PxTransform::createIdentity();

		float shapeSizeFactor = 0.75f;

		Ogre::Vector3 center = ent->getBoundingBox().getCenter() * mDestructible->mScale;

		Ogre::Vector3 scale = mDestructible->mScale * shapeSizeFactor;

		Ogre::Vector3 boundingBoxHalfSize = ent->getBoundingBox().getHalfSize() * scale;

		Cooker::Params params;
		params.scale(scale);
		if (boundingBoxHalfSize.x > 0.1f &&
			boundingBoxHalfSize.y > 0.1f &&
			boundingBoxHalfSize.z > 0.1f)
		{
			Ogre::Vector3 scaledOffset = center * shapeSizeFactor;
			shapeOffset.p = Convert::toPx(center - scaledOffset);

			mActor = mDestructible->mScene->createRigidDynamic(Geometry::convexMeshGeometry(ent->getMesh(), params), mDestructible->mDensity, *mDestructible->mMaterial, shapeOffset); 
		}
		else 
		{
			//mesh is too small for convex mesh
			PxBoxGeometry boxGeometry = PxBoxGeometry(Convert::toPx(boundingBoxHalfSize));
			shapeOffset.p = Convert::toPx(center);
			mActor = mDestructible->mScene->createRigidDynamic(boxGeometry, mDestructible->mDensity, *mDestructible->mMaterial, shapeOffset); 
		}

		mRenderedActor = new RenderedActorBinding(mActor.getPxActor(), new NodeRenderable(node));
		((NodeRenderable*)mRenderedActor->getPointRenderable())->getOgreNode();
	}

	Destructible::SplitPart::~SplitPart()
	{
		if (mRenderedActor)
		{
			mDestructible->mScene->removeActor(mRenderedActor->getActor());
			NodeRenderable *nodeRenderable = (NodeRenderable*)mRenderedActor->getPointRenderable();
			Ogre::Entity *ent = (Ogre::Entity*)nodeRenderable->getOgreNode()->getAttachedObject(0);
			mDestructible->mScene->getOgreSceneMgr()->destroyEntity(ent);
			mDestructible->mScene->getOgreSceneMgr()->destroySceneNode(nodeRenderable->getOgreNode());
			delete nodeRenderable;
		}
	}

	Destructible::~Destructible()
	{
		for (auto i = mParts.begin(); i != mParts.end(); i++)
			delete i->second;

		for (auto i = mJoints.begin(); i != mJoints.end(); i++)
		{
			//TODO: remove joints
		}
	}

	Destructible::SplitPart* Destructible::getOrCreatePart(const Ogre::String &meshName)
	{
		auto find = mParts.find(meshName);
		if (find != mParts.end()) return find->second;

		SplitPart *part = new SplitPart(this, meshName);
		mParts.insert(std::make_pair(meshName, part));
		return part;
	}

	void Destructible::createJoint(SplitPart *part1, SplitPart *part2, float breakForce, float breakTorque)
	{
		part1->getRenderedActor()->getActor()->setSolverIterationCounts(16);
		part2->getRenderedActor()->getActor()->setSolverIterationCounts(16);
		PxFixedJoint *joint = PxFixedJointCreate(*OgrePhysX::getPxPhysics(),
			part1->getRenderedActor()->getActor(), PxTransform::createIdentity(),
			part2->getRenderedActor()->getActor(), PxTransform::createIdentity());

		part1->mEdges.push_back(SplitPart::Neighbor(part2, joint));
		part2->mEdges.push_back(SplitPart::Neighbor(part1, joint));

		joint->setBreakForce(breakForce, breakTorque);

		joint->setConstraintFlag(PxConstraintFlag::eCOLLISION_ENABLED, false);

		mJoints.push_back(joint);
	}

	void Destructible::setFreezed(bool freezed)
	{
		for (auto i = mParts.begin(); i != mParts.end(); i++)
			i->second->getActor().setFreezed(freezed);
	}

	void Destructible::setSimulationFilterData(PxFilterData &data)
	{
		for (auto i = mParts.begin(); i != mParts.end(); i++)
			i->second->getActor().getFirstShape()->setSimulationFilterData(data);
	}

	void Destructible::setQueryFilterData(PxFilterData &data)
	{
		for (auto i = mParts.begin(); i != mParts.end(); i++)
			i->second->getActor().getFirstShape()->setQueryFilterData(data);
	}

	void Destructible::setGlobalPosition(const Ogre::Vector3 &position)
	{
		for (auto i = mParts.begin(); i != mParts.end(); i++)
		{
			i->second->getActor().setGlobalPosition(position);	
		}
	}

	void Destructible::setOrientation(const Ogre::Quaternion &orientation)
	{
		for (auto i = mParts.begin(); i != mParts.end(); i++)
		{
			i->second->getActor().setGlobalOrientation(orientation);
		}
	}

	void Destructible::renderPart(SplitPart *part, const Ogre::Vector3 &position, const Ogre::Quaternion &orientation)
	{
		if (part->mWasRendered) return;

		part->getRenderedActor()->getPointRenderable()->setTransform(position, orientation);
		part->mWasRendered = true;

		for (auto i = part->mEdges.begin(); i != part->mEdges.end(); i++)
			if (!(i->joint->getConstraintFlags() & PxConstraintFlag::eBROKEN))
				renderPart(i->splitPart, position, orientation);
	}

	void Destructible::sync()
	{
		if (mParts.size() > 0)
		{
			mPosition = mParts.begin()->second->getActor().getGlobalPosition();
			mOrientation = mParts.begin()->second->getActor().getGlobalOrientation();
		}

		for (auto i = mParts.begin(); i != mParts.end(); i++)
			i->second->mWasRendered = false;

		for (auto i = mParts.begin(); i != mParts.end(); i++)
		{
			if (!i->second->mWasRendered)
				renderPart(i->second, i->second->getActor().getGlobalPosition(), i->second->getActor().getGlobalOrientation());
		}

		if (mPointRenderable) 
			mPointRenderable->setTransform(mPosition, mOrientation);
	}
}