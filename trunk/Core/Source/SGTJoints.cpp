
#include "SGTJoints.h"
#include "SGTSceneManager.h"


SGTJoint::SGTJoint()
{
	mParent = NULL;
	mInitialised = false;

	SGTMessageSystem::Instance().JoinNewsgroup(this, "UPDATE_PER_FRAME");
}

SGTJoint::~SGTJoint()
{
	Clear();
	mBodyB->UnregisterChild(this);

	SGTMessageSystem::Instance().QuitNewsgroup(this, "UPDATE_PER_FRAME");
}


void SGTJoint::InitCustom(SGTDataMap *params, SGTBody* BodyA, SGTAbstractBody* BodyB, bool intern)
{
	if (!intern)
	{
		if (mParent != NULL && mInitialised) mParent->UnregisterChild(this);
		if (mBodyB != NULL && mInitialised) mBodyB->UnregisterChild(this);
	}
	Clear();
	mInitialised = true;

	Ogre::Vector3 position = params->GetOgreVec3("Position");
	mName = params->HasKey("Name") ? params->GetOgreString("Name") : "Default";

	if (BodyA->GetType() == "Body") BodyA->GetBody()->setSolverIterationCount(8);
	if (BodyB->GetType() == "Body") BodyB->GetBody()->setSolverIterationCount(8);

	mJointParams.setToDefault();
	mJointParams.mBreakableMaxForce = params->GetFloat("BreakableMaxForce");
	mJointParams.mBreakableMaxTorque = params->GetFloat("BreakableMaxTorque");
	Ogre::String breakableCallback = params->GetOgreString("BreakableCallback");	//TODO
	CreateJoint(params, BodyA, BodyB, position);

	mParent = BodyA;
	mBodyB = BodyB;
	mNode = SGTMain::Instance().GetOgreSceneMgr()->getRootSceneNode()->createChildSceneNode(mTypeAsString + Ogre::StringConverter::toString(SGTSceneManager::Instance().RequestID()));
	mNode->setInheritScale(false);
	mNode->setPosition(position);
	mDebugVisual = SGTMain::Instance().GetOgreSceneMgr()->createEntity(mNode->getName() + "_DummyVisual", "JointVisual.mesh");
	mNode->attachObject(mDebugVisual);

	if (!intern)
	{
		mParent->RegisterChild(this);
		mBodyB->RegisterChild(this);
	}

	mParentOffset1 = mParent->GetGlobalOrientation().Inverse() * (mJoint->getGlobalAnchor() - mParent->GetGlobalPosition());
	mParentOffset2 = mBodyB->GetGlobalOrientation().Inverse() * (mJoint->getGlobalAnchor() - mBodyB->GetGlobalPosition());

	SGTSceneManager::Instance().NotifyEntityCreation(this);
}

void SGTJoint::GetParams(SGTDataMap *params)
{
	params->AddOgreString("Name", mName);
	params->AddFloat("BreakableMaxForce", mJointParams.mBreakableMaxForce);
	params->AddFloat("BreakableMaxTorque", mJointParams.mBreakableMaxTorque);
	params->AddOgreString("BreakableCallback", "");	//TODO
	params->AddOgreVec3("Position", mJoint->getGlobalAnchor());
	GetJointParams(params);
}

void SGTJoint::Clear()
{
	if (mInitialised)
	{
		mInitialised = false;
		SGTMain::Instance().GetNxScene()->releaseJoint(mJoint);
		SGTMain::Instance().GetOgreSceneMgr()->destroySceneNode(mNode);
		SGTMain::Instance().GetOgreSceneMgr()->destroyEntity(mDebugVisual);
	}
}

Ogre::Vector3 SGTJoint::GetGlobalPosition()
{
	return mJoint->getGlobalAnchor();
}

void SGTJoint::SetGlobalPosition(Ogre::Vector3 pos)
{
	mJoint->setGlobalAnchor(pos);
	mNode->setPosition(pos);

	mParentOffset1 = mParent->GetGlobalOrientation().Inverse() * (mJoint->getGlobalAnchor() - mParent->GetGlobalPosition());
	mParentOffset2 = mBodyB->GetGlobalOrientation().Inverse() * (mJoint->getGlobalAnchor() - mBodyB->GetGlobalPosition());
}

