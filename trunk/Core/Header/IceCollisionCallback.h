
#pragma once

#include "OgrePhysX.h"
#include "IceScript.h"
#include "IceMessageSystem.h"
#include "IceIncludes.h"
#include "PxPhysicsAPI.h"
#include "PxDefaultSimulationFilterShader.h"

namespace Ice
{

	enum CollisionGroups
	{
		DYNAMICBODY = 1,
		STATICBODY = 2,
		CHARACTER = 4,
		BONE = 8,	
		INTERN = 16		//no collision (for example AI meshes)
	};

	class DllExport PhysXFilterData
	{
	public:
		PhysXFilterData();

		PxFilterData DynamicBody;
		PxFilterData StaticBody;		
		PxFilterData Character;
		PxFilterData Bone;
		PxFilterData Intern;

		//Singleton
		static PhysXFilterData& Instance();
	};

	DllExport PxFilterFlags PhysXSimulationFilterShader (	
		PxFilterObjectAttributes attributes0,
		PxFilterData filterData0, 
		PxFilterObjectAttributes attributes1,
		PxFilterData filterData1,
		PxPairFlags& pairFlags,
		const void* constantBlock,
		PxU32 constantBlockSize);

	class DllExport PhysXSimulationEventCallback : public PxSimulationEventCallback
	{
	private:

		void onMaterialContact(PxMaterial *mat1, PxMaterial *mat2, const Ogre::Vector3 &position, float force);

	public:
		PhysXSimulationEventCallback() {}
		~PhysXSimulationEventCallback() {}

		void onContact(PxContactPair &pair, PxU32 events);

		void onSleep(PxActor** actors, PxU32 count);
		void onWake(PxActor** actors, PxU32 count);

		void onTrigger(PxTriggerPair *pairs, PxU32 count);

		void onConstraintBreak(PxConstraintInfo *constraints, PxU32 count);
	};

};