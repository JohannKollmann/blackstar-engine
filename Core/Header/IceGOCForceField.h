#pragma once

#include "IceIncludes.h"
#include "IceGOComponent.h"
#include "IceGOCEditorInterface.h"
#include "OgrePhysX.h"
#include "IceGOCPhysics.h"

namespace Ice
{
	class GOCForceField : public GOComponent, public GOCStaticEditorInterface
	{
	private:
		Ogre::String mCollisionMeshName;
		int mShapeType;
		NxForceField *mForceField;
		NxForceFieldLinearKernelDesc mFieldLinearKernelDesc;
		float mForceMultiplier;
		float mFalloff;

		void _clear();
		void _create();

	public:
		GOCForceField(void);
		~GOCForceField(void);

		goc_id_type& GetComponentID() const { static std::string name = "ForceField"; return name; }

		void UpdatePosition(Ogre::Vector3 position);
		void UpdateOrientation(Ogre::Quaternion orientation);
		void UpdateScale(Ogre::Vector3 scale);

		//Editor interface
		BEGIN_GOCEDITORINTERFACE(GOCForceField, "ForceField")
			PROPERTY_STRING(mCollisionMeshName, "Volume mesh", ".mesh")
			PROPERTY_FLOAT(mForceMultiplier, "Force Multiplier", 1.0f)
			PROPERTY_FLOAT(mFalloff, "Falloff", 1.0f)
		END_GOCEDITORINTERFACE

		void OnSetParameters();

		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "ForceField"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCForceField; };
	};
}

