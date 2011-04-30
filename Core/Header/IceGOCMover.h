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

	class DllExport GOCAnimKey : public GOCEditorVisualised, public GOCStaticEditorInterface
	{
		friend class GOCMover;

	private:
		float mTimeToNextKey;

	public:
		GOCAnimKey() : mTimeToNextKey(1.0f) {}
		~GOCAnimKey() {};

		goc_id_type& GetComponentID() const { static std::string name = "MoverAnimKey"; return name; }

		float GetTimeToNextKey() { return mTimeToNextKey > 0 ? mTimeToNextKey : 0.01f; }

		void UpdatePosition(Ogre::Vector3 position);
		void UpdateOrientation(Ogre::Quaternion orientation) {}

		GameObjectPtr GetMover();

		GameObjectPtr CreateSuccessor();

		Ogre::String GetEditorVisualMeshName() { return "Editor_AnimKey.mesh"; }

		BEGIN_GOCEDITORINTERFACE(GOCAnimKey, "Anim Key")
			PROPERTY_FLOAT(mTimeToNextKey, "TimeToNextKey", 1.0f)
		END_GOCEDITORINTERFACE

		void Save(LoadSave::SaveSystem& mgr);
		void Load(LoadSave::LoadSystem& mgr);
		static void Register(std::string* pstrName, LoadSave::SaveableInstanceFn* pFn) { *pstrName = "MoverAnimKey"; *pFn = (LoadSave::SaveableInstanceFn)&NewInstance; };
		static LoadSave::Saveable* NewInstance() { return new GOCAnimKey; }
	};

	class DllExport GOCMover : public GOCEditorVisualised, public GOCStaticEditorInterface, public MessageListener
	{
		friend class GOCAnimKey;

	public:
		enum ReferenceTypes
		{
			KEY = 1,
			LOOKAT = 2,
			NORMALLOOKAT = 3,
			MOVER = 4
		};

	private:
		Ogre::String mKeyCallback;
		bool mMoving;	//for intern use
		bool mEnabled;	//for editor interface
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

		///Resets the mover to the the first key
		void Reset();

		void ReceiveMessage(Msg &msg);

		goc_id_type& GetComponentID() const { static std::string name = "Mover"; return name; }

		void UpdatePosition(Ogre::Vector3 position);
		void UpdateOrientation(Ogre::Quaternion orientation) {}

		void SetLookAtObject(GameObjectPtr target);
		void SetNormalLookAtObject(GameObjectPtr target);
		GameObjectPtr GetLookAtObject();
		GameObjectPtr GetNormalLookAtObject();

		//Creates a new key object which is inserted after the key at insertIndex.
		GameObjectPtr CreateKey(unsigned int insertIndex);

		unsigned int GetNumKeys();
		GameObjectPtr GetKey(unsigned int index);

		Ogre::String GetEditorVisualMeshName() { return "Editor_AnimKey.mesh"; }
		void ShowEditorVisual(bool show);		//Override, because we want to draw lines to the target objects

		bool IsMoving() { return mMoving; }

		//Editor interface
		BEGIN_GOCEDITORINTERFACE(GOCMover, "Mover")
			//PROPERTY_FLOAT(mTimeToNextKey, "TimeToNextKey", 1.0f);
			PROPERTY_BOOL(mIsClosed, "Closed", false);
			PROPERTY_BOOL(mStaticMode, "Static Mode", false);
			PROPERTY_BOOL(mIgnoreOrientation, "Ignore Orientation", false);
			PROPERTY_BOOL(mEnabled, "Enabled", false);
		END_GOCEDITORINTERFACE

		void OnSetParameters();
		void SetOwner(std::weak_ptr<GameObject> go);

		//Scripting
		std::vector<ScriptParam> AddKey(Script &caller, std::vector<ScriptParam> &params) { CreateKey(params[0].getInt());  return std::vector<ScriptParam>(); };
		std::vector<ScriptParam> SetLookAtObject(Script &caller, std::vector<ScriptParam> &params) { SetLookAtObject(Ice::SceneManager::Instance().GetObjectByInternID(params[0].getInt()));  return std::vector<ScriptParam>(); };
		std::vector<ScriptParam> SetNormalLookAtObject(Script &caller, std::vector<ScriptParam> &params) { SetNormalLookAtObject(Ice::SceneManager::Instance().GetObjectByInternID(params[0].getInt()));  return std::vector<ScriptParam>(); };
		std::vector<ScriptParam> TriggerMover(Script &caller, std::vector<ScriptParam> &params) { Trigger();  return std::vector<ScriptParam>(); };
		std::vector<ScriptParam> PauseMover(Script &caller, std::vector<ScriptParam> &params) { Pause();  return std::vector<ScriptParam>(); };
		std::vector<ScriptParam> StopMover(Script &caller, std::vector<ScriptParam> &params) { Stop();  return std::vector<ScriptParam>(); };
		DEFINE_TYPEDGOCLUAMETHOD(GOCMover, AddKey, "int")
		DEFINE_TYPEDGOCLUAMETHOD(GOCMover, SetLookAtObject, "int")
		DEFINE_TYPEDGOCLUAMETHOD(GOCMover, SetNormalLookAtObject, "int")
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