void SGTJoint::Translate(Ogre::Vector3 vec)
{
	mNode->translate(vec, Ogre::Node::TransformSpace::TS_WORLD);
	mJoint->setGlobalAnchor(mNode->_getDerivedPosition());

	mParentOffset1 = mParent->GetGlobalOrientation().Inverse() * (mJoint->getGlobalAnchor() - mParent->GetGlobalPosition());
	mParentOffset2 = mBodyB->GetGlobalOrientation().Inverse() * (mJoint->getGlobalAnchor() - mBodyB->GetGlobalPosition());
}

Ogre::Entity* SGTJoint::GetVisual()
{
	return mDebugVisual;
}

void SGTJoint::ShowDebugInfo(bool show)
{
	mNode->setVisible(show);
	if (show)
	{
		mParentOffset1 = mParent->GetGlobalOrientation().Inverse() * (mJoint->getGlobalAnchor() - mParent->GetGlobalPosition());
		mParentOffset2 = mBodyB->GetGlobalOrientation().Inverse() * (mJoint->getGlobalAnchor() - mBodyB->GetGlobalPosition());
	}
}

void SGTJoint::Save(SGTSaveSystem& myManager)
{
	myManager.SaveObject(mParent, "mParent");
	myManager.SaveObject(mBodyB, "mBodyB");
	SGTDataMap *params = new SGTDataMap;
	GetParams(params);
	myManager.SaveObject(params, "mParams");
}

void SGTJoint::Load(SGTLoadSystem& mgr)
{
	mParent = (SGTGameObject*)(mgr.LoadObject());
	mParent->RegisterChild(this);
	mBodyB = (SGTAbstractBody*)(mgr.LoadObject());

	SGTDataMap *params = (SGTDataMap*)(mgr.LoadObject());
	InitCustom(params, (SGTBody*)(mParent), mBodyB);
	delete params;

	mInitialised = true;
}

void SGTJoint::ReceiveMessage(SGTMsg &msg)
{
	if (msg.mNewsgroup == "UPDATE_PER_FRAME" && mInitialised && mParent != NULL)
	{
		if (mDebugVisual->isVisible())
		{
			mNode->setPosition(mJoint->getGlobalAnchor());
		}
		else
		{
			mNode->showBoundingBox(false);
		}
	}
}

void SGTJoint::OnParentChanged(SGTGameObject *parent)
{
	if (mDebugVisual->isVisible() && mInitialised)// && ((SGTBody*)mParent)->GetBody()->hasBodyFlag(NX_BF_FROZEN))
	{
		if (parent == mParent)
		{
			if (mParent->GetType() == "StaticBody" || mBodyB->GetType() == "StaticBody")
			{
				SGTDataMap params;
				GetJointParams(&params);
				SGTMain::Instance().GetNxScene()->releaseJoint(mJoint);
				CreateJoint(&params, (SGTBody*)mParent, mBodyB, parent->GetGlobalOrientation() * mParentOffset1 + parent->GetGlobalPosition());
			}
			else mJoint->setGlobalAnchor(parent->GetGlobalOrientation() * mParentOffset1 + parent->GetGlobalPosition());

			mNode->setPosition(mJoint->getGlobalAnchor());
		}
		else if (parent == mBodyB)
		{
			if (mParent->GetType() == "StaticBody" || mBodyB->GetType() == "StaticBody")
			{
				SGTDataMap params;
				params.AddOgreString("Name", mName);
				GetJointParams(&params);
				SGTMain::Instance().GetNxScene()->releaseJoint(mJoint);
				CreateJoint(&params, (SGTBody*)mParent, mBodyB, parent->GetGlobalOrientation() * mParentOffset2 + parent->GetGlobalPosition());
			}
			else mJoint->setGlobalAnchor(parent->GetGlobalOrientation() * mParentOffset2 + parent->GetGlobalPosition());

			mNode->setPosition(mJoint->getGlobalAnchor());
		}
	}
}

//Spherical Joint

