
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

		TiXmlDocument document;
		Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton().openResource(meshSplitConfigFile);
		if (stream.isNull())
		{
			Ogre::LogManager::getSingleton().logMessage("[OgrePhysX] Could not open file: " + meshSplitConfigFile);
			return;
		}

		//Get the file contents
		Ogre::String data = stream->getAsString();

		//Parse the XML document
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

		Ogre::SceneNode *node = mDestructible->mScene->getOgreSceneMgr()->getRootSceneNode()->createChildSceneNode();
		Ogre::Entity* ent = mDestructible->mScene->getOgreSceneMgr()->createEntity(meshName);
		node->attachObject(ent);
		node->setScale(mDestructible->mScale);

		PxTransform shapeOffset = PxTransform::createIdentity();

		float shapeSizeFactor = 0.8f;

		Ogre::Vector3 offset = ent->getBoundingBox().getCenter() * mDestructible->mScale;
		Ogre::Vector3 scaledOffset = offset * shapeSizeFactor;
		shapeOffset.p = Convert::toPx(offset - scaledOffset);

		Ogre::Vector3 scale = mDestructible->mScale * shapeSizeFactor;

		Cooker::Params params;
		params.scale(scale);
		if (ent->getBoundingBox().getSize().x * scale.x > 0.2f &&
			ent->getBoundingBox().getSize().y * scale.y > 0.2f &&
			ent->getBoundingBox().getSize().z * scale.z > 0.2f)
			mActor = mDestructible->mScene->createRigidDynamic(Geometry::convexMeshGeometry(ent->getMesh(), params), mDestructible->mDensity, *mDestructible->mMaterial, shapeOffset); 

		else 
		{
			//mesh is too small for convex mesh
			PxBoxGeometry boxGeometry = Geometry::boxGeometry(ent, shapeOffset, scale);
			mActor = mDestructible->mScene->createRigidDynamic(boxGeometry, mDestructible->mDensity, *mDestructible->mMaterial, shapeOffset); 
		}

		mRenderedActor = new RenderedActorBinding(mActor.getPxActor(), new NodeRenderable(node));
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

		part1->mEdges.push_back(part2);
		part2->mEdges.push_back(part1);

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

	void Destructible::setPosition(const Ogre::Vector3 &position)
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

	void Destructible::sync()
	{
		if (mParts.size() > 0)
		{
			mPosition = mParts.begin()->second->getActor().getGlobalPosition();
			mOrientation = mParts.begin()->second->getActor().getGlobalOrientation();
		}

		for (auto i = mParts.begin(); i != mParts.end(); i++)
			i->second->getRenderedActor()->sync();

		if (mPointRenderable) 
			mPointRenderable->setTransform(mPosition, mOrientation);
	}
}