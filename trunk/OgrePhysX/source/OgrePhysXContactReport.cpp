
#include "OgrePhysXContactReport.h"
#include "OgrePhysXConvert.h"

#pragma warning (disable:4482)

namespace OgrePhysX
{

	ContactReport::ContactReport(ContactReportListener *listener)
	{
		mListener = listener;
	}
	ContactReport::~ContactReport()
	{
		delete mListener;
	}

	void ContactReport::onContactNotify(NxContactPair &pair, NxU32 events)
	{
		// Iterate through contact points
		NxContactStreamIterator i(pair.stream);
		//user can call getNumPairs() here
		while(i.goNextPair())
		{
			NxShape *shape1 = i.getShape(0);
			NxShape *shape2 = i.getShape(1);

			//First material callback
			NxMaterialIndex material1 = shape1->getMaterial();
			NxMaterialIndex material2 = shape2->getMaterial();

			float summed_force = 0.0f;
			Ogre::Vector3 contactPoint;
			while(i.goNextPatch())
			{
				//user can also call getPatchNormal() and getNumPoints() here
				const NxVec3& contactNormal = i.getPatchNormal();
				while(i.goNextPoint())
				{
					//user can also call getPoint() and getSeparation() here
					contactPoint = Convert::toOgre(i.getPoint());
					summed_force += i.getPointNormalForce();

					if (shape1->getType() == NxShapeType::NX_SHAPE_MESH)	//Hack
					{
						NxTriangleMeshShape *meshShape = (NxTriangleMeshShape*)shape1;
						material1 = meshShape->getTriangleMesh().getTriangleMaterial(i.getFeatureIndex0());
					}
					if (shape2->getType() == NxShapeType::NX_SHAPE_MESH)
					{
						NxTriangleMeshShape *meshShape = (NxTriangleMeshShape*)shape2;
						material2 = meshShape->getTriangleMesh().getTriangleMaterial(i.getFeatureIndex1());
					}
				}
			}
			//The last contact point is returned, not for any particular reason, but in most cases there is only ohne contact point
			mListener->onMaterialContact(material1, material2, contactPoint, summed_force);

			//Simple actor-based callback
			mListener->onActorContact(shape1->getActor(), shape2->getActor());
			
		}
	}

}