void SGTSphericalJoint::CreateJoint(SGTDataMap *params, SGTBody* BodyA, SGTAbstractBody* BodyB, Ogre::Vector3 anchor)
{
	Ogre::Vector3 swingaxis = params->GetOgreVec3("SwingAxis");
	mJointParams.mSwingAxis = NxOgre::NxConvert<NxVec3, Ogre::Vector3>(swingaxis);

	bool jointprojection = params->GetBool("JointProjection");
	if (jointprojection) mJointParams.mJointProjectionMode = NxJointProjectionMode::NX_JPM_POINT_MINDIST;
	mJointParams.mJointProjectionDistance = params->GetFloat("JointProjectionDistance");
	mJointParams.mJointProjectionAngle = params->GetFloat("JointProjectionAngle");

	mJointParams.mHasSwingLimit = params->GetBool("HasSwingLimit");
	mJointParams.mSwingLimit_Restitution = params->GetFloat("SwingLimit_Restitution");
	mJointParams.mSwingLimit_Value = params->GetFloat("SwingLimit_Value");
	mJointParams.mSwingLimit_Hardness = params->GetFloat("SwingLimit_Hardness");
	
	mJointParams.mHasTwistLimit = params->GetBool("HasTwistLimit");
	mJointParams.mTwistLimit_High_Restitution = params->GetFloat("TwistLimit_High_Restitution");
	mJointParams.mTwistLimit_High_Value = params->GetFloat("TwistLimit_High_Value");
	mJointParams.mTwistLimit_High_Hardness = params->GetFloat("TwistLimit_High_Hardness");
	mJointParams.mTwistLimit_Low_Restitution = params->GetFloat("TwistLimit_Low_Restitution");
	mJointParams.mTwistLimit_Low_Value = params->GetFloat("TwistLimit_Low_Value");
	mJointParams.mTwistLimit_Low_Hardness = params->GetFloat("TwistLimit_Low_Hardness");

	mJointParams.mHasTwistSpring = params->GetBool("HasTwistSpring");
	mJointParams.mTwistSpring_Damper = params->GetFloat("TwistSpring_Damper");
	mJointParams.mTwistSpring_Spring = params->GetFloat("TwistSpring_Spring");
	mJointParams.mTwistSpring_Target = params->GetFloat("TwistSpring_Target");

	mJointParams.mHasSwingSpring = params->GetBool("HasSwingSpring");
	mJointParams.mSwingSpring_Damper = params->GetFloat("SwingSpring_Damper");
	mJointParams.mSwingSpring_Spring = params->GetFloat("SwingSpring_Spring");
	mJointParams.mSwingSpring_Target = params->GetFloat("SwingSpring_Target");

	mJointParams.mHasSpring = params->GetBool("HasSpring");
	mJointParams.mSpringDamper = params->GetFloat("SpringDamper");
	mJointParams.mSpringTarget = params->GetFloat("SpringTarget");
	mJointParams.mSpring = params->GetFloat("Spring");

	mJointParams.mHasMotor = params->GetBool("HasMotor");
	mJointParams.mMotorFreeSpin = params->GetBool("MotorFreeSpin");
	mJointParams.mMotorMaxForce = params->GetFloat("MotorMaxForce");
	mJointParams.mMotorVelocityTarget = params->GetFloat("MotorVelocityTarget");

	if (BodyB->GetType() == "StaticBody")
	{
		mJoint = SGTMain::Instance().GetNxScene()->createSphericalJoint(BodyA->GetBody(), anchor, mJointParams);
	}
	else
	{
		mJoint = SGTMain::Instance().GetNxScene()->createSphericalJoint(BodyA->GetBody(), BodyB->GetBody(), anchor, mJointParams);
	}
}

