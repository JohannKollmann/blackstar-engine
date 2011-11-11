
#include "IceSeeSense.h"
#include "IceMain.h"
#include "OgrePhysX.h"
#include "IceCollisionCallback.h"
#include "IceGameObject.h"
#include "IceAIManager.h"

namespace Ice
{
	const float SeeSense::RAY_MAXDIST = 50.0f;
	const float SeeSense::WIDTH_COVERAGE_PERCENT = 0.8f;
	const float SeeSense::HEIGHT_COVERAGE_PERCENT = 0.6f;
	const float SeeSense::UPDATE_VISIBILITY_INTERVAL = 1.0f;
	const float SeeSense::NOSIGHT_THRESHOLD = 0.01f;

	SeeSense::SeeSense(Origin *origin, Callback *callback)
	{
		mEyeOrigin = origin;
		mCallback = callback;
		mUpdateVisibilityCounter = 0.0f;
	}

	SeeSense::~SeeSense()
	{
	}

	float SeeSense::computeShadowFactor(const Ogre::Vector3 &position, Ogre::Vector3 &rayDir, float maxDist)
	{
		OgrePhysX::Scene::RaycastHit hit;
		return raycast(position, rayDir, maxDist, hit) ? 0 : 1;
	}

	float SeeSense::computeShadowFactor(const Ogre::Vector3 &position, const Ogre::Vector3 &lightPosition)
	{
		Ogre::Vector3 rayDir = lightPosition - position;
		float dist = rayDir.normalise();
		return computeShadowFactor(position, rayDir, dist);
	}

	float SeeSense::computeLighting(const Ogre::Vector3 &position, const Ogre::Vector3 &normal)
	{
		float totalLight = AIManager::Instance().GetAmbientLightBrightness();

		std::vector<Ogre::Light*> lights;
		AIManager::Instance().GetLights(lights);
		ITERATE(i, lights)
		{
			if ((*i)->getType() == Ogre::Light::LightTypes::LT_DIRECTIONAL)
			{
				Ogre::Vector3 lightDirInverted = -(*i)->getDirection();
				float light = std::max(0.0f, normal.dotProduct(lightDirInverted));
				//light *= computeShadowFactor(position + (normal * 0.2f) + (lightDirInverted * 0.2f), lightDirInverted);
				totalLight += light;
			}
		}
		return std::min(1.0f, totalLight);
	}

	float SeeSense::CalcVisibility(VisualObject *target)
	{
		std::vector<Ogre::Vector3> trackPoints;
		target->GetTrackPoints(trackPoints);
		float viewFactor = 0.0f;
		if (!trackPoints.empty())
		{
			int numHits = 0;
			float maxLighting = 0.0f;
			float rightAngle = Ogre::Math::PI*0.5f;
			float minViewAngle = rightAngle;
			Ogre::Vector3 eyeDir = mEyeOrigin->GetEyeOrientation() * Ogre::Vector3::UNIT_Z;
			ITERATE(point, trackPoints)
			{
				Ogre::Vector3 rayDir = *point - mEyeOrigin->GetEyePosition();
				rayDir.normalise();
				float viewAngle = eyeDir.angleBetween(rayDir).valueRadians();
				if (viewAngle < rightAngle)
				{
					OgrePhysX::Scene::RaycastHit hit;
					if (raycast(rayDir, hit))
					{			
						if (hit.hitActorUserData != nullptr)
						{
							GameObject *go = (GameObject*)hit.hitActorUserData;
							if (go->GetComponent<VisualObject>() == target)
							{
								numHits++;
								maxLighting = std::max(maxLighting, computeLighting(hit.position, hit.normal));
								minViewAngle = std::min(minViewAngle, viewAngle);
							}
						}
					}
				}
			}
			float numHitsFactor = (float)numHits / trackPoints.size();
			numHitsFactor = std::min(1.0f, numHitsFactor*2.0f);
			float lightingFactor = maxLighting;
			float viewAngleFactor = 1 - (minViewAngle / rightAngle);
			viewFactor = numHitsFactor * lightingFactor * viewAngleFactor;
		}

		return viewFactor;
	}

	bool SeeSense::raycast(const Ogre::Vector3 &rayDir, OgrePhysX::Scene::RaycastHit &hit)
	{
		return raycast(mEyeOrigin->GetEyePosition(), rayDir, RAY_MAXDIST, hit);
	}

	bool SeeSense::raycast(const Ogre::Vector3 &origin, const Ogre::Vector3 &rayDir, float maxDist, OgrePhysX::Scene::RaycastHit &hit)
	{
		PxSceneQueryFilterData filterData;
		filterData.data.word0 = CollisionGroups::CHARACTER|CollisionGroups::DYNAMICBODY|CollisionGroups::STATICBODY;
		filterData.flags = PxSceneQueryFilterFlag::eDYNAMIC|PxSceneQueryFilterFlag::eSTATIC;
		return Main::Instance().GetPhysXScene()->raycastClosest(origin, rayDir, maxDist, hit, filterData);
	}

	void SeeSense::UpdateSense(float time)
	{
		mUpdateVisibilityCounter += time;
		if (mUpdateVisibilityCounter >= UPDATE_VISIBILITY_INTERVAL)
		{
			ITERATE(i, mActiveImpulses)
			{
				i->second = CalcVisibility(i->first);
				if (i->second <= NOSIGHT_THRESHOLD)
				{
					i = mActiveImpulses.erase(i);
					if (i == mActiveImpulses.end()) break;
				}
			}
			mUpdateVisibilityCounter = 0.0f; 
		}

		int numSteps = time * RAYS_PER_SECOND;

		for (int i = 0; i < numSteps; i++)
		{
			std::pair<float, float> sample2D = mSpatialCoverage.getNext();
			Ogre::Vector3 eyeDir;
			eyeDir.x = (2*sample2D.first - 1) * WIDTH_COVERAGE_PERCENT;
			eyeDir.y = (2*sample2D.second - 1) * HEIGHT_COVERAGE_PERCENT;
			eyeDir.z = Ogre::Math::Sqrt(1 - (eyeDir.x*eyeDir.x + eyeDir.y*eyeDir.y));
			Ogre::Vector3 rayDir = mEyeOrigin->GetEyeOrientation() * Ogre::Vector3(eyeDir.x, eyeDir.y, eyeDir.z);

			OgrePhysX::Scene::RaycastHit hit;
			if (raycast(rayDir, hit))
			{			
				if (hit.hitActorUserData != nullptr)
				{
					GameObject *go = (GameObject*)hit.hitActorUserData;
					VisualObject *obj = go->GetComponent<VisualObject>();
					if (obj)
					{
						auto impulseIter = mActiveImpulses.find(obj);
						if (impulseIter == mActiveImpulses.end())
						{
							float viewFactor = CalcVisibility(obj);
							if (viewFactor > NOSIGHT_THRESHOLD)
							{
								mActiveImpulses.insert(std::make_pair(obj, viewFactor));
								mCallback->OnSeeSomething(eyeDir * hit.distance, hit.distance, viewFactor, go->GetID());
							}
						}
					}
				}
			}
		}
	}
}
