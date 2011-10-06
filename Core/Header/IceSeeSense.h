#pragma once

#include "IceSpatialCoverage.h"
#include "OgrePhysX.h"
#include "IceIncludes.h"

namespace Ice
{
	class DllExport SeeSense
	{
	public:
		class DllExport VisualObject
		{
		public:
			virtual ~VisualObject() {}

			virtual Ogre::String GetVisualObjectDescription() = 0;
			virtual void GetTrackPoints(std::vector<Ogre::Vector3> &outPoints) = 0;
		};

		class DllExport Origin
		{
		public:
			virtual Ogre::Vector3 GetEyePosition() = 0;
			virtual Ogre::Quaternion GetEyeOrientation() = 0;
		};

		class DllExport Callback
		{
		public:
			/**
			@param eyeSpacePosition The position of the observed object in eye space.
			@param brightness The brightness of the observed object.
			@param impulseStrength	A value between 0 and 1 that determines how sure the subject is that it sees the object in question.
									This contains the number of ray hits, the distance, the object brightness and the view angle.
			@param description The description of the object.
			*/
			virtual void OnSeeSomething(const Ogre::Vector3 &eyeSpacePosition, float distance, VisualObject *object) = 0; 
		};

		SeeSense(Origin *origin, Callback *callback);
		~SeeSense();

		void UpdateSense(float time);

		float CalcVisibility(VisualObject *target);

	private:

		struct Impulse
		{
			Ogre::Vector3 eyeSpacePosition;
			float distance;
			VisualObject *object;
		};

		std::unordered_set<VisualObject*> mActiveImpulses;

		Origin *mEyeOrigin;

		static const int RAYS_PER_SECOND = 200;
		static const int NUM_RAYHITS_OPTIMALSIGHT = 2;
		static const float RAY_MAXDIST;
		static const float WIDTH_COVERAGE_PERCENT;
		static const float HEIGHT_COVERAGE_PERCENT;

		SpatialCoverage mSpatialCoverage;

		bool raycast(const Ogre::Vector3 &rayDir, OgrePhysX::Scene::RaycastHit &hit);

		float computeLighting(const Ogre::Vector3 &position, const Ogre::Vector3 &normal);

		Callback *mCallback;
	};
}