void SGTSphericalJoint::GetJointParams(SGTDataMap *params)
{
	//params->AddOgreVec3("Position", mJoint->getGlobalAnchor());
	Ogre::Vector3 swingaxis = NxOgre::NxConvert<Ogre::Vector3, NxVec3>(mJointParams.mSwingAxis);
	params->AddOgreVec3("SwingAxis", swingaxis);

	bool jointprojection = (mJointParams.mJointProjectionMode == NxJointProjectionMode::NX_JPM_POINT_MINDIST);
	params->AddBool("JointProjection", jointprojection);
	params->AddFloat("JointProjectionDistance", mJointParams.mJointProjectionDistance);
	params->AddFloat("JointProjectionAngle", mJointParams.mJointProjectionAngle);

	params->AddBool("HasSwingLimit", mJointParams.mHasSwingLimit);
	params->AddFloat("SwingLimit_Restitution", mJointParams.mSwingLimit_Restitution);
	params->AddFloat("SwingLimit_Value", mJointParams.mSwingLimit_Value);
	params->AddFloat("SwingLimit_Hardness", mJointParams.mSwingLimit_Hardness);
	
	params->AddBool("HasTwistLimit", mJointParams.mHasTwistLimit);
	params->AddFloat("TwistLimit_High_Restitution", mJointParams.mTwistLimit_High_Restitution);
	params->AddFloat("TwistLimit_High_Value", mJointParams.mTwistLimit_High_Value);
	params->AddFloat("TwistLimit_High_Hardness", mJointParams.mTwistLimit_High_Hardness);
	params->AddFloat("TwistLimit_Low_Restitution", mJointParams.mTwistLimit_Low_Restitution);
	params->AddFloat("TwistLimit_Low_Value", mJointParams.mTwistLimit_Low_Value);
	params->AddFloat("TwistLimit_Low_Hardness", mJointParams.mTwistLimit_Low_Hardness);

	params->AddBool("HasTwistSpring", mJointParams.mHasTwistSpring);
	params->AddFloat("TwistSpring_Damper", mJointParams.mTwistSpring_Damper);
	params->AddFloat("TwistSpring_Spring", mJointParams.mTwistSpring_Spring);
	params->AddFloat("TwistSpring_Target", mJointParams.mTwistSpring_Target);

	params->AddBool("HasSwingSpring", mJointParams.mHasSwingSpring);
	params->AddFloat("SwingSpring_Damper", mJointParams.mSwingSpring_Damper);
	params->AddFloat("SwingSpring_Spring", mJointParams.mSwingSpring_Spring);
	params->AddFloat("SwingSpring_Target", mJointParams.mSwingSpring_Target);

	params->AddBool("HasSpring", mJointParams.mHasSpring);
	params->AddFloat("SpringDamper", mJointParams.mSpringDamper);
	params->AddFloat("SpringTarget", mJointParams.mSpringTarget);
	params->AddFloat("Spring", mJointParams.mSpring);

	params->AddBool("HasMotor", mJointParams.mHasMotor);
	params->AddBool("MotorFreeSpin", mJointParams.mMotorFreeSpin);
	params->AddFloat("MotorMaxForce", mJointParams.mMotorMaxForce);
	params->AddFloat("MotorVelocityTarget", mJointParams.mMotorVelocityTarget);
}

SGTDataMap* SGTSphericalJoint::GetDefaultParams()
{
	NxOgre::JointParams defaultJointParams;
	defaultJointParams.setToDefault();

	SGTDataMap *sphericalJointParams = new SGTDataMap;
	sphericalJointParams->AddOgreVec3("Position", Ogre::Vector3(0,0,0));
	sphericalJointParams->AddOgreVec3("SwingAxis", Ogre::Vector3(0,0,1));
	sphericalJointParams->AddBool("JointProjection", false);
	sphericalJointParams->AddFloat("JointProjectionDistance", defaultJointParams.mJointProjectionDistance);
	sphericalJointParams->AddFloat("JointProjectionAngle", defaultJointParams.mJointProjectionAngle);
	sphericalJointParams->AddFloat("BreakableMaxForce", 1000);
	sphericalJointParams->AddFloat("BreakableMaxTorque", 1000);
	sphericalJointParams->AddOgreString("BreakableCallback", "");
	sphericalJointParams->AddBool("HasSwingLimit", defaultJointParams.mHasSwingLimit);
	sphericalJointParams->AddFloat("SwingLimit_Restitution", defaultJointParams.mSwingLimit_Restitution);
	sphericalJointParams->AddFloat("SwingLimit_Value", defaultJointParams.mSwingLimit_Value);
	sphericalJointParams->AddFloat("SwingLimit_Hardness", defaultJointParams.mSwingLimit_Hardness);
	sphericalJointParams->AddBool("HasTwistLimit", defaultJointParams.mHasTwistLimit);
	sphericalJointParams->AddFloat("TwistLimit_High_Restitution", defaultJointParams.mTwistLimit_High_Restitution);
	sphericalJointParams->AddFloat("TwistLimit_High_Value", defaultJointParams.mTwistLimit_High_Value);
	sphericalJointParams->AddFloat("TwistLimit_High_Hardness", defaultJointParams.mTwistLimit_High_Hardness);
	sphericalJointParams->AddFloat("TwistLimit_Low_Restitution", defaultJointParams.mTwistLimit_Low_Restitution);
	sphericalJointParams->AddFloat("TwistLimit_Low_Value", defaultJointParams.mTwistLimit_Low_Value);
	sphericalJointParams->AddFloat("TwistLimit_Low_Hardness", defaultJointParams.mTwistLimit_Low_Hardness);
	sphericalJointParams->AddBool("HasTwistSpring", defaultJointParams.mHasTwistSpring);
	sphericalJointParams->AddFloat("TwistSpring_Damper", defaultJointParams.mTwistSpring_Damper);
	sphericalJointParams->AddFloat("TwistSpring_Spring", defaultJointParams.mTwistSpring_Spring);
	sphericalJointParams->AddFloat("TwistSpring_Target", defaultJointParams.mTwistSpring_Target);
	sphericalJointParams->AddBool("HasSwingSpring", defaultJointParams.mHasSwingSpring);
	sphericalJointParams->AddFloat("SwingSpring_Damper", defaultJointParams.mSwingSpring_Damper);
	sphericalJointParams->AddFloat("SwingSpring_Spring", defaultJointParams.mSwingSpring_Spring);
	sphericalJointParams->AddFloat("SwingSpring_Target", defaultJointParams.mSwingSpring_Target);
	sphericalJointParams->AddBool("HasSpring", defaultJointParams.mHasSpring);
	sphericalJointParams->AddFloat("SpringDamper", defaultJointParams.mSpringDamper);
	sphericalJointParams->AddFloat("SpringTarget", defaultJointParams.mSpringTarget);
	sphericalJointParams->AddFloat("Spring", defaultJointParams.mSpring);
	sphericalJointParams->AddBool("HasMotor", defaultJointParams.mHasMotor);
	sphericalJointParams->AddBool("MotorFreeSpin", defaultJointParams.mMotorFreeSpin);
	sphericalJointParams->AddFloat("MotorMaxForce", defaultJointParams.mMotorMaxForce);
	sphericalJointParams->AddFloat("MotorVelocityTarget", defaultJointParams.mMotorVelocityTarget);

	return sphericalJointParams;
}

