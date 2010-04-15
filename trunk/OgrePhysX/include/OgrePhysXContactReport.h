#pragma once

#include "OgrePhysXClasses.h"
#include "NxPhysics.h"
#include "Ogre.h"

namespace OgrePhysX
{
	class OgrePhysXClass ContactReportListener
	{
	public:
		virtual void onActorContact(NxActor &actor1, NxActor &actor2) {};

		/*
		This method is intended for playing 3D sounds.
		*/
		virtual void onMaterialContact(NxMaterialIndex material1, NxMaterialIndex material2, Ogre::Vector3 position, float force) {};
	};
	class OgrePhysXClass ContactReport : public NxUserContactReport
	{
	private:
		ContactReportListener *mListener;
	public:
		ContactReport(ContactReportListener *listener);
		~ContactReport();
		void onContactNotify(NxContactPair &pair, NxU32 events);
	};

}