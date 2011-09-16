#pragma once

#include "IceIncludes.h"
#include "IceGOComponent.h"
#include "IceGOCEditorInterface.h"
#include "OgrePhysX.h"
#include "IceGOCPhysics.h"
#include "IceGOCOgreNode.h"
#include "IceGOCScriptMakros.h"

namespace Ice
{
	class GOCForceField : public GOCOgreNodeUser, public GOCStaticEditorInterface
	{
	private:
		Ogre::String mCollisionMeshName;
		Ogre::Vector3 mForceDirection;
		int mShapeType;
		float mForceMultiplier;
		float mFalloff;
		bool mActive;
		Ogre::Entity *mEditorVisual;	//Volume
		Ogre::Entity *mEditorVisual2;	//Arrow
		Ogre::SceneNode *mArrowNode;

		void _clear();
		void _create();

	public:
		GOCForceField(void);
		~GOCForceField(void);

		GOComponent::TypeID& GetComponentID() const { static std::string name = "ForceField"; return name; }

		void UpdatePosition(const Ogre::Vector3 &position);
		void UpdateOrientation(const Ogre::Quaternion &orientation);
		void UpdateScale(const Ogre::Vector3 &scale);

		void ShowEditorVisual(bool show);

		//Editor interface
		BEGIN_GOCEDITORINTERFACE(GOCForceField, "ForceField")
			PROPERTY_VECTOR3(mForceDirection, "Force Direction", Ogre::Vector3(0,1,0))
			PROPERTY_STRING(mCollisionMeshName, "Volume mesh", ".mesh")
			PROPERTY_FLOAT(mForceMultiplier, "Force Multiplier", 1.0f)
			PROPERTY_FLOAT(mFalloff, "Falloff", 1.0f)
			PROPERTY_BOOL(mActive, "Active", true)
		END_GOCEDITORINTERFACE

		void OnSetParameters();

		std::vector<ScriptParam> Forcefield_Activate(Script& caller, std::vector<ScriptParam> &vParams);
		DEFINE_TYPEDGOCLUAMETHOD(GOCForceField, Forcefield_Activate, "bool")

		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "ForceField"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCForceField; };
	};
}