//Revolute Joint

void SGTRevoluteJoint::CreateJoint(SGTDataMap *params, SGTBody* BodyA, SGTAbstractBody* BodyB, Ogre::Vector3 anchor)
{
	bool jointprojection = params->GetBool("JointProjection");
	if (jointprojection) mJointParams.mJointProjectionMode = NxJointProjectionMode::NX_JPM_POINT_MINDIST;
	mJointParams.mJointProjectionDistance = params->GetFloat("JointProjectionDistance");
	mJointParams.mJointProjectionAngle = params->GetFloat("JointProjectionAngle");
	
	mJointParams.mHasLimits = params->GetBool("mHasLimits");
	mJointParams.mUpperLimitRestitution = params->GetFloat("mUpperLimitRestitution");
	mJointParams.mUpperLimit = params->GetFloat("mUpperLimit");
	mJointParams.mLowerLimitRestitution = params->GetFloat("mLowerLimitRestitution");
	mJointParams.mLowerLimit = params->GetFloat("mLowerLimit");

	mJointParams.mHasSpring = params->GetBool("HasSpring");
	mJointParams.mSpringDamper = params->GetFloat("SpringDamper");
	mJointParams.mSpringTarget = params->GetFloat("SpringTarget");
	mJointParams.mSpring = params->GetFloat("Spring");

	mJointParams.mHasMotor = params->GetBool("HasMotor");
	mJointParams.mMotorFreeSpin = params->GetBool("MotorFreeSpin");
	mJointParams.mMotorMaxForce = params->GetFloat("MotorMaxForce");
	mJointParams.mMotorVelocityTarget = params->GetFloat("MotorVelocityTarget");

	if (BodyB->GetType() == "StaticBody")
	{
		mJoint = SGTMain::Instance().GetNxScene()->createSphericalJoint(BodyA->GetBody(), anchor, mJointParams);
	}
	else
	{
		mJoint = SGTMain::Instance().GetNxScene()->createSphericalJoint(BodyA->GetBody(), BodyB->GetBody(), anchor, mJointParams);
	}
}

