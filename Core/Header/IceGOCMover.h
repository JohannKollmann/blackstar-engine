#pragma once

#include "IceIncludes.h"
#include "IceGOComponent.h"
#include "IceMain.h"
#include "Ice3D.h"
#include "IceGOCEditorInterface.h"
#include "IceMessageListener.h"
#include "IceGOCView.h"
#include "IceSpline.h"
#include "IceUtils.h"
#include "IceGOCScriptMakros.h"

namespace Ice
{
	class GOCMover;

	class DllExport AnimKey
	{
	protected:
		GOCMover *mMover;
		float mTimeToNextKey;

	public:
		AnimKey() : mMover(nullptr), mTimeToNextKey(1) {}
		virtual ~AnimKey() {}

		virtual void SetMover(GOCMover *mover) {};
		GOCMover* GetMover() { return mMover; }

		float GetTimeToNextKey() { return mTimeToNextKey; }

		GameObject* CreateSuccessor();
	};

	class DllExport GOCAnimKey : public AnimKey, public GOCEditorVisualised, public GOCStaticEditorInterface
	{
		friend class GOCMover;
	private:
		AnimKey *mPredecessor;

	public:
		GOCAnimKey() {mTimeToNextKey = 1;}
		GOCAnimKey(AnimKey *pred);
		~GOCAnimKey(void);

		void SetMover(GOCMover *mover);

		goc_id_type& GetComponentID() const { static std::string name = "MoverAnimKey"; return name; }

		void UpdatePosition(Ogre::Vector3 position);
		void UpdateOrientation(Ogre::Quaternion orientation) {}

		Ogre::String GetEditorVisualMeshName() { return "Editor_AnimKey.mesh"; }

		BEGIN_GOCEDITORINTERFACE(GOCAnimKey, "Anim Key")
			PROPERTY_FLOAT(mTimeToNextKey, "TimeToNextKey", 1.0f)
		END_GOCEDITORINTERFACE

		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "MoverAnimKey"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCAnimKey; }
	};

	class DllExport GOCMover : /*public AnimKey, */public GOCEditorVisualised, public GOCStaticEditorInterface, public MessageListener, public Utils::DeleteListener
	{
		friend class GOCAnimKey;

	private:
		std::vector<GameObject*> mAnimKeys;
		Ogre::String mKeyCallback;
		bool mMoving;
		bool mPerformingMovement;
		bool mPaused;
		bool mLoop;
		bool mIsClosed;
		bool mStaticMode;
		bool mIgnoreOrientation;
		float mfLastPos;
		int mLastKeyIndex;
		Spline mSpline;
		std::vector<double> mKeyTiming;
		//Spline mTimeSpline;
		Ogre::ManualObject *mSplineObject;
		Ogre::ManualObject *mLookAtLine;
		Ogre::ManualObject *mNormalLookAtLine;

		GameObject *mLookAtObject;			//the mover will always face this object
		GameObject *mNormalLookAtObject;	//the mover's normal will always face this object

		void notifyKeyDelete(GOCAnimKey *key);

		void _updateNormalLookAtLine();
		void _updateLookAtLine();
		void _destroyNormalLookAtLine();
		void _destroyLookAtLine();

	public:
		GOCMover(void);
		~GOCMover(void);

		void Init();

		void PrepareMovement(bool prepare);

		void Trigger();
		void Pause();
		void Stop();
		void UpdateKeys();

		void ReceiveMessage(Msg &msg);

		goc_id_type& GetComponentID() const { static std::string name = "Mover"; return name; }

		void UpdatePosition(Ogre::Vector3 position);
		void UpdateOrientation(Ogre::Quaternion orientation) {}

		void SetLookAtObject(GameObject *target);
		void SetNormalLookAtObject(GameObject *target);
		void onDeleteSubject(Utils::DeleteListener* subject);

		Ogre::String GetEditorVisualMeshName() { return "Editor_AnimKey.mesh"; }
		void ShowEditorVisual(bool show);		//Override, because we want to draw lines to the target objects

		bool IsMoving() { return mMoving; }

		void InsertKey(GameObject *key, AnimKey *pred);

		void OnAddChild(GameObject *child);

		void SetKeyIgnoreParent(bool ignore);

		//Editor interface
		BEGIN_GOCEDITORINTERFACE(GOCMover, "Mover")
			//PROPERTY_FLOAT(mTimeToNextKey, "TimeToNextKey", 1.0f);
			PROPERTY_BOOL(mIsClosed, "Closed", false);
			PROPERTY_BOOL(mStaticMode, "Static Mode", false);
			PROPERTY_BOOL(mIgnoreOrientation, "Ignore Orientation", false);
		END_GOCEDITORINTERFACE

		void OnSetParameters();
		void SetOwner(GameObject *owner);

		//Scripting
		std::vector<ScriptParam> TriggerMover(Script &caller, std::vector<ScriptParam> &params) { Trigger();  return std::vector<ScriptParam>(); };
		std::vector<ScriptParam> PauseMover(Script &caller, std::vector<ScriptParam> &params) { Pause();  return std::vector<ScriptParam>(); };
		std::vector<ScriptParam> StopMover(Script &caller, std::vector<ScriptParam> &params) { Stop();  return std::vector<ScriptParam>(); };
		DEFINE_GOCLUAMETHOD(GOCMover, TriggerMover)
		DEFINE_GOCLUAMETHOD(GOCMover, PauseMover)
		DEFINE_GOCLUAMETHOD(GOCMover, StopMover)

		//Load Save
		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "Mover"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCMover; }
	};

}