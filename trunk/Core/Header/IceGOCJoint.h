
#pragma once

#include "IceGOComponent.h"
#include "IceGOCEditorInterface.h"
#include "IceGOCView.h"
#include "NxScene.h"

namespace Ice
{

	class GOCJoint : public GOCEditorVisualised
	{
	public:
		enum ReferenceTypes
		{
			JOINT = 12345670,
			JOINTACTOR = 12345671
		};

	protected:
		NxJoint *mPhysXJoint;

		float mMaxTorque;
		float mMaxForce;

		void createJoint(NxJointDesc &desc);
		void clear();

	public:
		GOCJoint() : mPhysXJoint(nullptr), mMaxForce(NX_MAX_REAL), mMaxTorque(NX_MAX_REAL) {}
		virtual ~GOCJoint();

		void SetActorObjects(GameObjectPtr actor1, GameObjectPtr actor2);

		goc_id_type& GetFamilyID() const { static std::string name = "Joint"; return name; }

		Ogre::String GetEditorVisualMeshName() { return "sphere.mesh"; }

		virtual void Save(LoadSave::SaveSystem& mgr);
		virtual void Load(LoadSave::LoadSystem& mgr);
	};

	class GOCFixedJoint : public GOCJoint, public GOCStaticEditorInterface
	{
	public:
		goc_id_type& GetComponentID() const { static std::string name = "Fixed Joint"; return name; }

		BEGIN_GOCEDITORINTERFACE(GOCFixedJoint, "Fixed Joint")
			PROPERTY_FLOAT(mMaxForce, "Max Force", NX_MAX_REAL);
			PROPERTY_FLOAT(mMaxTorque, "Max Torque", NX_MAX_REAL);
		END_GOCEDITORINTERFACE

		void OnSetParameters();
		void NotifyOwnerGO();

		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "Fixed Joint"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCFixedJoint; }
	};

}