void SGTRevoluteJoint::GetJointParams(SGTDataMap *params)
{
	//params->AddOgreVec3("Position", mJoint->getGlobalAnchor());

	bool jointprojection = (mJointParams.mJointProjectionMode == NxJointProjectionMode::NX_JPM_POINT_MINDIST);
	params->AddBool("JointProjection", jointprojection);
	params->AddFloat("JointProjectionDistance", mJointParams.mJointProjectionDistance);
	params->AddFloat("JointProjectionAngle", mJointParams.mJointProjectionAngle);

	params->AddBool("mHasLimits", mJointParams.mHasLimits);
	params->AddFloat("mUpperLimitRestitution", mJointParams.mUpperLimitRestitution);
	params->AddFloat("mUpperLimit", mJointParams.mUpperLimit);
	params->AddFloat("mLowerLimitRestitution", mJointParams.mLowerLimitRestitution);
	params->AddFloat("mLowerLimit", mJointParams.mLowerLimit);

	params->AddBool("HasSpring", mJointParams.mHasSpring);
	params->AddFloat("SpringDamper", mJointParams.mSpringDamper);
	params->AddFloat("SpringTarget", mJointParams.mSpringTarget);
	params->AddFloat("Spring", mJointParams.mSpring);

	params->AddBool("HasMotor", mJointParams.mHasMotor);
	params->AddBool("MotorFreeSpin", mJointParams.mMotorFreeSpin);
	params->AddFloat("MotorMaxForce", mJointParams.mMotorMaxForce);
	params->AddFloat("MotorVelocityTarget", mJointParams.mMotorVelocityTarget);
}

SGTDataMap* SGTRevoluteJoint::GetDefaultParams()
{
	NxOgre::JointParams defaultJointParams;
	defaultJointParams.setToDefault();

	SGTDataMap *revoluteJointParams = new SGTDataMap;
	revoluteJointParams->AddOgreVec3("Position", Ogre::Vector3(0,0,0));
	revoluteJointParams->AddFloat("BreakableMaxForce", 1000);
	revoluteJointParams->AddFloat("BreakableMaxTorque", 1000);
	revoluteJointParams->AddOgreString("BreakableCallback", "");
	revoluteJointParams->AddBool("JointProjection", false);
	revoluteJointParams->AddFloat("JointProjectionDistance", defaultJointParams.mJointProjectionDistance);
	revoluteJointParams->AddFloat("JointProjectionAngle", defaultJointParams.mJointProjectionAngle);
	revoluteJointParams->AddBool("mHasLimits", defaultJointParams.mHasLimits);
	revoluteJointParams->AddFloat("mUpperLimitRestitution", defaultJointParams.mUpperLimitRestitution);
	revoluteJointParams->AddFloat("mUpperLimit", defaultJointParams.mUpperLimit);
	revoluteJointParams->AddFloat("mLowerLimitRestitution", defaultJointParams.mLowerLimitRestitution);
	revoluteJointParams->AddFloat("mLowerLimit", defaultJointParams.mLowerLimit);
	revoluteJointParams->AddBool("HasSpring", defaultJointParams.mHasSpring);
	revoluteJointParams->AddFloat("SpringDamper", defaultJointParams.mSpringDamper);
	revoluteJointParams->AddFloat("SpringTarget", defaultJointParams.mSpringTarget);
	revoluteJointParams->AddFloat("Spring", defaultJointParams.mSpring);
	revoluteJointParams->AddBool("HasMotor", defaultJointParams.mHasMotor);
	revoluteJointParams->AddBool("MotorFreeSpin", defaultJointParams.mMotorFreeSpin);
	revoluteJointParams->AddFloat("MotorMaxForce", defaultJointParams.mMotorMaxForce);
	revoluteJointParams->AddFloat("MotorVelocityTarget", defaultJointParams.mMotorVelocityTarget);

	return revoluteJointParams;
}

//Fixed Joint

void SGTFixedJoint::CreateJoint(SGTDataMap *params, SGTBody* BodyA, SGTAbstractBody* BodyB, Ogre::Vector3 anchor)
{
	if (BodyB->GetType() == "StaticBody")
	{
		mJoint = SGTMain::Instance().GetNxScene()->createSphericalJoint(BodyA->GetBody(), anchor, mJointParams);
	}
	else
	{
		mJoint = SGTMain::Instance().GetNxScene()->createSphericalJoint(BodyA->GetBody(), BodyB->GetBody(), anchor, mJointParams);
	}
}

void SGTFixedJoint::GetJointParams(SGTDataMap *params)
{
	//params->AddOgreVec3("Position", mJoint->getGlobalAnchor());
}

SGTDataMap* SGTFixedJoint::GetDefaultParams()
{

	SGTDataMap *fixedJointParams = new SGTDataMap;
	fixedJointParams->AddOgreVec3("Position", Ogre::Vector3(0,0,0));
	fixedJointParams->AddFloat("BreakableMaxForce", 1000);
	fixedJointParams->AddFloat("BreakableMaxTorque", 1000);
	fixedJointParams->AddOgreString("BreakableCallback", "");

	return fixedJointParams;
}