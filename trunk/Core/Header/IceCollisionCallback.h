
#pragma once

#include "OgrePhysX.h"
#include "IceScript.h"
#include "IceMessageSystem.h"

namespace Ice
{

	class ActorContactReport : public NxUserContactReport, public MessageListener
	{
	private:
		Script mScript;
		Ogre::String mScriptFileName;

		void onMaterialContact(Ogre::String material1, Ogre::String material2, Ogre::Vector3 position, float force);
	public:
		ActorContactReport(Ogre::String scriptFileName);
		~ActorContactReport() {}
		void onContactNotify(NxContactPair &pair, NxU32 events);

		void ReceiveMessage(Msg &msg);
	};

	class TriggerCallback : public OgrePhysX::TriggerReportListener
	{
	public:
		TriggerCallback(void) {}
		~TriggerCallback(void) {}

		void onEnter(NxActor &trigger, NxActor &other);
		void onLeave(NxActor &trigger, NxActor &other);
	};

	class PhysXUserCallback : public NxUserNotify
	{
        void onSleep(NxActor** actors, NxU32 count);
		void onWake(NxActor** actors, NxU32 count);

		bool onJointBreak(NxReal breakingImpulse, NxJoint& brokenJoint);

	};

};